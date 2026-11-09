#include "cbl_import_model.h"

#include <QFont>

#include <algorithm>

// the internal id of an entry is 0; the one of a detail is the row of its entry plus one
static bool isDetailIndex(const QModelIndex &index)
{
    return index.isValid() && index.internalId() != 0;
}

CblImportModel::CblImportModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex CblImportModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column < 0 || column >= ColumnCount)
        return { };
    if (!parent.isValid())
        return row >= 0 && row < importEntries.size() ? createIndex(row, column, quintptr(0)) : QModelIndex();
    if (isDetailIndex(parent) || row != 0)
        return { };
    return createIndex(0, column, quintptr(parent.row() + 1));
}

QModelIndex CblImportModel::parent(const QModelIndex &child) const
{
    if (!isDetailIndex(child))
        return { };
    return createIndex(int(child.internalId() - 1), 0, quintptr(0));
}

int CblImportModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return importEntries.size();
    return !isDetailIndex(parent) && parent.column() == 0 ? 1 : 0;
}

int CblImportModel::columnCount(const QModelIndex &) const
{
    return ColumnCount;
}

Qt::ItemFlags CblImportModel::flags(const QModelIndex &index) const
{
    if (isDetailIndex(index))
        return Qt::ItemIsEnabled;
    return QAbstractItemModel::flags(index);
}

// also for the indexes of a proxy model
bool CblImportModel::isDetail(const QModelIndex &index)
{
    return index.isValid() && index.parent().isValid();
}

QVariant CblImportModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || isDetailIndex(index) || index.row() >= importEntries.size())
        return { };

    const auto &entry = importEntries.at(index.row());
    const auto entryStatus = status(index.row());

    if (role == StatusRole)
        return int(entryStatus);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case NumberColumn:
            return index.row() + 1;
        case BookColumn:
            return bookTitle(entry.book);
        case ComicColumn:
            return comicText(entry);
        case StatusColumn:
            switch (entryStatus) {
            case CblImportStatus::Matched:
                return tr("Matched");
            case CblImportStatus::Ambiguous:
                return tr("Ambiguous");
            case CblImportStatus::Missing:
                return tr("Missing");
            case CblImportStatus::Changed:
                return tr("Changed");
            }
        }
        return { };
    }

    if (role == Qt::FontRole && index.column() == ComicColumn && comicTextIsNote(entry)) {
        QFont font;
        font.setItalic(true);
        return font;
    }

    if (role == Qt::ForegroundRole && index.column() == StatusColumn && warningColor.isValid() &&
        (entryStatus == CblImportStatus::Ambiguous || entryStatus == CblImportStatus::Missing))
        return warningColor;

    return { };
}

QVariant CblImportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return { };

    switch (section) {
    case NumberColumn:
        return tr("#");
    case BookColumn:
        return tr("In the file");
    case ComicColumn:
        return tr("Comic in your library");
    case StatusColumn:
        return tr("Status");
    case ActionColumn:
        return QString();
    }
    return { };
}

void CblImportModel::setEntries(const QList<CblImportEntry> &entries)
{
    beginResetModel();
    importEntries = entries;
    endResetModel();
}

void CblImportModel::setWarningColor(const QColor &color)
{
    warningColor = color;
    if (!importEntries.isEmpty())
        emit dataChanged(index(0, StatusColumn), index(int(importEntries.size()) - 1, StatusColumn), { Qt::ForegroundRole });
}

CblImportStatus CblImportModel::status(int row) const
{
    const auto &entry = importEntries.at(row);
    if (entry.changed)
        return CblImportStatus::Changed;
    if (entry.choice)
        return CblImportStatus::Matched;
    return entry.match.candidates.isEmpty() ? CblImportStatus::Missing : CblImportStatus::Ambiguous;
}

int CblImportModel::count(CblImportStatus status) const
{
    int result = 0;
    for (int row = 0; row < importEntries.size(); ++row) {
        if (this->status(row) == status)
            ++result;
    }
    return result;
}

int CblImportModel::withComicCount() const
{
    return std::count_if(importEntries.cbegin(), importEntries.cend(), [](const CblImportEntry &entry) {
        const auto *comic = entry.choice ? std::get_if<CblLibraryComic>(&*entry.choice) : nullptr;
        return comic != nullptr && comic->hasFile;
    });
}

QList<int> CblImportModel::problemRows() const
{
    QList<int> rows;
    for (int row = 0; row < importEntries.size(); ++row) {
        const auto rowStatus = status(row);
        if (rowStatus == CblImportStatus::Ambiguous || rowStatus == CblImportStatus::Missing)
            rows.append(row);
    }
    return rows;
}

QList<std::optional<CblCandidate>> CblImportModel::choices() const
{
    QList<std::optional<CblCandidate>> result;
    result.reserve(importEntries.size());
    for (const auto &entry : importEntries)
        result.append(entry.choice);
    return result;
}

void CblImportModel::choose(int row, const CblCandidate &candidate)
{
    changeRows({ row }, [&candidate](CblImportEntry &entry) {
        entry.choice = candidate;
        entry.changed = entry.match.accepted != entry.choice;
    });
}

void CblImportModel::useOriginalMatch(const QList<int> &rows)
{
    changeRows(rows, [](CblImportEntry &entry) {
        entry.choice = entry.match.accepted;
        entry.changed = false;
    });
}

void CblImportModel::importAsMissing(const QList<int> &rows)
{
    changeRows(rows, [](CblImportEntry &entry) {
        entry.choice.reset();
        // a book that found nothing is already missing, so nothing changes
        entry.changed = entry.match.accepted || !entry.match.candidates.isEmpty();
    });
}

QString CblImportModel::bookTitle(const CblBook &book)
{
    if (book.series.isEmpty())
        return book.fileName;

    auto title = book.series;
    if (!book.number.isEmpty())
        title += QStringLiteral(" #") + book.number;
    const auto &volume = book.volume.isEmpty() ? book.year : book.volume;
    if (!volume.isEmpty())
        title += QStringLiteral(" (") + volume + QStringLiteral(")");
    return title;
}

QString CblImportModel::fileName(const CblLibraryComic &comic)
{
    return comic.path.section('/', -1);
}

QString CblImportModel::comicText(const CblImportEntry &entry) const
{
    if (entry.choice) {
        const auto *comic = std::get_if<CblLibraryComic>(&*entry.choice);
        if (comic != nullptr && comic->hasFile)
            return fileName(*comic);
        return tr("Already missing in your library");
    }

    if (!entry.changed && !entry.match.candidates.isEmpty())
        return tr("%n possible comic(s)", "", entry.match.candidates.size());
    return tr("Not in library");
}

bool CblImportModel::comicTextIsNote(const CblImportEntry &entry) const
{
    const auto *comic = entry.choice ? std::get_if<CblLibraryComic>(&*entry.choice) : nullptr;
    return comic == nullptr || !comic->hasFile;
}

void CblImportModel::changeRows(const QList<int> &rows, const std::function<void(CblImportEntry &)> &change)
{
    for (const auto row : rows) {
        if (row < 0 || row >= importEntries.size())
            continue;
        change(importEntries[row]);
        emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
    }
}

CblImportFilterModel::CblImportFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterKeyColumn(-1);
}

void CblImportFilterModel::setStatusFilter(std::optional<CblImportStatus> status)
{
    statusFilter = status;
    invalidateRowsFilter();
}

bool CblImportFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (sourceParent.isValid())
        return true;
    if (statusFilter && sourceModel()->index(sourceRow, 0, sourceParent).data(CblImportModel::StatusRole).toInt() != int(*statusFilter))
        return false;
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
