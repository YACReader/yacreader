#ifndef COMIC_QUERY_RESULT_PROCESSOR_H
#define COMIC_QUERY_RESULT_PROCESSOR_H

#include "comic_item.h"
#include "concurrent_queue.h"

#include <QSqlQuery>
#include <QtCore>

namespace YACReader {

class ComicQueryResultProcessor : public QObject
{
    Q_OBJECT
public:
    ComicQueryResultProcessor();

public slots:
    void createModelData(const QString &filter, const QString &databasePath);
signals:
    void newData(QList<ComicItem> *, const QString &);

private:
    ConcurrentQueue querySearchQueue;
};
};

#endif // COMIC_QUERY_RESULT_PROCESSOR_H
