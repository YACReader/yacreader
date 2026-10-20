#ifndef SMART_LIST_H
#define SMART_LIST_H

#include "library_ids.h"

#include <QString>
#include <QStringList>

class QSqlQuery;
class QSqlRecord;

// a saved search: its comics are the comics that match `query` now
struct SmartList {
    SmartListId id;
    QString name;
    // in the search syntax of the database version; a change of the syntax rewrites it in the database update
    QString query;
    int ordering = 0;

    static const QStringList &columns();
    static QList<int> columnIndexes(const QSqlRecord &record, const QString &alias);
    static SmartList fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes);
};

#endif
