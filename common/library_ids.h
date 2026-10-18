#ifndef LIBRARY_IDS_H
#define LIBRARY_IDS_H

#include <QHashFunctions>
#include <QtGlobal>

#include <compare>
#include <variant>

template<typename Tag>
class TableId
{
public:
    constexpr TableId() = default;
    constexpr explicit TableId(qulonglong value)
        : id(value)
    {
    }

    constexpr qulonglong value() const { return id; }

    friend constexpr bool operator==(TableId, TableId) = default;
    friend constexpr auto operator<=>(TableId, TableId) = default;

    friend size_t qHash(TableId key, size_t seed = 0) noexcept { return ::qHash(key.id, seed); }

private:
    qulonglong id = 0;
};

using FolderId = TableId<struct FolderIdTag>;
using ComicId = TableId<struct ComicIdTag>;
using ComicInfoId = TableId<struct ComicInfoIdTag>;
using ReadingListId = TableId<struct ReadingListIdTag>;
using LabelId = TableId<struct LabelIdTag>;
using PlaceholderId = TableId<struct PlaceholderIdTag>;

enum class ListKind {
    ReadingList,
    Label,
};

// an entry id is unique only in its own table (reading_list_entry or label_entry)
struct ListItemId {
    ListKind listKind;
    qulonglong id;

    friend constexpr bool operator==(const ListItemId &, const ListItemId &) = default;
    friend size_t qHash(const ListItemId &key, size_t seed = 0) noexcept { return qHashMulti(seed, static_cast<int>(key.listKind), key.id); }
};

// which row this is: a comic in folders and special lists, an entry in reading lists and labels
using ComicRowId = std::variant<ComicId, ListItemId>;

inline size_t qHash(const ComicRowId &key, size_t seed = 0) noexcept
{
    return std::visit([&key, seed](const auto &id) { return qHashMulti(seed, key.index(), id); }, key);
}

#endif
