#include "comic_query_result_processor.h"

#include "comic_model.h"
#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "search_query.h"

#include <QSqlRecord>

YACReader::ComicQueryResultProcessor::ComicQueryResultProcessor()
    : querySearchQueue(1)
{
}

void YACReader::ComicQueryResultProcessor::createModelData(const QString &filter, const QString &databasePath)
{
    querySearchQueue.cancelPending();

    querySearchQueue.enqueue([=, this] {
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
            try {
                auto query = comicsSearchQuery(db, filter);

                auto data = new QList<ComicItem>(ComicModel::sortedForFolder(ComicModel::createItems(query)));

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
