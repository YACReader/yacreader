#ifndef LIBRARYCOMICOPENER_H
#define LIBRARYCOMICOPENER_H

#include "yacreader_global.h"

class ComicDB;
class QString;

namespace YACReader {

bool openComic(const ComicDB &comic,
               unsigned long long libraryId,
               const QString &path,
               OpenComicSource source);

bool openComicInThirdPartyApp(const QString &command,
                              const QString &path);

}

#endif // LIBRARYCOMICOPENER_H
