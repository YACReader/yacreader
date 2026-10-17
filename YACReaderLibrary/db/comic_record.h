#ifndef COMIC_RECORD_H
#define COMIC_RECORD_H

#include "comic_db.h"
#include "list_entry.h"

#include <optional>
#include <variant>

// what the info panel and the dialogs show: the comic without its list or location data
namespace ComicRecordCase {

struct Comic {
    ComicDB comic;
};

struct DeletedComic {
    ComicInfo info;
    std::optional<LastKnownFile> lastKnownFile;
};

}

using ComicRecord = std::variant<ComicRecordCase::Comic, ComicRecordCase::DeletedComic>;

#endif
