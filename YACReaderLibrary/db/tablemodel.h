#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlDatabase>

#include "yacreader_global.h"

class ComicDB;

class TableItem;

//! [0]
class TableModel : public QAbstractItemModel
{
    Q_OBJECT

public:
	TableModel(QObject *parent = 0);
    TableModel( QSqlQuery &sqlquery, QObject *parent = 0);
    ~TableModel();


    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
	void setupModelData(unsigned long long int parentFolder,const QString & databasePath);

	//Métodos de conveniencia
	QStringList getPaths(const QString & _source);
	QString getComicPath(QModelIndex mi);
	ComicDB getComic(const QModelIndex & mi); //--> para la edición
	ComicDB getComic(int row);
	QVector<YACReaderComicReadStatus> getReadList();
	QVector<YACReaderComicReadStatus> setAllComicsRead(YACReaderComicReadStatus readStatus);
	QList<ComicDB> getComics(QList<QModelIndex> list); //--> recupera la información común a los comics seleccionados
	QList<ComicDB> getAllComics();
	QModelIndex getIndexFromId(quint64 id);
	//setcomicInfo(QModelIndex & mi); --> inserta en la base datos
	//setComicInfoForAllComics(); --> inserta la información común a todos los cómics de una sola vez.
	//setComicInfoForSelectedComis(QList<QModelIndex> list); -->inserta la información común para los comics seleccionados
	QVector<YACReaderComicReadStatus> setComicsRead(QList<QModelIndex> list,YACReaderComicReadStatus read);
	qint64 asignNumbers(QList<QModelIndex> list,int startingNumber);
	void remove(ComicDB * comic, int row);
	void removeInTransaction(int row);
	void reload(const ComicDB & comic);

public slots:
	void remove(int row);
	void startTransaction();
	void finishTransaction();

private:
    void setupModelData( QSqlQuery &sqlquery);
	ComicDB _getComic(const QModelIndex & mi);
    QList<TableItem *> _data;

	QString _databasePath;

	QSqlDatabase dbTransaction;

signals:
	void beforeReset();
	void reset();
};
//! [0]

#endif
