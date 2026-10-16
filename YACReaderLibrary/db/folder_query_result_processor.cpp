#include "folder_query_result_processor.h"

#include "data_base_management.h"
#include "db_helper.h"
#include "folder_item.h"
#include "folder_model.h"
#include "search_query.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

// Copy/pasted from "folder_model.cpp"
#define ROOT 1

YACReader::FolderQueryResultProcessor::FolderQueryResultProcessor(FolderModel *model)
    : querySearchQueue(1), model(model)
{
}

void YACReader::FolderQueryResultProcessor::createModelData(const QString &filter)
{
    querySearchQueue.cancelPending();

    querySearchQueue.enqueue([=, this] {
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(model->getDatabase());
            try {
                auto query = foldersSearchQuery(db, filter);

                setupFilteredModelData(query);
            } catch (const std::exception &e) {
                // Do nothing, uncomplete search string will end here and it is part of how the QueryParser works
                // I don't like the idea of using exceptions for this though
            }

            connectionName = db.connectionName();
        }

        QSqlDatabase::removeDatabase(connectionName);
    });
}

void YACReader::FolderQueryResultProcessor::setupFilteredModelData(QSqlQuery &sqlquery)
{
    FolderItem *rootItem = 0;

    // inicializar el nodo ra�z
    Folder rootFolder;
    rootFolder.id = FolderId(ROOT);
    rootFolder.name = "root";
    rootItem = new FolderItem(rootFolder);
    rootItem->parentItem = 0;

    FolderItem *parent = rootItem;

    QMap<unsigned long long int, FolderItem *> *filteredItems = new QMap<unsigned long long int, FolderItem *>();

    // add tree root node
    filteredItems->insert(parent->folder.id.value(), parent);

    const auto indexes = Folder::columnIndexes(sqlquery.record(), "f");
    while (sqlquery.next()) {
        const auto folder = Folder::fromQuery(sqlquery, indexes);
        auto item = new FolderItem(folder);

        // id del padre
        quint64 parentId = folder.parentId.value();

        // se a�ade el item al map, de forma que se pueda encontrar como padre en siguientes iteraciones
        if (!filteredItems->contains(item->folder.id.value()))
            filteredItems->insert(item->folder.id.value(), item);

        // es necesario conocer las coordenadas de origen para poder realizar scroll autom�tico en la vista
        item->originalItem = model->items.value(item->folder.id.value());

        // si el padre ya existe en el modelo, el item se a�ade como hijo
        if (filteredItems->contains(parentId))
            filteredItems->value(parentId)->appendChild(item);
        else // si el padre a�n no se ha a�adido, hay que a�adirlo a �l y todos los padres hasta el nodo ra�z
        {
            // comprobamos con esta variable si el �ltimo de los padres (antes del nodo ra�z) ya exist�a en el modelo
            bool parentPreviousInserted = false;

            // mientras no se alcance el nodo ra�z se procesan todos los padres (de abajo a arriba)
            while (parentId != ROOT) {
                // el padre no estaba en el modelo filtrado, as� que se rescata del modelo original
                FolderItem *parentItem = model->items.value(parentId);
                // se debe crear un nuevo nodo (para no compartir los hijos con el nodo original)
                FolderItem *newparentItem = new FolderItem(parentItem->folder); // padre que se a�adir� a la estructura de directorios filtrados

                newparentItem->originalItem = parentItem;

                // si el modelo contiene al padre, se a�ade el item actual como hijo
                if (filteredItems->contains(parentId)) {
                    filteredItems->value(parentId)->appendChild(item);
                    parentPreviousInserted = true;
                }
                // sino se registra el nodo para poder encontrarlo con posterioridad y se a�ade el item actual como hijo
                else {
                    newparentItem->appendChild(item);
                    filteredItems->insert(newparentItem->folder.id.value(), newparentItem);
                    parentPreviousInserted = false;
                }

                // variables de control del bucle, se avanza hacia el nodo padre
                item = newparentItem;
                parentId = parentItem->parentItem->folder.id.value();
            }

            // si el nodo es hijo de 1 y no hab�a sido previamente insertado como hijo, se a�ade como tal
            if (!parentPreviousInserted) {
                filteredItems->value(ROOT)->appendChild(item);
            } else {
                delete item;
            }
        }
    }

    emit newData(filteredItems, rootItem);
}
