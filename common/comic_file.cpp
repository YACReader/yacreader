#include "comic_file.h"

#include "table_projection.h"

#include <QSqlQuery>
#include <QSqlRecord>

const QStringList &ComicFile::columns()
{
    static const QStringList columns = { "parentId", "comicInfoId", "fileName", "path" };
    return columns;
}

namespace {

// `value(k)` is column k of {id} + ComicFile::columns()
template<typename Value>
ComicFile readComicFile(const Value &value)
{
    ComicFile file;
    file.id = ComicId(value(0).toULongLong());
    file.folderId = FolderId(value(1).toULongLong());
    file.comicInfoId = ComicInfoId(value(2).toULongLong());
    file.fileName = value(3).toString();
    file.path = value(4).toString();
    return file;
}

}

ComicFile ComicFile::fromRecord(const QSqlRecord &record, const QString &alias)
{
    const auto indexes = TableProjection::columnIndexes(record, alias, columns());
    return readComicFile([&](int column) { return record.value(indexes.at(column)); });
}

ComicFile ComicFile::fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes)
{
    return readComicFile([&](int column) { return query.value(columnIndexes.at(column)); });
}

void ComicFile::bindTo(QSqlQuery &query) const
{
    query.bindValue(":parentId", folderId.value());
    query.bindValue(":comicInfoId", comicInfoId.value());
    query.bindValue(":fileName", fileName);
    query.bindValue(":path", path);
}
