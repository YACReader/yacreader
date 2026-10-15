#ifndef TABLE_PROJECTION_H
#define TABLE_PROJECTION_H

#include <QString>
#include <QStringList>
#include <QVariant>

class QSqlRecord;

namespace TableProjection {

// `columns` are the data columns of a table; every table here has its key in `id`.
QString selectList(const QString &alias, const QStringList &columns);
QVariant value(const QSqlRecord &record, const QString &alias, const QString &column);

QString insertColumns(const QStringList &columns);
QString insertValues(const QStringList &columns);
QString assignments(const QStringList &columns);

}

#endif
