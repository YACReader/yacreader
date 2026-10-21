#ifndef GRID_CONTENT_MODEL_H
#define GRID_CONTENT_MODEL_H

#include <QAbstractListModel>
#include <QPersistentModelIndex>
#include <QUrl>

class ComicModel;
class FolderModel;
struct Folder;

// a folder or a list in the grid of a folder of lists
struct GridListCard {
    enum Kind {
        Folder,
        ReadingList,
        SmartList,
    };

    Kind kind = Folder;
    // the ReadingListModel::TypeList of its row in the sidebar
    int listType = 0;
    qulonglong id = 0;
    QString name;
    // empty when the card shows its type
    QUrl cover;
    QUrl icon;
};

class GridContentModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ItemKind {
        FolderItem = 0,
        ComicItem,
        SpacerItem,
        ListCardItem
    };
    Q_ENUM(ItemKind)

    enum Roles {
        ItemKindRole = Qt::UserRole + 1,
        SourceIndexRole,
        NumberRole,
        TitleRole,
        FileNameRole,
        NumPagesRole,
        IdRole,
        ReadRole,
        CurrentPageRole,
        RatingRole,
        HasBeenOpenedRole,
        CoverPathRole,
        AddedRole,
        TypeRole,
        ShowRecentRole,
        RecentRangeRole,
        UpdatedRole,
        FinishedRole,
        ReadableTitleRole,
        KindRole,
        AvailableRole,
        LastKnownNameRole,
        ListItemIdRole,
        ListIdRole,
        CardKindRole,
        IconPathRole,
    };

    explicit GridContentModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setComicModel(ComicModel *model);
    void setFolderModel(FolderModel *model, const QModelIndex &selectedFolderIndex);
    void clearFolderModel();
    // only these cards, with no folder or comic rows, until a folder model or a comic model is set
    void setListCards(const QList<GridListCard> &cards);
    bool hasListCards() const;
    bool isListCardRow(int viewRow) const;
    GridListCard listCardAt(int viewRow) const;
    void setMixFoldersAndComics(bool enabled);
    void setStartComicsOnNewRow(bool enabled);
    void setGridColumnCount(int columns);

    bool isFolderRow(int viewRow) const;
    bool isSpacerRow(int viewRow) const;
    int visibleFolderCount() const;
    int sourceComicRow(int viewRow) const;
    int viewRowForComicRow(int sourceRow) const;
    int viewRowForFolderId(qulonglong id) const;
    QModelIndex sourceFolderIndex(int viewRow) const;
    Folder folderAt(int viewRow) const;
    Q_INVOKABLE QUrl comicCoverUrlForHash(const QString &hash) const;

private:
    void reconnectModels();
    void resetFromSource();
    int sourceFolderCount() const;
    int spacerCount() const;
    bool forwardsFolderRowsDirectly() const;
    bool forwardsComicRowsDirectly() const;

    ComicModel *comicModel = nullptr;
    FolderModel *folderModel = nullptr;
    QPersistentModelIndex selectedFolderIndex;
    bool selectedFolderIsRoot = false;
    bool mixFoldersAndComics = true;
    bool startComicsOnNewRow = false;
    int gridColumnCount = 1;
    QList<QMetaObject::Connection> sourceConnections;
    bool showsListCards = false;
    QList<GridListCard> listCards;
};

#endif // GRID_CONTENT_MODEL_H
