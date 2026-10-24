#include "comic_item.h"

#include "db_helper.h"
#include "overloaded.h"
#include "qnaturalsorting.h"

#include <QSqlQuery>
#include <QSqlRecord>

#include <algorithm>
#include <numeric>
#include <optional>
#include <utility>

const DescriptiveComicMetadata &rowMetadata(const ComicItem &item)
{
    return std::visit(overloaded {
                              [](const ComicItemCase::Comic &row) -> const DescriptiveComicMetadata & { return row.comic.info; },
                              [](const ComicItemCase::ComicInList &row) -> const DescriptiveComicMetadata & { return row.comic.info; },
                              [](const ComicItemCase::DeletedComicInList &row) -> const DescriptiveComicMetadata & { return row.info; },
                              [](const ComicItemCase::PlaceholderInList &row) -> const DescriptiveComicMetadata & { return row.placeholder; },
                              [](const ComicItemCase::PlaceholderInFolder &row) -> const DescriptiveComicMetadata & { return row.placeholder; },
                      },
                      item);
}

const ComicDB *rowComic(const ComicItem &item)
{
    return std::visit(overloaded {
                              [](const ComicItemCase::Comic &row) -> const ComicDB * { return &row.comic; },
                              [](const ComicItemCase::ComicInList &row) -> const ComicDB * { return &row.comic; },
                              [](const ComicItemCase::DeletedComicInList &) -> const ComicDB * { return nullptr; },
                              [](const ComicItemCase::PlaceholderInList &) -> const ComicDB * { return nullptr; },
                              [](const ComicItemCase::PlaceholderInFolder &) -> const ComicDB * { return nullptr; },
                      },
                      item);
}

QString rowFileName(const ComicItem &item)
{
    return std::visit(overloaded {
                              [](const ComicItemCase::Comic &row) { return row.comic.file.fileName; },
                              [](const ComicItemCase::ComicInList &row) { return row.comic.file.fileName; },
                              [](const ComicItemCase::DeletedComicInList &row) { return row.entry.lastKnownFile ? row.entry.lastKnownFile->name : QString(); },
                              [](const ComicItemCase::PlaceholderInList &) { return QString(); },
                              [](const ComicItemCase::PlaceholderInFolder &) { return QString(); },
                      },
                      item);
}

namespace {

int sign(int order)
{
    return order < 0 ? -1 : (order > 0 ? 1 : 0);
}

// -1, 0, or 1 in natural order without case; an empty text is first when `emptyFirst`, else last
int compareNatural(const QString &a, const QString &b, bool emptyFirst)
{
    if (a.isEmpty() != b.isEmpty())
        return a.isEmpty() == emptyFirst ? -1 : 1;
    return sign(naturalCompare(a, b, Qt::CaseInsensitive));
}

// the order of comicNumberLessThan for two numbers without file names: numbered first
int compareNumbers(const std::optional<QString> &a, const std::optional<QString> &b)
{
    if (a && b)
        return sign(naturalCompare(*a, *b, Qt::CaseInsensitive));
    if (a.has_value() == b.has_value())
        return 0;
    return a ? -1 : 1;
}

struct SearchSortKey {
    QString series;
    QString volume;
    std::optional<QString> number;
    QString fileName;
    QString path;
};

SearchSortKey searchSortKey(const ComicItem &item)
{
    const auto &metadata = rowMetadata(item);
    const auto *comic = rowComic(item);
    return {
        metadata.series.toString().trimmed(),
        metadata.volume.toString().trimmed(),
        metadata.number.isNull() ? std::nullopt : std::optional(metadata.number.toString()),
        rowFileName(item),
        comic != nullptr ? comic->file.path : QString(),
    };
}

int compareSearchSortKeys(const SearchSortKey &a, const SearchSortKey &b)
{
    if (const auto order = compareNatural(a.series, b.series, false); order != 0)
        return order;
    if (const auto order = compareNatural(a.volume, b.volume, true); order != 0)
        return order;
    if (const auto order = compareNumbers(a.number, b.number); order != 0)
        return order;
    if (const auto order = compareNatural(a.fileName, b.fileName, true); order != 0)
        return order;
    return compareNatural(a.path, b.path, true);
}

}

namespace ComicItems {

QList<ComicItem> fromQuery(QSqlQuery &query)
{
    QList<ComicItem> items;
    const auto indexes = ComicDB::columnIndexes(query.record(), "c", "ci");
    while (query.next()) {
        items.append(ComicItemCase::Comic { ComicDB::fromQuery(query, indexes) });
    }
    return items;
}

QList<ComicItem> fromQuery(QSqlQuery &query, ListKind listKind)
{
    QList<ComicItem> items;
    const auto record = query.record();
    const auto indexes = ComicDB::columnIndexes(record, "c", "ci");
    const auto entryIndexes = ListEntry::columnIndexes(record, "e", listKind);
    const auto placeholderIndexes = Placeholder::columnIndexes(record, "p");
    const auto fileId = indexes.file.first();
    const auto placeholderId = placeholderIndexes.first();
    while (query.next()) {
        auto entry = ListEntry::fromQuery(query, entryIndexes, listKind);
        if (!query.value(placeholderId).isNull())
            items.append(ComicItemCase::PlaceholderInList { Placeholder::fromQuery(query, placeholderIndexes), std::move(entry) });
        else if (query.value(fileId).isNull())
            items.append(ComicItemCase::DeletedComicInList { ComicInfo::fromQuery(query, indexes.info), std::move(entry) });
        else
            items.append(ComicItemCase::ComicInList { ComicDB::fromQuery(query, indexes), std::move(entry) });
    }
    return items;
}

QList<ComicItem> loadFolder(FolderId folderId, QSqlDatabase &db)
{
    QSqlQuery selectQuery(db);
    selectQuery.setForwardOnly(true);
    selectQuery.prepare("SELECT " + ComicDB::selectList("c", "ci") + " "
                                                                     "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                                                                     "WHERE c.parentId = :parentId");
    selectQuery.bindValue(":parentId", folderId.value());
    selectQuery.exec();

    auto items = fromQuery(selectQuery);
    if (db.isOpen()) {
        for (auto &placeholder : DBHelper::loadFolderPlaceholders(folderId, db).value_or(QList<Placeholder>()))
            items.append(ComicItemCase::PlaceholderInFolder { std::move(placeholder), folderId });
    }
    std::sort(items.begin(), items.end(), folderLessThan);
    return items;
}

bool folderLessThan(const ComicItem &c1, const ComicItem &c2)
{
    return comicNumberLessThan(rowMetadata(c1).number, rowFileName(c1), rowMetadata(c2).number, rowFileName(c2));
}

bool searchLessThan(const ComicItem &c1, const ComicItem &c2)
{
    return compareSearchSortKeys(searchSortKey(c1), searchSortKey(c2)) < 0;
}

QList<ComicItem> sortedForFolder(QList<ComicItem> items)
{
    std::sort(items.begin(), items.end(), folderLessThan);
    return items;
}

QList<ComicItem> sortedForSearch(QList<ComicItem> items)
{
    QList<SearchSortKey> keys;
    keys.reserve(items.size());
    for (const auto &item : std::as_const(items))
        keys.append(searchSortKey(item));

    QList<qsizetype> order(items.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&keys](qsizetype a, qsizetype b) { return compareSearchSortKeys(keys.at(a), keys.at(b)) < 0; });

    QList<ComicItem> sorted;
    sorted.reserve(items.size());
    for (const auto row : std::as_const(order))
        sorted.append(std::move(items[row]));
    return sorted;
}

}
