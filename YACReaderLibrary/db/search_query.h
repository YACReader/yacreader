
#ifndef SEARCHQUERY_H
#define SEARCHQUERY_H

#include <QSqlDatabase>

QSqlQuery foldersSearchQuery(QSqlDatabase &db, const QString &filter);
QSqlQuery comicsSearchQuery(QSqlDatabase &db, const QString &filter);

#endif // SEARCHQUERY_H
