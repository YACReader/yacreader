
#ifndef COMIC_VINE_JSON_PARSER_H
#define COMIC_VINE_JSON_PARSER_H

#include "comic_db.h"

struct SelectedVolumeInfo;

namespace YACReader {

// the descriptive fields that Comic Vine returned; the other fields keep their values
DescriptiveMetadataPatch parseCVJSONComicInfo(const QString &json, const SelectedVolumeInfo &volumeInfo);

enum class ComicVineAnswer {
    Answered,
    Failed,
    // Comic Vine does not document its limit answer; clients report status code 107
    LimitReached,
};
// a request fails when the network fails or Comic Vine gives a status other than 1
ComicVineAnswer comicVineAnswer(bool error, bool timeout, const QByteArray &body);

}

#endif // COMIC_VINE_JSON_PARSER_H
