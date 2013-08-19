
#include <QtGui>
#include <QtDebug>

#include "tableitem.h"
#include "tablemodel.h"
#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "comic_db.h"
#include "db_helper.h"

//ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read
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

	if (index.column() == Columns::Rating && role == Qt::DecorationRole)
	{
		TableItem *item = static_cast<TableItem*>(index.internalPointer());
		return QPixmap(QString(":/images/rating%1.png").arg(item->data(index.column()).toInt()));
	}

	if (role == Qt::DecorationRole)
	{
			return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		switch(index.column())//TODO obtener esto de la query
		{
		case 0:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		case 3:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		case 7:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		case Columns::CurrentPage:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		default:
			return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
		}
	}


	if (role != Qt::DisplayRole)
		return QVariant();

	TableItem *item = static_cast<TableItem*>(index.internalPointer());
	if(index.column() == Columns::Hash)
		return QString::number(item->data(index.column()).toString().right(item->data(index.column()).toString().length()-40).toInt()/1024.0/1024.0,'f',2)+"Mb";
	if(index.column() == Columns::ReadColumn)
		return (item->data(Columns::CurrentPage).toInt()==item->data(Columns::NumPages).toInt() || item->data(Columns::ReadColumn).toBool())?QVariant(tr("yes")):QVariant(tr("no"));
	if(index.column() == Columns::CurrentPage)
		return item->data(Columns::HasBeenOpened).toBool()?item->data(index.column()):QVariant("-");
	
	if (index.column() == Columns::Rating)
		return QVariant();

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
			return QVariant(QString("#"));
		case 1:
			return QVariant(QString(tr("Title")));
		case 2:
			return QVariant(QString(tr("File Name")));
		case 3:
			return QVariant(QString(tr("Pages")));
		case 7:
			return QVariant(QString(tr("Size")));
		case 8:
			return QVariant(QString(tr("Read")));
		case Columns::CurrentPage:
			return QVariant(QString(tr("Current Page")));
		case Columns::Rating:
			return QVariant(QString(tr("Rating")));
		}
	}

	if (orientation == Qt::Horizontal && role == Qt::TextAlignmentRole)
	{
		switch(section)//TODO obtener esto de la query
		{
		case 0:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		case 3:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		case 7:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		case Columns::CurrentPage:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		default:
			return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
		}
	}


	if(orientation == Qt::Vertical && role == Qt::DecorationRole)
	{
		QString fileName = _data.value(section)->data(Columns::FileName).toString();
		QFileInfo fi(fileName);
		QString ext = fi.suffix();

		if (ext.compare("cbr",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/comicRar.png"));
		else if (ext.compare("cbz",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/comicZip.png"));
		else if(ext.compare("pdf",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/pdf.png"));
		else if (ext.compare("tar",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/tar.png"));
		else if(ext.compare("zip",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/zip.png"));
		else if(ext.compare("rar",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/rar.png"));
		else if (ext.compare("7z",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/7z.png"));
		else if (ext.compare("cb7",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/comic7z.png"));
		else if (ext.compare("cb7",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/comicTar.png"));

	}

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
		QString hash = (*itr)->data(Columns::Hash).toString();
		paths << source+ hash +".jpg";
	}

	return paths;
}

void TableModel::setupModelData(unsigned long long int folderId,const QString & databasePath)
{
	//QFile f(QCoreApplication::applicationDirPath()+"/performance.txt");
	//f.open(QIODevice::Append);
	beginResetModel();
	//QElapsedTimer timer;
	//timer.start();
	qDeleteAll(_data);
	_data.clear();

	//QTextStream txtS(&f);
	//txtS << "TABLEMODEL: Tiempo de borrado: " << timer.elapsed() << "ms\r\n";
	_databasePath = databasePath;
	QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
	{
	//crear la consulta
	//timer.restart();
	QSqlQuery selectQuery(db); //TODO check
	selectQuery.prepare("select ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
	selectQuery.bindValue(":parentId", folderId);
	selectQuery.exec();
	//txtS << "TABLEMODEL: Tiempo de consulta: " << timer.elapsed() << "ms\r\n";
	//timer.restart();
	setupModelData(selectQuery);
	//txtS << "TABLEMODEL: Tiempo de creación del modelo: " << timer.elapsed() << "ms\r\n";
	//selectQuery.finish();
	}
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);
	endResetModel();
	//f.close();
}

QString TableModel::getComicPath(QModelIndex mi)
{
	if(mi.isValid())
		return _data.at(mi.row())->data(Columns::Path).toString();
	return "";
}
#define NUMBER_MAX 99999999
void TableModel::setupModelData(QSqlQuery &sqlquery)
{
	TableItem * currentItem;
	while (sqlquery.next()) 
	{
		QList<QVariant> data;
		QSqlRecord record = sqlquery.record();
		for(int i=0;i<record.count();i++)
			data << record.value(i);

		currentItem = new TableItem(data);
		bool lessThan = false;
		if(_data.isEmpty())
			_data.append(currentItem);
		else
		{
			TableItem * last = _data.back();
			QString nameLast = last->data(Columns::FileName).toString();
			QString nameCurrent = currentItem->data(Columns::FileName).toString();
			int numberLast,numberCurrent;
			numberLast = numberCurrent = NUMBER_MAX; //TODO change by std limit

			if(!last->data(Columns::Number).isNull())
			numberLast = last->data(Columns::Number).toInt();
			
			if(!currentItem->data(Columns::Number).isNull())
			numberCurrent = currentItem->data(Columns::Number).toInt();
			
			QList<TableItem *>::iterator i;
			i = _data.end();
			i--;

			if(numberCurrent != NUMBER_MAX)
			{
				while ((lessThan =numberCurrent < numberLast) && i != _data.begin())
				{
					i--;
					numberLast = NUMBER_MAX; //TODO change by std limit

					if(!(*i)->data(Columns::Number).isNull())
						numberLast = (*i)->data(Columns::Number).toInt();
				}
			}
			else
			{
				while ((lessThan = naturalSortLessThanCI(nameCurrent,nameLast)) && i != _data.begin() && numberLast == 99999999)
				{
					i--;
					nameLast = (*i)->data(Columns::FileName).toString();
					numberLast = NUMBER_MAX; //TODO change by std limit

					if(!(*i)->data(Columns::Number).isNull())
						numberLast = (*i)->data(Columns::Number).toInt();
				}

			}
			if(!lessThan) //si se ha encontrado un elemento menor que current, se inserta justo después
			{
				if(numberCurrent != NUMBER_MAX)
				{
					if(numberCurrent == numberLast)
						if(currentItem->data(Columns::IsBis).toBool())
						{
							_data.insert(++i,currentItem);
						}
						else
							_data.insert(i,currentItem);
					else
						_data.insert(++i,currentItem);
				}
				else
					_data.insert(++i,currentItem);
			}
			else
			{
				_data.insert(i,currentItem);
			}

		}
	}
}

ComicDB TableModel::getComic(const QModelIndex & mi)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	ComicDB c = DBHelper::loadComic(_data.at(mi.row())->data(Columns::Id).toULongLong(),db);
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	return c;
}

ComicDB TableModel::_getComic(const QModelIndex & mi)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	ComicDB c = DBHelper::loadComic(_data.at(mi.row())->data(Columns::Id).toULongLong(),db);
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	return c;
}


QVector<YACReaderComicReadStatus> TableModel::getReadList()
{
	int numComics = _data.count();
	QVector<YACReaderComicReadStatus> readList(numComics);
	for(int i=0;i<numComics;i++)
	{
		if(_data.value(i)->data(Columns::ReadColumn).toBool())
			readList[i] = YACReaderComicReadStatus::Read;
		else if (_data.value(i)->data(Columns::CurrentPage).toInt() == _data.value(i)->data(Columns::NumPages).toInt())
			 readList[i] = YACReaderComicReadStatus::Read;
		else if (_data.value(i)->data(Columns::HasBeenOpened).toBool())
			readList[i] = YACReaderComicReadStatus::Opened;
		else
			readList[i] = YACReaderComicReadStatus::Unread;
	}
	return readList;
}
//TODO check other read status
QVector<YACReaderComicReadStatus> TableModel::setAllComicsRead(YACReaderComicReadStatus read)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();
	int numComics = _data.count();
	QVector<YACReaderComicReadStatus> readList(numComics);
	for(int i=0;i<numComics;i++)
	{
		readList[i] = read;
		if(read == YACReaderComicReadStatus::Read)
		{
			_data.value(i)->setData(Columns::ReadColumn,QVariant(true));
			ComicDB c = DBHelper::loadComic(_data.value(i)->data(Columns::Id).toULongLong(),db);
			c.info.read = true;
			DBHelper::update(&(c.info),db);
		}
		if(read == YACReaderComicReadStatus::Unread)
		{
			_data.value(i)->setData(Columns::ReadColumn,QVariant(false));
			ComicDB c = DBHelper::loadComic(_data.value(i)->data(Columns::Id).toULongLong(),db);
			c.info.read = false;
			DBHelper::update(&(c.info),db);
		}
	}
	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	emit dataChanged(index(0,Columns::ReadColumn),index(numComics-1,Columns::ReadColumn));

	return readList;
}

QList<ComicDB> TableModel::getAllComics()
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();

	QList<ComicDB> comics;
	int numComics = _data.count();
	for(int i=0;i<numComics;i++)
	{
		comics.append(DBHelper::loadComic(_data.value(i)->data(Columns::Id).toULongLong(),db));
	}

	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	return comics;
}

QList<ComicDB> TableModel::getComics(QList<QModelIndex> list)
{
	QList<ComicDB> comics;

	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();
	QList<QModelIndex>::const_iterator itr;
	for(itr = list.constBegin(); itr!= list.constEnd();itr++)
	{
		comics.append(_getComic(*itr));
	}
	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);
	return comics;
}
//TODO
QVector<YACReaderComicReadStatus> TableModel::setComicsRead(QList<QModelIndex> list,YACReaderComicReadStatus read)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();
	foreach (QModelIndex mi, list)
	{
		if(read == YACReaderComicReadStatus::Read)
		{
		_data.value(mi.row())->setData(Columns::ReadColumn, QVariant(true));
		ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(Columns::Id).toULongLong(),db);
		c.info.read = true;
		DBHelper::update(&(c.info),db);
		}
		if(read == YACReaderComicReadStatus::Unread)
		{
		_data.value(mi.row())->setData(Columns::ReadColumn, QVariant(false));
		ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(Columns::Id).toULongLong(),db);
		c.info.read = false;
		DBHelper::update(&(c.info),db);
		}
	}
	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	emit dataChanged(index(list.first().row(),Columns::ReadColumn),index(list.last().row(),Columns::ReadColumn));

	return getReadList();
}
qint64 TableModel::asignNumbers(QList<QModelIndex> list,int startingNumber)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();
	qint64 idFirst = _data.value(list[0].row())->data(Columns::Id).toULongLong();
	int i = 0;
	foreach (QModelIndex mi, list)
	{
		ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(Columns::Id).toULongLong(),db);
		c.info.setNumber(startingNumber+i);
		c.info.edited = true;
		DBHelper::update(&(c.info),db);
		i++;
	}

	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	//emit dataChanged(index(list.first().row(),READ),index(list.last().row(),READ));

	return idFirst;
}
QModelIndex TableModel::getIndexFromId(quint64 id)
{
	QList<TableItem *>::ConstIterator itr;
	int i=0;
	for(itr = _data.constBegin();itr != _data.constEnd();itr++)
	{
		if((*itr)->data(Columns::Id).toULongLong() == id)
			break;
		i++;
	}

	return index(i,0);
}

void TableModel::startTransaction()
{
	
	dbTransaction = DataBaseManagement::loadDatabase(_databasePath);
	dbTransaction.transaction();
}

void TableModel::finishTransaction()
{
	dbTransaction.commit();
	dbTransaction.close();
	QSqlDatabase::removeDatabase(_databasePath);


}

void TableModel::removeInTransaction(int row)
{
	ComicDB c = DBHelper::loadComic(_data.at(row)->data(Columns::Id).toULongLong(),dbTransaction);

	DBHelper::removeFromDB(&c,dbTransaction);
	beginRemoveRows(QModelIndex(),row,row);
	removeRow(row);
	delete _data.at(row);
	_data.removeAt(row);

	endRemoveRows();
}

void TableModel::remove(ComicDB * comic, int row)
{
	beginRemoveRows(QModelIndex(),row,row);
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

	DBHelper::removeFromDB(comic,db);
	
	removeRow(row);
	delete _data.at(row);
	_data.removeAt(row);

	db.close();
	QSqlDatabase::removeDatabase(_databasePath);
	endRemoveRows();
}

ComicDB TableModel::getComic(int row)
{
	return getComic(index(row,0));
}

void TableModel::remove(int row)
{
	removeInTransaction(row);
}

void TableModel::reload(const ComicDB & comic)
{
	int row = 0;
	bool found = false;
	foreach(TableItem * item,_data)
	{
		if(item->data(Columns::Id).toULongLong() == comic.id)
		{
			found = true;
			item->setData(Columns::CurrentPage,comic.info.currentPage);
			item->setData(Columns::HasBeenOpened,true);
			break;
				
		}
		row++;
	}
	if(found)
		emit dataChanged(index(row,Columns::CurrentPage),index(row,Columns::CurrentPage));
}