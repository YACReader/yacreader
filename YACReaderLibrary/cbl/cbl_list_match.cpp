#include "cbl_list_match.h"

#include "QsLog.h"
#include "cbl_importer.h"
#include "db_helper.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace {

// the entry as it is now
struct StoredEntry {
    QVariant comicInfoId;
    QVariant placeholderId;
    bool hasFile = false;
    std::optional<EntrySource> source;
};

std::optional<StoredEntry> loadEntry(ListItemId id, QSqlDatabase &db)
{
    const bool label = id.listKind == ListKind::Label;
    QSqlQuery query(db);
    query.prepare(QString("SELECT e.comic_info_id, e.placeholder_id, EXISTS (SELECT 1 FROM comic WHERE comicInfoId = e.comic_info_id), %1 FROM %2 e WHERE e.id = :id")
                          .arg(label ? "NULL, NULL" : "e.source, e.source_format", label ? "label_entry" : "reading_list_entry"));
    query.bindValue(":id", id.id);
    if (!query.exec()) {
        QLOG_ERROR() << "Unable to read a list entry:" << query.lastError().text();
        return std::nullopt;
    }
    if (!query.next())
        return std::nullopt;

    StoredEntry entry;
    entry.comicInfoId = query.value(0);
    entry.placeholderId = query.value(1);
    entry.hasFile = query.value(2).toBool();
    if (!query.value(3).isNull())
        entry.source = EntrySource { query.value(3).toString(), query.value(4).toString() };
    return entry;
}

bool makeMissing(ListItemId id, const StoredEntry &entry, QSqlDatabase &db)
{
    Placeholder placeholder;
    const auto book = entry.source ? readCblBook(entry.source->text, entry.source->format) : std::nullopt;
    if (book) {
        placeholder = placeholderFromCblBook(*book);
    } else {
        const auto info = DBHelper::loadComicInfo(ComicInfoId(entry.comicInfoId.toULongLong()), db);
        if (!info)
            return false;
        static_cast<DescriptiveComicMetadata &>(placeholder) = *info;
    }

    const auto placeholderId = DBHelper::insertPlaceholder(placeholder, db);
    if (!placeholderId)
        return false;

    QSqlQuery update(db);
    update.prepare(QString("UPDATE %1 SET comic_info_id = NULL, placeholder_id = :placeholder_id, last_known_name = NULL, last_known_path = NULL WHERE id = :id")
                           .arg(id.listKind == ListKind::Label ? "label_entry" : "reading_list_entry"));
    update.bindValue(":placeholder_id", placeholderId->value());
    update.bindValue(":id", id.id);
    if (!update.exec() || update.numRowsAffected() != 1) {
        QLOG_ERROR() << "Unable to make an entry missing:" << update.lastError().text();
        return false;
    }
    return true;
}

}

CblListEntry cblListEntry(const ComicRecord &record, const ListEntry &entry)
{
    CblListEntry result;
    result.id = entry.id;
    result.listId = entry.listId;

    const auto &metadata = recordMetadata(record);
    if (const auto *placeholder = recordPlaceholder(record)) {
        CblLibraryPlaceholder current;
        static_cast<CblLibraryMetadata &>(current) = cblLibraryMetadata(metadata);
        current.id = placeholder->id;
        current.coverKey = placeholder->coverKey.value_or(QString());
        result.current = current;
    } else if (const auto *info = recordInfo(record)) {
        CblLibraryComic current;
        static_cast<CblLibraryMetadata &>(current) = cblLibraryMetadata(metadata);
        current.id = info->id;
        current.hash = info->hash;
        if (const auto *comic = recordComic(record)) {
            current.hasFile = true;
            current.path = comic->file.path;
        }
        result.current = current;
    }

    const auto book = entry.source ? readCblBook(entry.source->text, entry.source->format) : std::nullopt;
    result.book = book.value_or(cblBookOf(metadata));
    return result;
}

CblBook cblBookOf(const DescriptiveComicMetadata &metadata)
{
    CblBook book;
    book.series = metadata.series.toString();
    book.number = metadata.number.toString();
    book.volume = metadata.volume.toString();
    book.format = metadata.format.toString();
    if (const auto year = cblYearOfDate(metadata.date.toString()); year > 0)
        book.year = QString::number(year);
    if (const auto issueId = metadata.comicVineID.toString().trimmed(); !issueId.isEmpty())
        book.databases.append({ QStringLiteral("cv"), QString(), issueId });
    return book;
}

CblLibraryMetadata cblLibraryMetadata(const DescriptiveComicMetadata &metadata)
{
    CblLibraryMetadata result;
    result.series = metadata.series.toString();
    result.number = metadata.number.toString();
    result.volume = metadata.volume.toString();
    result.date = metadata.date.toString();
    result.format = metadata.format.toString();
    result.comicVineId = metadata.comicVineID.toString();
    return result;
}

bool applyCblListChanges(const QList<CblListChange> &changes, const QString &libraryDataPath, QSqlDatabase &db, ComicMatchFiles &files)
{
    for (const auto &change : changes) {
        const auto entry = loadEntry(change.entry, db);
        if (!entry)
            return false;

        bool applied = true;
        if (change.comic) {
            const auto comicInfoId = *change.comic;
            if (!entry->placeholderId.isNull())
                applied = matchPlaceholderToComic(PlaceholderId(entry->placeholderId.toULongLong()), comicInfoId, libraryDataPath, db, files);
            else if (entry->comicInfoId.toULongLong() == comicInfoId.value())
                continue;
            else if (!entry->hasFile)
                applied = matchDeletedComicToComic(ComicInfoId(entry->comicInfoId.toULongLong()), comicInfoId, db, files);
            else
                applied = giveComicToEntry(change.entry, comicInfoId, db);
        } else if (entry->placeholderId.isNull() && entry->hasFile) {
            applied = makeMissing(change.entry, *entry, db);
        }
        if (!applied)
            return false;
    }
    return true;
}
