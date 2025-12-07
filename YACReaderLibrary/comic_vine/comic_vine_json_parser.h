
#ifndef COMIC_VINE_JSON_PARSER_H
#define COMIC_VINE_JSON_PARSER_H

#include "comic_db.h"

struct SelectedVolumeInfo;

namespace YACReader {

ComicDB parseCVJSONComicInfo(ComicDB &comic, const QString &json, const SelectedVolumeInfo &volumeInfo);

}

#endif // COMIC_VINE_JSON_PARSER_H
