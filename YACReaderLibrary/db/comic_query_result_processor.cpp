#include "comic_query_result_processor.h"

#include "comic_item.h"
#include "comic_model.h"
#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "db_helper.h"
#include "query_parser.h"

#include "QsLog.h"

QString getLastExecutedQuery(const QSqlQuery &query)
{
    QString str = query.lastQuery();
    QMapIterator<QString, QVariant> it(query.boundValues());
    while (it.hasNext()) {
        it.next();
        str.replace(it.key(), it.value().toString());
    }
    return str;
}

YACReader::ComicQueryResultProcessor::ComicQueryResultProcessor()
    : querySearchQueue(1)
{
}

void YACReader::ComicQueryResultProcessor::createModelData(const YACReader::SearchModifiers modifier, const QString &filter, const QString &databasePath)
{
    querySearchQueue.cancelPending();

    querySearchQueue.enqueue([=] {
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
            QSqlQuery selectQuery(db);

            std::string queryString("SELECT ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened "
                                    "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) LEFT JOIN folder f ON (f.id == c.parentId) WHERE ");

            try {
                QueryParser parser;
                auto result = parser.parse(filter.toStdString());
                result.buildSqlString(queryString);

                switch (modifier) {
                case YACReader::NoModifiers:
                    queryString += " LIMIT :limit";
                    break;

                case YACReader::OnlyRead:
                    queryString += " AND ci.read = 1 LIMIT :limit";
                    break;

                case YACReader::OnlyUnread:
                    queryString += " AND ci.read = 0 LIMIT :limit";
                    break;

                default:
                    queryString += " LIMIT :limit";
                    QLOG_ERROR() << "not implemented";
                    break;
                }
                selectQuery.prepare(queryString.c_str());
                selectQuery.bindValue(":limit", 500); //TODO, load this value from settings
                result.bindValues(selectQuery);

                selectQuery.exec();

                auto data = modelData(selectQuery);

                emit newData(data, databasePath);
            } catch (const std::exception &e) {
                //Do nothing, uncomplete search string will end here and it is part of how the QueryParser works
                //I don't like the idea of using exceptions for this though
            }

            connectionName = db.connectionName();
        }
        QSqlDatabase::removeDatabase(connectionName);
    });
}

QList<ComicItem *> *YACReader::ComicQueryResultProcessor::modelData(QSqlQuery &sqlquery)
{
    auto list = new QList<ComicItem *>();

    int numColumns = sqlquery.record().count();

    while (sqlquery.next()) {
        QList<QVariant> data;

        for (int i = 0; i < numColumns; i++)
            data << sqlquery.value(i);

        list->append(new ComicItem(data));
    }

    std::sort(list->begin(), list->end(), [](const ComicItem *c1, const ComicItem *c2) {
        if (c1->data(ComicModel::Number).isNull() && c2->data(ComicModel::Number).isNull()) {
            return naturalSortLessThanCI(c1->data(ComicModel::FileName).toString(), c2->data(ComicModel::FileName).toString());
        } else {
            if (c1->data(ComicModel::Number).isNull() == false && c2->data(ComicModel::Number).isNull() == false) {
                return c1->data(ComicModel::Number).toInt() < c2->data(ComicModel::Number).toInt();
            } else {
                return c2->data(ComicModel::Number).isNull();
            }
        }
    });

    return list;
}
