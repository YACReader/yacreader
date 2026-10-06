#ifndef MISSING_COMIC_SERVER_HELPER_H
#define MISSING_COMIC_SERVER_HELPER_H

#include "comic_db.h"

class QSqlDatabase;

namespace MissingComicServerHelper {
bool isPlaceholderId(qulonglong id);
qulonglong placeholderId(qulonglong entryId, bool label);
ComicDB placeholder(qulonglong libraryId, const ComicDB &entry, bool label = false);
ComicDB resolve(qulonglong libraryId, qulonglong comicId);
ComicDB resolve(qulonglong libraryId, qulonglong comicId, QSqlDatabase &db);
QByteArray cover();
qulonglong idFromHash(const QString &hash);
QString description();
}

#endif
