#include "local_comic_list_model.h"

LocalComicListModel::LocalComicListModel(QObject *parent) :
	QAbstractItemModel(parent),numExtraRows(0)
{
}

void LocalComicListModel::load(QList<ComicDB> &comics)
{
	_data = comics;
}


QModelIndex LocalComicListModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index)
	return QModelIndex(); //no parent
}

int LocalComicListModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return _data.count() + numExtraRows;
}

int LocalComicListModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	if(_data.isEmpty())
		return 0;
	else
		return 1;//_data.at(0)->count();
}

QVariant LocalComicListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DecorationRole)
	{
			return QVariant();
	}
	if (role == Qt::TextAlignmentRole)
	{
		//TODO
	}

	if(role != Qt::DisplayRole)
		return QVariant();

	int row = index.row();

	if(row < _data.count())
		return _data[row].getFileName();
	else
		return QVariant();
}

Qt::ItemFlags LocalComicListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant LocalComicListModel::headerData(int section, Qt::Orientation orientation, int role) const
{

	if ( role == Qt::TextAlignmentRole)
		return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		return QVariant(QString(tr("file name")));
	}

	return QVariant();
}

QModelIndex LocalComicListModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	return createIndex(row, column);
}

void LocalComicListModel::addExtraRows(int numRows)
{
	numExtraRows = numRows;
}

