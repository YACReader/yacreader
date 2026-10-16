#ifndef __FOLDER_H
#define __FOLDER_H

#include "library_ids.h"
#include "yacreader_global.h"

#include <QString>
#include <QStringList>

class QSqlQuery;
class QSqlRecord;

struct Folder {
    FolderId id;
    FolderId parentId;
    QString name;
    QString path;

    bool finished = false; // finished means read, the user has read all the content in this folder
    bool completed = true; // completed means the folder has all the content, e.g. a series got its final issue and the user has collected all of them

    qint32 numChildren = 0; // the column is nullable, but NULL means 0
    QString firstChildHash; // empty for unknown first child
    QString customImage; // empty for none custom image

    YACReader::FileType type = YACReader::FileType::Comic;
    qint64 added = 0; // epoch in seconds
    qint64 updated = 0; // epoch in seconds

    friend bool operator==(const Folder &, const Folder &) = default;

    static const QStringList &columns();
    static Folder fromRecord(const QSqlRecord &record, const QString &alias);
    static QList<int> columnIndexes(const QSqlRecord &record, const QString &alias);
    static Folder fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes);
    void bindTo(QSqlQuery &query) const;
};

#endif
