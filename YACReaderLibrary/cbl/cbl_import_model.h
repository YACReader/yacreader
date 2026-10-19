#ifndef CBL_IMPORT_MODEL_H
#define CBL_IMPORT_MODEL_H

#include "cbl_matcher.h"
#include "cbl_reader.h"

#include <QAbstractItemModel>
#include <QColor>
#include <QSortFilterProxyModel>

#include <functional>
#include <optional>

struct CblImportEntry {
    CblBook book;
    CblBookMatch match;
    // what the import uses; it starts as the accepted match
    std::optional<CblCandidate> choice;
    bool changed = false;
};

enum class CblImportStatus {
    Matched,
    Ambiguous,
    Missing,
    Changed,
};

// one row per entry; each entry has one child row, which holds its detail
class CblImportModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column {
        NumberColumn,
        BookColumn,
        ComicColumn,
        StatusColumn,
        ActionColumn,
        ColumnCount,
    };

    enum Roles {
        StatusRole = Qt::UserRole + 1,
    };

    explicit CblImportModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setEntries(const QList<CblImportEntry> &entries);
    const QList<CblImportEntry> &entries() const { return importEntries; }
    void setWarningColor(const QColor &color);

    static bool isDetail(const QModelIndex &index);
    CblImportStatus status(int row) const;
    int count(CblImportStatus status) const;
    // the entries whose choice is a comic with a file
    int withComicCount() const;
    // the Ambiguous and Missing entries, in order
    QList<int> problemRows() const;
    QList<std::optional<CblCandidate>> choices() const;

    void choose(int row, const CblCandidate &candidate);
    void useOriginalMatch(const QList<int> &rows);
    void importAsMissing(const QList<int> &rows);

    static QString bookTitle(const CblBook &book);
    static QString fileName(const CblLibraryComic &comic);

private:
    QString comicText(const CblImportEntry &entry) const;
    bool comicTextIsNote(const CblImportEntry &entry) const;
    void changeRows(const QList<int> &rows, const std::function<void(CblImportEntry &)> &change);

    QList<CblImportEntry> importEntries;
    QColor warningColor;
};

class CblImportFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit CblImportFilterModel(QObject *parent = nullptr);

    // nothing shows every status
    void setStatusFilter(std::optional<CblImportStatus> status);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    std::optional<CblImportStatus> statusFilter;
};

#endif
