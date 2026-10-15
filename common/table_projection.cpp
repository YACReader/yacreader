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

QVariant TableProjection::value(const QSqlRecord &record, const QString &alias, const QString &column)
{
    return record.value(aliased(alias, column));
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
