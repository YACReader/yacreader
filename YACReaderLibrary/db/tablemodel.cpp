
#include <QtGui>
#include <QtDebug>

#include "tableitem.h"
#include "tablemodel.h"
#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "comic_db.h"

//ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read
#define NUMBER 0
#define TITLE 1
#define FILE_NAME 2
#define NUM_PAGES 3
#define ID 4
#define PARENT_ID 5
#define PATH 6
#define HASH 7
#define READ 8
#define IS_BIS 9

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
	if(index.column() == HASH)
		return QString::number(item->data(index.column()).toString().right(item->data(index.column()).toString().length()-40).toInt()/1024.0/1024.0,'f',2)+"Mb";
	if(index.column() == READ)
		return item->data(index.column()).toBool()?QVariant(tr("yes")):QVariant(tr("no"));
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
		}
	}

	if(orientation == Qt::Vertical && role == Qt::DecorationRole)
	{
		QString fileName = _data.value(section)->data(FILE_NAME).toString();
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
		QString hash = (*itr)->data(HASH).toString();
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
	selectQuery.prepare("select ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
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
		return _data.at(mi.row())->data(PATH).toString();
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
			QString nameLast = last->data(FILE_NAME).toString();
			QString nameCurrent = currentItem->data(FILE_NAME).toString();
			int numberLast,numberCurrent;
			numberLast = numberCurrent = NUMBER_MAX; //TODO change by std limit

			if(!last->data(NUMBER).isNull())
			numberLast = last->data(NUMBER).toInt();
			
			if(!currentItem->data(NUMBER).isNull())
			numberCurrent = currentItem->data(NUMBER).toInt();
			
			QList<TableItem *>::iterator i;
			i = _data.end();
			i--;

			if(numberCurrent != NUMBER_MAX)
			{
				while ((lessThan =numberCurrent < numberLast) && i != _data.begin())
				{
					i--;
					numberLast = NUMBER_MAX; //TODO change by std limit

					if(!(*i)->data(NUMBER).isNull())
						numberLast = (*i)->data(NUMBER).toInt();
				}
			}
			else
			{
				while ((lessThan = naturalSortLessThanCI(nameCurrent,nameLast)) && i != _data.begin() && numberLast == 99999999)
				{
					i--;
					nameLast = (*i)->data(FILE_NAME).toString();
					numberLast = NUMBER_MAX; //TODO change by std limit

					if(!(*i)->data(NUMBER).isNull())
						numberLast = (*i)->data(NUMBER).toInt();
				}

			}
			if(!lessThan) //si se ha encontrado un elemento menor que current, se inserta justo después
			{
				if(numberCurrent != NUMBER_MAX)
				{
					if(numberCurrent == numberLast)
						if(currentItem->data(IS_BIS).toBool())
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
	ComicDB c;

	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	c.load(_data.at(mi.row())->data(ID).toULongLong(),db);
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	return c;
}

ComicDB TableModel::_getComic(const QModelIndex & mi)
{
	ComicDB c;

	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	c.load(_data.at(mi.row())->data(ID).toULongLong(),db);
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	return c;
}


QVector<bool> TableModel::getReadList()
{
	int numComics = _data.count();
	QVector<bool> readList(numComics);
	for(int i=0;i<numComics;i++)
	{
		readList[i] = _data.value(i)->data(READ).toBool();
	}
	return readList;
}

QVector<bool> TableModel::setAllComicsRead(bool read)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();
	int numComics = _data.count();
	QVector<bool> readList(numComics);
	for(int i=0;i<numComics;i++)
	{
		readList[i] = read; 
		_data.value(i)->setData(READ,QVariant(read));
		ComicDB c;
		c.load(_data.value(i)->data(ID).toULongLong(),db);
		c.info.read = read;
		c.info.update(db);
	}
	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	emit dataChanged(index(0,READ),index(numComics-1,READ));

	return readList;
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

QVector<bool> TableModel::setComicsRead(QList<QModelIndex> list,bool read)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();
	foreach (QModelIndex mi, list)
	{
		_data.value(mi.row())->setData(READ, QVariant(read));
		ComicDB c;
		c.load(_data.value(mi.row())->data(ID).toULongLong(),db);
		c.info.read = read;
		c.info.update(db);
	}
	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	emit dataChanged(index(list.first().row(),READ),index(list.last().row(),READ));

	return getReadList();
}
qint64 TableModel::asignNumbers(QList<QModelIndex> list,int startingNumber)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();
	qint64 idFirst = _data.value(list[0].row())->data(ID).toULongLong();
	int i = 0;
	foreach (QModelIndex mi, list)
	{
		ComicDB c;
		c.load(_data.value(mi.row())->data(ID).toULongLong(),db);
		c.info.setNumber(startingNumber+i);
		c.info.edited = true;
		c.info.update(db);
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
		if((*itr)->data(ID).toULongLong() == id)
			break;
		i++;
	}

	return index(i,0);
}