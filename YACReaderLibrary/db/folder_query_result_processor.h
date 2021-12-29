#ifndef FOLDER_QUERY_RESULT_PROCESSOR_H
#define FOLDER_QUERY_RESULT_PROCESSOR_H

#include <QObject>

#include "yacreader_global.h"
#include "concurrent_queue.h"

class FolderItem;
class FolderModel;
class QSqlQuery;

namespace YACReader {

class FolderQueryResultProcessor : public QObject
{
    Q_OBJECT
public:
    FolderQueryResultProcessor(FolderModel *model);

public slots:
    void createModelData(const QString &filter, bool includeComics);

signals:
    void newData(QMap<unsigned long long int, FolderItem *> *filteredItems, FolderItem *root);

private:
    ConcurrentQueue querySearchQueue;

    FolderModel *model;

    void setupFilteredModelData(QSqlQuery &sqlquery);
};
};

#endif // FOLDER_QUERY_RESULT_PROCESSOR_H
