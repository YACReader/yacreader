#include "folder_query_result_processor.h"

#include "folder_item.h"
#include "qnaturalsorting.h"
#include "yacreader_global_gui.h"
#include "query_parser.h"
#include "folder_model.h"
#include "data_base_management.h"

#include "QsLog.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

//Copy/pasted from "folder_model.cpp"
#define ROOT 1

YACReader::FolderQueryResultProcessor::FolderQueryResultProcessor(FolderModel *model)
    : querySearchQueue(1), model(model)
{
}

void YACReader::FolderQueryResultProcessor::createModelData(const YACReader::SearchModifiers modifier, const QString &filter, bool includeComics)
{
    querySearchQueue.cancellPending();

    querySearchQueue.enqueue([=] {
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(model->getDatabase());

            QSqlQuery selectQuery(db); //TODO check
            if (!includeComics) {
                selectQuery.prepare("select * from folder where id <> 1 and upper(name) like upper(:filter) order by parentId,name ");
                selectQuery.bindValue(":filter", "%%" + filter + "%%");
            } else {
                std::string queryString("SELECT DISTINCT f.id, f.parentId, f.name, f.path, f.finished, f.completed "
                                        "FROM folder f LEFT JOIN comic c ON (f.id = c.parentId) "
                                        "INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) WHERE ");

                try {
                    QueryParser parser;
                    auto result = parser.parse(filter.toStdString());
                    result.buildSqlString(queryString);

                    switch (modifier) {
                    case YACReader::NoModifiers:
                        queryString += " AND f.id <> 1 ORDER BY f.parentId,f.name";
                        break;

                    case YACReader::OnlyRead:
                        queryString += " AND f.id <> 1 AND ci.read = 1 ORDER BY f.parentId,f.name";
                        break;

                    case YACReader::OnlyUnread:
                        queryString += " AND f.id <> 1 AND ci.read = 0 ORDER BY f.parentId,f.name";
                        break;

                    default:
                        queryString += " AND f.id <> 1 ORDER BY f.parentId,f.name";
                        QLOG_ERROR() << "not implemented";
                        break;
                    }

                    selectQuery.prepare(queryString.c_str());
                    result.bindValues(selectQuery);

                    selectQuery.exec();
                    QLOG_DEBUG() << selectQuery.lastError() << "--";

                    setupFilteredModelData(selectQuery);
                } catch (const std::exception &e) {
                    //Do nothing, uncomplete search string will end here and it is part of how the QueryParser works
                    //I don't like the idea of using exceptions for this though
                }
            }

            connectionName = db.connectionName();
        }

        QSqlDatabase::removeDatabase(connectionName);
    });
}

void YACReader::FolderQueryResultProcessor::setupFilteredModelData(QSqlQuery &sqlquery)
{
    FolderItem *rootItem = 0;

    //inicializar el nodo ra�z
    QList<QVariant> rootData;
    rootData << "root";
    rootItem = new FolderItem(rootData);
    rootItem->id = ROOT;
    rootItem->parentItem = 0;

    FolderItem *parent = rootItem;

    QMap<unsigned long long int, FolderItem *> *filteredItems = new QMap<unsigned long long int, FolderItem *>();

    //add tree root node
    filteredItems->insert(parent->id, parent);

    QSqlRecord record = sqlquery.record();

    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int finished = record.indexOf("finished");
    int completed = record.indexOf("completed");
    int parentIdIndex = record.indexOf("parentId");

    while (sqlquery.next()) { //se procesan todos los folders que cumplen con el filtro
        //datos de la base de datos
        QList<QVariant> data;

        data << sqlquery.value(name).toString();
        data << sqlquery.value(path).toString();
        data << sqlquery.value(finished).toBool();
        data << sqlquery.value(completed).toBool();

        auto item = new FolderItem(data);
        item->id = sqlquery.value(0).toULongLong();

        //id del padre
        quint64 parentId = sqlquery.value(parentIdIndex).toULongLong();

        //se a�ade el item al map, de forma que se pueda encontrar como padre en siguientes iteraciones
        if (!filteredItems->contains(item->id))
            filteredItems->insert(item->id, item);

        //es necesario conocer las coordenadas de origen para poder realizar scroll autom�tico en la vista
        item->originalItem = model->items.value(item->id);

        //si el padre ya existe en el modelo, el item se a�ade como hijo
        if (filteredItems->contains(parentId))
            filteredItems->value(parentId)->appendChild(item);
        else //si el padre a�n no se ha a�adido, hay que a�adirlo a �l y todos los padres hasta el nodo ra�z
        {
            //comprobamos con esta variable si el �ltimo de los padres (antes del nodo ra�z) ya exist�a en el modelo
            bool parentPreviousInserted = false;

            //mientras no se alcance el nodo ra�z se procesan todos los padres (de abajo a arriba)
            while (parentId != ROOT) {
                //el padre no estaba en el modelo filtrado, as� que se rescata del modelo original
                FolderItem *parentItem = model->items.value(parentId);
                //se debe crear un nuevo nodo (para no compartir los hijos con el nodo original)
                FolderItem *newparentItem = new FolderItem(parentItem->getData()); //padre que se a�adir� a la estructura de directorios filtrados
                newparentItem->id = parentId;

                newparentItem->originalItem = parentItem;

                //si el modelo contiene al padre, se a�ade el item actual como hijo
                if (filteredItems->contains(parentId)) {
                    filteredItems->value(parentId)->appendChild(item);
                    parentPreviousInserted = true;
                }
                //sino se registra el nodo para poder encontrarlo con posterioridad y se a�ade el item actual como hijo
                else {
                    newparentItem->appendChild(item);
                    filteredItems->insert(newparentItem->id, newparentItem);
                    parentPreviousInserted = false;
                }

                //variables de control del bucle, se avanza hacia el nodo padre
                item = newparentItem;
                parentId = parentItem->parentItem->id;
            }

            //si el nodo es hijo de 1 y no hab�a sido previamente insertado como hijo, se a�ade como tal
            if (!parentPreviousInserted) {
                filteredItems->value(ROOT)->appendChild(item);
            } else {
                delete item;
            }
        }
    }

    emit newData(filteredItems, rootItem);
}
