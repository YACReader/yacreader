#include "table_projection.h"

#include <QSqlRecord>

namespace {

QString aliased(const QString &alias, const QString &column)
{
    return alias + "_" + column;
}

}

QString TableProjection::selectList(const QString &alias, const QStringList &columns)
{
    QStringList result;
    result.append(alias + ".id AS " + aliased(alias, "id"));
    for (const auto &column : columns) {
        result.append(alias + "." + column + " AS " + aliased(alias, column));
    }
    return result.join(", ");
}

QList<int> TableProjection::columnIndexes(const QSqlRecord &record, const QString &alias, const QStringList &columns)
{
    QList<int> indexes;
    indexes.reserve(columns.size() + 1);
    indexes.append(record.indexOf(aliased(alias, "id")));
    for (const auto &column : columns) {
        indexes.append(record.indexOf(aliased(alias, column)));
    }
    return indexes;
}

QString TableProjection::insertColumns(const QStringList &columns)
{
    return columns.join(", ");
}

QString TableProjection::insertValues(const QStringList &columns)
{
    QStringList result;
    for (const auto &column : columns) {
        result.append(":" + column);
    }
    return result.join(", ");
}

QString TableProjection::assignments(const QStringList &columns)
{
    QStringList result;
    for (const auto &column : columns) {
        result.append(column + " = :" + column);
    }
    return result.join(", ");
}
