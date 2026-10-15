#include "comic_file.h"

#include "table_projection.h"

#include <QSqlQuery>
#include <QSqlRecord>

const QStringList &ComicFile::columns()
{
    static const QStringList columns = { "parentId", "comicInfoId", "fileName", "path" };
    return columns;
}

ComicFile ComicFile::fromRecord(const QSqlRecord &record, const QString &alias)
{
    using TableProjection::value;

    ComicFile file;
    file.id = ComicId(value(record, alias, "id").toULongLong());
    file.folderId = FolderId(value(record, alias, "parentId").toULongLong());
    file.comicInfoId = ComicInfoId(value(record, alias, "comicInfoId").toULongLong());
    file.fileName = value(record, alias, "fileName").toString();
    file.path = value(record, alias, "path").toString();
    return file;
}

void ComicFile::bindTo(QSqlQuery &query) const
{
    query.bindValue(":parentId", folderId.value());
    query.bindValue(":comicInfoId", comicInfoId.value());
    query.bindValue(":fileName", fileName);
    query.bindValue(":path", path);
}
