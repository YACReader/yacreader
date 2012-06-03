
#include <QtGui>
#include <QtDebug>

#include "tableitem.h"
#include "tablemodel.h"
#include "data_base_management.h"
#include "qnaturalsorting.h"

TableModel::TableModel(QObject *parent)
    : QAbstractItemModel(parent)
{
	connect(this,SIGNAL(beforeReset()),this,SIGNAL(modelAboutToBeReset()));
	connect(this,SIGNAL(reset()),this,SIGNAL(modelReset()));
}

//! [0]
TableModel::TableModel( QSqlQuery &sqlquery, QObject *parent)
    : QAbstractItemModel(parent)
{
    setupModelData(sqlquery);
}
//! [0]

//! [1]
TableModel::~TableModel()
{
	qDeleteAll(_data);
}
//! [1]

//! [2]
int TableModel::columnCount(const QModelIndex &parent) const
{
	if(_data.isEmpty())
		return 0;
	return _data.first()->columnCount();
}
//! [2]

//! [3]
QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

	if (role == Qt::DecorationRole)
	{
			return QVariant();
	}

    if (role != Qt::DisplayRole)
        return QVariant();

    TableItem *item = static_cast<TableItem*>(index.internalPointer());

	return item->data(index.column());
}
//! [3]

//! [4]
Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//! [4]

//! [5]
QVariant TableModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)//TODO obtener esto de la query
		{
		case 0:
			return QVariant(QString("Id"));
		case 1:
			return QVariant(QString("ParentId"));
		case 2:
			return QVariant(QString("File Name"));
		case 3:
			return QVariant(QString("Path"));
		}
	}

	if(orientation == Qt::Vertical && role == Qt::DecorationRole)
		return QVariant(QIcon(":/images/icon.png"));

    return QVariant();
}
//! [5]

//! [6]
QModelIndex TableModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

	return createIndex(row, column, _data.at(row));
}
//! [6]

//! [7]
QModelIndex TableModel::parent(const QModelIndex &index) const
{
     return QModelIndex();
}
//! [7]

//! [8]
int TableModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
		return _data.count();

	return 0;
}
//! [8]

QStringList TableModel::getPaths(const QString & _source)
{
	QStringList paths;
	QString source = _source + "/.yacreaderlibrary/covers/";
	QList<TableItem *>::ConstIterator itr;
	for(itr = _data.constBegin();itr != _data.constEnd();itr++)
	{
		QString path = (*itr)->data(4).toString();
		paths << source+ path +".jpg";
	}

	return paths;
}

void TableModel::setupModelData(unsigned long long int folderId,QSqlDatabase & db)
{
	//QFile f(QCoreApplication::applicationDirPath()+"/performance.txt");
	//f.open(QIODevice::Append);
	emit(beforeReset());
	//QElapsedTimer timer;
	//timer.start();
	qDeleteAll(_data);
	_data.clear();

	//QTextStream txtS(&f);
	//txtS << "TABLEMODEL: Tiempo de borrado: " << timer.elapsed() << "ms\r\n";
	db.open();
	//crear la consulta
	//timer.restart();
	QSqlQuery selectQuery(db); //TODO check
	selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash,ci.read from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
	selectQuery.bindValue(":parentId", folderId);
	selectQuery.exec();
	//txtS << "TABLEMODEL: Tiempo de consulta: " << timer.elapsed() << "ms\r\n";
	//timer.restart();
	setupModelData(selectQuery);
	//txtS << "TABLEMODEL: Tiempo de creación del modelo: " << timer.elapsed() << "ms\r\n";
	db.close();
	_database = db;
	emit(reset());
	//f.close();
}

QString TableModel::getComicPath(QModelIndex & mi)
{
	if(mi.isValid())
		return _data.at(mi.row())->data(3).toString();
	return "";
}
void TableModel::setupModelData(QSqlQuery &sqlquery)
{
	TableItem * currentItem;
	while (sqlquery.next()) 
	{
		QList<QVariant> data;
		QSqlRecord record = sqlquery.record();
		for(int i=0;i<record.count();i++)
			data << record.value(i);
		//TODO sort by sort indicator and name
		currentItem = new TableItem(data);
		bool lessThan = false;
		if(_data.isEmpty())
			_data.append(currentItem);
		else
		{
			TableItem * last = _data.back();
			QString nameLast = last->data(2).toString(); //TODO usar info name si está disponible, sino el nombre del fichero.....
			QString nameCurrent = currentItem->data(2).toString();
			QList<TableItem *>::iterator i;
			i = _data.end();
			i--;
			while ((lessThan = naturalSortLessThanCI(nameCurrent,nameLast)) && i != _data.begin())
			{
				i--;
				nameLast = (*i)->data(2).toString();
			}
			if(!lessThan) //si se ha encontrado un elemento menor que current, se inserta justo después
				_data.insert(++i,currentItem);
			else
				_data.insert(i,currentItem);

		}
	}
}

Comic TableModel::getComic(QModelIndex & mi)
{
	Comic c;
	_database.open();
	c.load(_data.at(mi.row())->data(0).toLongLong(),_database);
	_database.close();
	return c;
}

QVector<bool> TableModel::getReadList()
{
	int numComics = _data.count();
	QVector<bool> readList(numComics);
	for(int i=0;i<numComics;i++)
	{
		//TODO reemplazar el acceso a las columnas con enteros por defines
		readList[i] = _data.value(i)->data(5).toBool();
	}
	return readList;
}

QVector<bool> TableModel::setAllComicsRead(bool read)
{
	_database.open();
	_database.transaction();
	int numComics = _data.count();
	QVector<bool> readList(numComics);
	for(int i=0;i<numComics;i++)
	{
		//TODO reemplazar el acceso a las columnas con enteros por defines
		readList[i] = read; 
		_data.value(i)->data(5) = QVariant(true);
		Comic c;
		c.load(_data.value(i)->data(0).toLongLong(),_database);
		c.info.read = read;
		c.info.update(_database);
	}
	_database.commit();
	_database.close();

	return readList;
}