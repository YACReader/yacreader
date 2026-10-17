
#ifndef COMIC_VINE_JSON_PARSER_H
#define COMIC_VINE_JSON_PARSER_H

#include "comic_db.h"

struct SelectedVolumeInfo;

namespace YACReader {

// the descriptive fields that Comic Vine returned; the other fields keep their values
DescriptiveMetadataPatch parseCVJSONComicInfo(const QString &json, const SelectedVolumeInfo &volumeInfo);

}

#endif // COMIC_VINE_JSON_PARSER_H
