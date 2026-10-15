#ifndef READING_LIST_H
#define READING_LIST_H

#include "library_ids.h"

#include <QString>
#include <QStringList>

#include <optional>

class QSqlQuery;
class QSqlRecord;

struct ReadingList {
    ReadingListId id;
    std::optional<ReadingListId> parentId;
    int ordering = 0;
    QString name;
    bool finished = false;
    bool completed = true;

    static const QStringList &columns();
    static ReadingList fromRecord(const QSqlRecord &record, const QString &alias);
    void bindTo(QSqlQuery &query) const;
};

struct Label {
    LabelId id;
    QString name;
    QString color;
    int ordering = 0;

    static const QStringList &columns();
    static Label fromRecord(const QSqlRecord &record, const QString &alias);
    void bindTo(QSqlQuery &query) const;
};

#endif // READING_LIST_H
