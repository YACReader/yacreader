#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include "comic_item.h"
#include "comic_record.h"
#include "yacreader_global.h"

#include <QAbstractItemModel>
#include <QHash>
#include <QModelIndex>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QUrl>
#include <QVariant>

#include <optional>

using namespace YACReader;

class ComicModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Columns {
        Number = 0,
        Title,
        FileName,
        NumPages,
        Id,
        Parent_Id,
        Path,
        Hash,
        ReadColumn,
        CurrentPage,
        Rating,
        HasBeenOpened,
        PublicationDate,
        Added,
        Type,
        LastTimeOpened,
        Series,
        Volume,
        StoryArc,
    };

    enum CalculatedColumns {
        Size = Columns::StoryArc + 1,
        State,
    };

    enum Roles {
        NumberRole = Qt::UserRole + 1,
        TitleRole,
        FileNameRole,
        NumPagesRole,
        IdRole,
        Parent_IdRole,
        PathRole,
        HashRole,
        ReadColumnRole,
        CurrentPageRole,
        RatingRole,
        HasBeenOpenedRole,
        CoverPathRole,
        PublicationDateRole,
        ReadableTitle,
        AddedRole,
        TypeRole,
        ShowRecentRole,
        RecentRangeRole,
        SizeRole,
        SeriesRole,
        VolumeRole,
        StoryArcRole,
        KindRole,
        AvailableRole,
        LastKnownNameRole,
        ListItemIdRole,
        ListIdRole,
    };

    enum Mode {
        Folder,
        Favorites,
        Reading,
        Recent,
        Label,
        ReadingList,
        SearchResult
    };

public:
    explicit ComicModel(QObject *parent = nullptr);
    ~ComicModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    bool canBeResorted();
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QStringList mimeTypes() const override;

    void setupFolderModelData(unsigned long long int parentFolder, const QString &databasePath);
    void setupLabelModelData(unsigned long long int parentLabel, const QString &databasePath);
    void setupReadingListModelData(unsigned long long int parentReadingList, const QString &databasePath);
    void setupFavoritesModelData(const QString &databasePath);
    void setupReadingModelData(const QString &databasePath);
    void setupRecentModelData(const QString &databasePath);

    // Métodos de conveniencia
    QStringList getPaths(const QString &_source);
    QList<bool> getUnavailableList() const;
    QString getComicPath(QModelIndex mi); // empty for a row without a file
    QString getCurrentPath() { return QString(_databasePath).remove("/.yacreaderlibrary"); }
    // the comic of a row with a file, loaded again; nothing for a deleted comic
    std::optional<ComicDB> getComic(const QModelIndex &mi);
    // from the row, without a query
    ComicRecord record(const QModelIndex &index) const;
    // nothing for a row that is not in a list
    std::optional<ListEntry> listEntry(const QModelIndex &index) const;
    QVector<YACReaderComicReadStatus> getReadList();
    // the comics of the rows that have a file, loaded again; rows without a file are skipped
    QList<ComicDB> getComics(QList<QModelIndex> list);
    bool supports(const QModelIndex &index, ComicAction action) const;
    bool hasUnavailableRows() const;
    QModelIndexList supportedIndexes(const QModelIndexList &indexes, ComicAction action) const;
    // each comic_info of the rows once, loaded again, in row order
    void deleteMetadata(const QModelIndexList &indexes);
    // from the rows, without a query: the rows are as fresh as the last load
    std::optional<ComicDB> firstUnreadComic() const;
    QModelIndex getIndexFromId(quint64 id); // the first row with that file
    ComicRowId rowId(const QModelIndex &index) const;
    // an invalid index when the row is not in the model
    QModelIndex indexOf(const ComicRowId &rowId) const;
    QModelIndexList indexesOf(const QList<ComicRowId> &rowIds) const;
    QList<QModelIndex> getIndexesFromIds(const QList<qulonglong> &comicIds);
    QVector<YACReaderComicReadStatus> setComicsRead(QList<QModelIndex> list, YACReaderComicReadStatus read);
    void setComicsType(QList<QModelIndex> list, FileType type);
    ComicRowId asignNumbers(QList<QModelIndex> list, int startingNumber);
    // void remove(ComicDB * comic, int row);
    void removeInTransaction(int row);
    void reloadContinueReading();
    void reload();
    void reload(const ComicDB &comic);
    void resetComicRating(const QModelIndex &mi);
    void notifyCoverChange(ComicInfoId id);
    // the rows of the placeholder show its new cover
    void notifyPlaceholderCoverChange(PlaceholderId id, const std::optional<QString> &coverKey);

    Q_INVOKABLE QUrl getCoverUrlPathForComicHash(const QString &hash) const;

    void addComicsToFavorites(const QList<QModelIndex> &comicsList);
    void addComicsToLabel(const QList<QModelIndex> &comicsList, qulonglong labelId);
    void addComicsToReadingList(const QList<QModelIndex> &comicsList, qulonglong readingListId);

    void deleteComicsFromFavorites(const QList<QModelIndex> &comicsList);
    void deleteComicsFromReading(const QList<QModelIndex> &comicsList);
    void deleteComicsFromSpecialList(const QList<QModelIndex> &comicsList, qulonglong specialListId);
    void deleteComicsFromLists(const QList<QModelIndex> &comicsList);

    void deleteComicsFromModel(const QList<QModelIndex> &comicsList);

    bool isFavorite(const QModelIndex &index);

    ComicModel::Mode getMode() { return mode; }
    unsigned long long int getSourceId() { return sourceId; }

    void setShowRecent(bool visible);
    void setRecentRange(int days);

    QHash<int, QByteArray> roleNames() const override;

    static QList<ComicItem> createItems(QSqlQuery &query);
    static QList<ComicItem> createItems(QSqlQuery &query, ListKind listKind);
    static QList<ComicItem> sortedForFolder(QList<ComicItem> items);
    // by series, volume, number, file name, and path, in natural order
    static QList<ComicItem> sortedForSearch(QList<ComicItem> items);

public slots:
    void remove(int row);
    void startTransaction();
    void finishTransaction();
    void updateRating(int rating, QModelIndex mi);

    void addComicsToFavorites(const QList<qulonglong> &comicIds);
    void addComicsToLabel(const QList<ComicRowId> &rows, qulonglong labelId);
    void addComicsToReadingList(const QList<ComicRowId> &rows, qulonglong readingListId);

    void setModelData(QList<ComicItem> *data, const QString &databasePath);

protected:
private:
    QHash<quint64, quint64> coverRevisions; // by comic_info id

    QList<ComicItem> createFolderModelData(unsigned long long parentLabel, const QString &databasePath) const;
    QList<ComicItem> createLabelModelData(unsigned long long parentLabel, const QString &databasePath) const;
    QList<ComicItem> createReadingListData(unsigned long long parentReadingList, const QString &databasePath, bool &enableResorting) const;
    QList<ComicItem> createFavoritesModelData(const QString &databasePath) const;
    QList<ComicItem> createReadingModelData(const QString &databasePath) const;
    QList<ComicItem> createRecentModelData(const QString &databasePath) const;

    void takeData(const QList<ComicItem> &data);
    void takeUpdatedData(const QList<ComicItem> &updatedData, std::function<bool(const ComicItem &, const ComicItem &)> comparator);
    void updateRowsOf(ComicInfoId id, const std::function<void(ComicInfo &)> &change, int firstColumn, int lastColumn, const QList<int> &roles);
    QList<ComicItem> _data;

    QString _databasePath;
    QString _databaseConnection;

    bool enableResorting;
    Mode mode;
    qulonglong sourceId;
    QString localizedDate(const QString &dbDate) const;

    bool showRecent;
    qlonglong recentDays;

signals:
    void isEmpty();
    void favoritesChanged(const QList<qulonglong> &comicIds);
    void resortedIndexes(QList<int>);
    void newSelectedIndex(const QModelIndex &);
};

#endif
