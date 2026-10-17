#ifndef COMIC_RECORD_H
#define COMIC_RECORD_H

#include "comic_db.h"
#include "list_entry.h"
#include "overloaded.h"

#include <optional>
#include <utility>
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

inline const ComicInfo &recordInfo(const ComicRecord &record)
{
    return std::visit(overloaded {
                              [](const ComicRecordCase::Comic &value) -> const ComicInfo & { return value.comic.info; },
                              [](const ComicRecordCase::DeletedComic &value) -> const ComicInfo & { return value.info; },
                      },
                      record);
}

inline ComicInfo &recordInfo(ComicRecord &record)
{
    return const_cast<ComicInfo &>(recordInfo(std::as_const(record)));
}

// nullptr when the comic has no file
inline const ComicDB *recordComic(const ComicRecord &record)
{
    const auto *value = std::get_if<ComicRecordCase::Comic>(&record);
    return value != nullptr ? &value->comic : nullptr;
}

inline ComicDB *recordComic(ComicRecord &record)
{
    auto *value = std::get_if<ComicRecordCase::Comic>(&record);
    return value != nullptr ? &value->comic : nullptr;
}

#endif
