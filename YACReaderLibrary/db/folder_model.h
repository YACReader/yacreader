#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlDatabase>

#include "yacreader_global.h"
#include "folder_query_result_processor.h"

class FolderItem;

class FolderModelProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FolderModelProxy(QObject *parent = nullptr);
    ~FolderModelProxy() override;

    void setFilterData(QMap<unsigned long long, FolderItem *> *filteredItems, FolderItem *root);
    void clear();

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

protected:
    FolderItem *rootItem;
    QMap<unsigned long long int, FolderItem *> filteredItems; // relación entre folders

    bool filterEnabled;
};

class FolderModel : public QAbstractItemModel
{
    Q_OBJECT

    friend class FolderModelProxy;
    friend class YACReader::FolderQueryResultProcessor;

public:
    explicit FolderModel(QObject *parent = nullptr);
    explicit FolderModel(QSqlQuery &sqlquery, QObject *parent = nullptr);
    ~FolderModel() override;

    // QAbstractItemModel methods
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Convenience methods
    void setupModelData(QString path);
    QString getDatabase();
    QString getFolderPath(const QModelIndex &folder);
    // QModelIndex indexFromItem(FolderItem * item, int column);

    // bool isFilterEnabled(){return filterEnabled;};

    void updateFolderCompletedStatus(const QModelIndexList &list, bool status);
    void updateFolderFinishedStatus(const QModelIndexList &list, bool status);
    void updateFolderManga(const QModelIndexList &list, bool manga);

    QStringList getSubfoldersNames(const QModelIndex &mi);

    void fetchMoreFromDB(const QModelIndex &parent);

    QModelIndex addFolderAtParent(const QString &folderName, const QModelIndex &parent);

    enum Columns {
        Name = 0,
        Path = 1,
        Finished = 2,
        Completed = 3,
        Manga = 4
    }; // id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, name TEXT NOT NULL, path TEXT NOT NULL

    enum Roles {
        FinishedRole = Qt::UserRole + 1,
        CompletedRole,
        IdRole,
        MangaRole
    };

public slots:
    void deleteFolder(const QModelIndex &mi);
    void updateFolderChildrenInfo(qulonglong folderId);

private:
    void setupModelData(QSqlQuery &sqlquery, FolderItem *parent);
    void updateFolderModelData(QSqlQuery &sqlquery, FolderItem *parent);

    FolderItem *rootItem; // el árbol
    QMap<unsigned long long int, FolderItem *> items; // relación entre folders

    QString _databasePath;
};

#endif
