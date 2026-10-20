#include "placeholder.h"

#include "table_projection.h"

#include <QSqlQuery>
#include <QSqlRecord>

const QStringList &Placeholder::columns()
{
    static const QStringList columns = [] {
        QStringList result;
        for (const auto &column : DescriptiveComicMetadata::columns())
            result.append(QString::fromLatin1(column.name));
        result.append("coverKey");
        return result;
    }();
    return columns;
}

QList<int> Placeholder::columnIndexes(const QSqlRecord &record, const QString &alias)
{
    return TableProjection::columnIndexes(record, alias, columns());
}

// `columnIndexes` are of {id} + Placeholder::columns()
Placeholder Placeholder::fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes)
{
    const auto value = [&](int column) { return query.value(columnIndexes.at(column)); };

    Placeholder placeholder;
    placeholder.id = PlaceholderId(value(0).toULongLong());

    int column = 1;
    for (const auto &descriptive : DescriptiveComicMetadata::columns()) {
        placeholder.*descriptive.member = value(column++);
    }

    const auto coverKey = value(column);
    if (!coverKey.isNull()) {
        placeholder.coverKey = coverKey.toString();
    }
    return placeholder;
}

bool Placeholder::hasSameValues(const Placeholder &other) const
{
    for (const auto &column : DescriptiveComicMetadata::columns()) {
        if (this->*column.member != other.*column.member)
            return false;
    }
    return id == other.id && coverKey == other.coverKey;
}

void Placeholder::bindTo(QSqlQuery &query) const
{
    DescriptiveComicMetadata::bindTo(query);
    query.bindValue(":coverKey", coverKey ? QVariant(*coverKey) : QVariant());
}
