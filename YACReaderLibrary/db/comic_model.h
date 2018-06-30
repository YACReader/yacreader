#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QUrl>

#include "yacreader_global_gui.h"

class ComicDB;

class ComicItem;

using namespace YACReader;

//! [0]
class ComicModel : public QAbstractItemModel
{
	Q_OBJECT

public:
    enum Columns {
        Number = 0,
        Title = 1,
        FileName = 2,
        NumPages = 3,
        Id = 4,
        Parent_Id = 5,
        Path = 6,
        Hash = 7,
        ReadColumn = 8,
        IsBis = 9,
        CurrentPage = 10,
        Rating = 11,
        HasBeenOpened = 12
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
        IsBisRole,
        CurrentPageRole,
        RatingRole,
        HasBeenOpenedRole,
        CoverPathRole

    };

    enum Mode {
        Folder,
        Favorites,
        Reading,
        Label,
        ReadingList
    };


public:
    ComicModel(QObject *parent = 0);
    ComicModel( QSqlQuery &sqlquery, QObject *parent = 0);
    ~ComicModel();

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
					  const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    bool canBeResorted();
    QMimeData * mimeData(const QModelIndexList &indexes) const;
    QStringList mimeTypes() const;

    void setupFolderModelData(unsigned long long int parentFolder,const QString & databasePath);
    void setupLabelModelData(unsigned long long int parentLabel, const QString & databasePath);
    void setupReadingListModelData(unsigned long long int parentReadingList, const QString & databasePath);
    void setupFavoritesModelData(const QString & databasePath);
    void setupReadingModelData(const QString & databasePath);
    //configures the model for showing the comics matching the filter criteria.
    void setupModelData(const SearchModifiers modifier, const QString & filter, const QString & databasePath);

	//Métodos de conveniencia
	QStringList getPaths(const QString & _source);
	QString getComicPath(QModelIndex mi);
    QString getCurrentPath(){return QString(_databasePath).remove("/.yacreaderlibrary");}
	ComicDB getComic(const QModelIndex & mi); //--> para la edición
    //ComicDB getComic(int row);
	QVector<YACReaderComicReadStatus> getReadList();
	QVector<YACReaderComicReadStatus> setAllComicsRead(YACReaderComicReadStatus readStatus);
	QList<ComicDB> getComics(QList<QModelIndex> list); //--> recupera la información común a los comics seleccionados
	QList<ComicDB> getAllComics();
	QModelIndex getIndexFromId(quint64 id);
    QList<QModelIndex> getIndexesFromIds(const QList<qulonglong> &comicIds);
	//setcomicInfo(QModelIndex & mi); --> inserta en la base datos
	//setComicInfoForAllComics(); --> inserta la información común a todos los cómics de una sola vez.
	//setComicInfoForSelectedComis(QList<QModelIndex> list); -->inserta la información común para los comics seleccionados
	QVector<YACReaderComicReadStatus> setComicsRead(QList<QModelIndex> list,YACReaderComicReadStatus read);
	qint64 asignNumbers(QList<QModelIndex> list,int startingNumber);
    //void remove(ComicDB * comic, int row);
	void removeInTransaction(int row);
	void reload(const ComicDB & comic);
    void resetComicRating(const QModelIndex & mi);

    Q_INVOKABLE QUrl getCoverUrlPathForComicHash(const QString& hash) const;


    void addComicsToFavorites(const QList<QModelIndex> &comicsList);
    void addComicsToLabel(const QList<QModelIndex> &comicsList, qulonglong labelId);
    void addComicsToReadingList(const QList<QModelIndex> &comicsList, qulonglong readingListId);

    void deleteComicsFromFavorites(const QList<QModelIndex> &comicsList);
    void deleteComicsFromLabel(const QList<QModelIndex> &comicsList, qulonglong labelId);
    void deleteComicsFromReadingList(const QList<QModelIndex> &comicsList, qulonglong readingListId);

    void deleteComicsFromModel(const QList<QModelIndex> &comicsList);

    bool isFavorite(const QModelIndex &index);

    ComicModel::Mode getMode() {return mode;}

    QHash<int, QByteArray> roleNames() const;

public slots:
	void remove(int row);
	void startTransaction();
	void finishTransaction();
	void updateRating(int rating, QModelIndex mi);

    void addComicsToFavorites(const QList<qulonglong> &comicIds);
    void addComicsToLabel(const QList<qulonglong> &comicIds, qulonglong labelId);
    void addComicsToReadingList(const QList<qulonglong> &comicIds, qulonglong readingListId);

protected:

private:
	void setupModelData( QSqlQuery &sqlquery);
    void setupModelDataForList(QSqlQuery &sqlquery);
	ComicDB _getComic(const QModelIndex & mi);
	QList<ComicItem *> _data;

	QString _databasePath;

	QSqlDatabase dbTransaction;

    bool enableResorting;
    Mode mode;
    qulonglong sourceId;

signals:
	void beforeReset();
	void reset();
    void isEmpty();
    void searchNumResults(int);
    void resortedIndexes(QList<int>);
    void newSelectedIndex(const QModelIndex &);
};
//! [0]

#endif
