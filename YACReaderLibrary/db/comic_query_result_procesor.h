#ifndef COMIC_QUERY_RESULT_PROCESOR_H
#define COMIC_QUERY_RESULT_PROCESOR_H

#include <QtCore>
#include <QSqlQuery>

#include "yacreader_global_gui.h"
#include "concurrent_queue.h"

class ComicItem;

namespace YACReader {

class ComicQueryResultProcesor : public QObject
{
    Q_OBJECT
public:
    ComicQueryResultProcesor();

public slots:
    void createModelData(const SearchModifiers modifier, const QString &filter, const QString &databasePath);
signals:
    void newData(QList<ComicItem *> *, const QString &);

private:
    ConcurrentQueue querySearchQueue;

    QList<ComicItem *> *modelData(QSqlQuery &sqlquery);
};
};

#endif // COMIC_QUERY_RESULT_PROCESOR_H
