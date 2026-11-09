#ifndef COMIC_FILE_H
#define COMIC_FILE_H

#include "library_ids.h"

#include <QString>
#include <QStringList>

class QSqlQuery;
class QSqlRecord;

struct ComicFile {
    ComicId id;
    FolderId folderId; // column parentId
    ComicInfoId comicInfoId;
    QString fileName;
    QString path;

    friend bool operator==(const ComicFile &, const ComicFile &) = default;

    static const QStringList &columns();
    static ComicFile fromRecord(const QSqlRecord &record, const QString &alias);
    static ComicFile fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes);
    void bindTo(QSqlQuery &query) const;
};

#endif
