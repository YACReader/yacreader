#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlDatabase>

#include "comic_db.h"

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
	QVector<bool> getReadList();
	QVector<bool> setAllComicsRead(bool read);
	QList<ComicDB> getComics(QList<QModelIndex> list); //--> recupera la información común a los comics seleccionados
	QModelIndex getIndexFromId(quint64 id);
	//setcomicInfo(QModelIndex & mi); --> inserta en la base datos
	//setComicInfoForAllComics(); --> inserta la información común a todos los cómics de una sola vez.
	//setComicInfoForSelectedComis(QList<QModelIndex> list); -->inserta la información común para los comics seleccionados
private:
    void setupModelData( QSqlQuery &sqlquery);
	ComicDB _getComic(const QModelIndex & mi);
    QList<TableItem *> _data;

	QString _databasePath;

signals:
	void beforeReset();
	void reset();
};
//! [0]

#endif
