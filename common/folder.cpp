#include "folder.h"

#include "table_projection.h"

#include <QSqlQuery>
#include <QSqlRecord>

const QStringList &Folder::columns()
{
    static const QStringList columns = { "parentId", "name", "path", "finished", "completed", "numChildren",
                                         "firstChildHash", "customImage", "type", "added", "updated" };
    return columns;
}

Folder Folder::fromRecord(const QSqlRecord &record, const QString &alias)
{
    using TableProjection::value;

    Folder folder;
    folder.id = FolderId(value(record, alias, "id").toULongLong());
    folder.parentId = FolderId(value(record, alias, "parentId").toULongLong());
    folder.name = value(record, alias, "name").toString();
    folder.path = value(record, alias, "path").toString();
    folder.finished = value(record, alias, "finished").toBool();
    folder.completed = value(record, alias, "completed").toBool();
    const auto numChildren = value(record, alias, "numChildren");
    if (!numChildren.isNull()) {
        folder.numChildren = numChildren.toInt();
    }
    folder.firstChildHash = value(record, alias, "firstChildHash").toString();
    folder.customImage = value(record, alias, "customImage").toString();
    folder.type = value(record, alias, "type").value<YACReader::FileType>();
    folder.added = value(record, alias, "added").toLongLong();
    folder.updated = value(record, alias, "updated").toLongLong();
    return folder;
}

void Folder::bindTo(QSqlQuery &query) const
{
    query.bindValue(":parentId", parentId.value());
    query.bindValue(":name", name);
    query.bindValue(":path", path);
    query.bindValue(":finished", finished);
    query.bindValue(":completed", completed);
    query.bindValue(":numChildren", numChildren ? QVariant(*numChildren) : QVariant());
    query.bindValue(":firstChildHash", firstChildHash);
    query.bindValue(":customImage", customImage);
    query.bindValue(":type", static_cast<int>(type));
    query.bindValue(":added", added);
    query.bindValue(":updated", updated);
}
