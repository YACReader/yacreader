#ifndef COMIC_RECORD_H
#define COMIC_RECORD_H

#include "comic_db.h"
#include "list_entry.h"
#include "overloaded.h"
#include "placeholder.h"

#include <QFileInfo>

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

struct Placeholder {
    ::Placeholder placeholder;
};

}

using ComicRecord = std::variant<ComicRecordCase::Comic, ComicRecordCase::DeletedComic, ComicRecordCase::Placeholder>;

// the descriptive metadata, which every case has
inline const DescriptiveComicMetadata &recordMetadata(const ComicRecord &record)
{
    return std::visit(overloaded {
                              [](const ComicRecordCase::Comic &value) -> const DescriptiveComicMetadata & { return value.comic.info; },
                              [](const ComicRecordCase::DeletedComic &value) -> const DescriptiveComicMetadata & { return value.info; },
                              [](const ComicRecordCase::Placeholder &value) -> const DescriptiveComicMetadata & { return value.placeholder; },
                      },
                      record);
}

// nullptr for a placeholder, which has no comic_info
inline const ComicInfo *recordInfo(const ComicRecord &record)
{
    return std::visit(overloaded {
                              [](const ComicRecordCase::Comic &value) -> const ComicInfo * { return &value.comic.info; },
                              [](const ComicRecordCase::DeletedComic &value) -> const ComicInfo * { return &value.info; },
                              [](const ComicRecordCase::Placeholder &) -> const ComicInfo * { return nullptr; },
                      },
                      record);
}

inline ComicInfo *recordInfo(ComicRecord &record)
{
    return const_cast<ComicInfo *>(recordInfo(std::as_const(record)));
}

inline const Placeholder *recordPlaceholder(const ComicRecord &record)
{
    const auto *value = std::get_if<ComicRecordCase::Placeholder>(&record);
    return value != nullptr ? &value->placeholder : nullptr;
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

// the file name, or the last known one of a deleted comic; only for display and searches
inline QString recordFileName(const ComicRecord &record)
{
    return std::visit(overloaded {
                              [](const ComicRecordCase::Comic &value) { return QFileInfo(value.comic.file.path).fileName(); },
                              [](const ComicRecordCase::DeletedComic &value) { return value.lastKnownFile ? value.lastKnownFile->name : QString(); },
                              [](const ComicRecordCase::Placeholder &) { return QString(); },
                      },
                      record);
}

// the path in the library, or the last known one of a deleted comic; only for display and searches
inline QString recordPath(const ComicRecord &record)
{
    return std::visit(overloaded {
                              [](const ComicRecordCase::Comic &value) { return value.comic.file.path; },
                              [](const ComicRecordCase::DeletedComic &value) { return value.lastKnownFile ? value.lastKnownFile->path : QString(); },
                              [](const ComicRecordCase::Placeholder &) { return QString(); },
                      },
                      record);
}

#endif
