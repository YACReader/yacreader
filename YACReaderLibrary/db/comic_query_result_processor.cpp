#include "comic_query_result_processor.h"

#include "comic_item.h"
#include "comic_model.h"
#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "search_query.h"

YACReader::ComicQueryResultProcessor::ComicQueryResultProcessor()
    : querySearchQueue(1)
{
}

void YACReader::ComicQueryResultProcessor::createModelData(const QString &filter, const QString &databasePath)
{
    querySearchQueue.cancelPending();

    querySearchQueue.enqueue([=] {
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
            try {
                auto query = comicsSearchQuery(db, filter);

                auto data = modelData(query);

                emit newData(data, databasePath);
            } catch (const std::exception &e) {
                // Do nothing, uncomplete search string will end here and it is part of how the QueryParser works
                // I don't like the idea of using exceptions for this though
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
