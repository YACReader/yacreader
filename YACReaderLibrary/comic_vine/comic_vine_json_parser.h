
#ifndef COMIC_VINE_JSON_PARSER_H
#define COMIC_VINE_JSON_PARSER_H

#include "comic_db.h"

namespace YACReader {

ComicDB parseCVJSONComicInfo(ComicDB &comic, const QString &json, int count, const QString &publisher);

}

#endif // COMIC_VINE_JSON_PARSER_H
