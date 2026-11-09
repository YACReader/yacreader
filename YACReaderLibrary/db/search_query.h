
#ifndef SEARCHQUERY_H
#define SEARCHQUERY_H

#include <QSqlDatabase>

QSqlQuery foldersSearchQuery(QSqlDatabase &db, const QString &filter);
// the first 500 comics that match; throws when the filter cannot be parsed
QSqlQuery comicsSearchQuery(QSqlDatabase &db, const QString &filter);
// the first 500 comics of the folder that match; throws when the filter cannot be parsed
QSqlQuery comicsSearchQuery(QSqlDatabase &db, const QString &filter, qulonglong folderId);
// every comic that matches, for a smart list; throws when the filter cannot be parsed
QSqlQuery allComicsSearchQuery(QSqlDatabase &db, const QString &filter);

#endif // SEARCHQUERY_H
