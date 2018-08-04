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
	return _data.count();
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

	//if(row < _data.count())
		return _data[row].getFileName();
	//else
		//return QVariant();
}

Qt::ItemFlags LocalComicListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant LocalComicListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);

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

QList<ComicDB> LocalComicListModel::getData()
{
    return _data;
}

void LocalComicListModel::removeComics(const QList<QModelIndex> &selectedIndexes)
{
    QModelIndex mi = selectedIndexes.first();
    QModelIndex lastMi = selectedIndexes.last();
    int sourceRow = mi.row();
    int sourceLastRow = lastMi.row();

    beginRemoveRows(QModelIndex(),selectedIndexes.first().row(),selectedIndexes.last().row());

    for(int i = sourceLastRow;i>=sourceRow;i--)
    {
        _removed.push_front(_data.at(i));
        _data.removeAt(i);
    }

    endRemoveRows();

    beginInsertRows(QModelIndex(),_data.count()-_removed.count(),_data.count()-1);
    for(int i = 0; i<_removed.count(); i++)
        _data.append(ComicDB());
    endInsertRows();
}

void LocalComicListModel::restoreAll()
{
    int numItemsToRemove = 0;
    for(int i = 0;numItemsToRemove<_removed.count();i++)
    {
        if(_data.at(i).getFileName().isEmpty())
        {
            beginRemoveRows(QModelIndex(),i,i);
            _data.removeAt(i);
            endRemoveRows();

            beginInsertRows(QModelIndex(),i,i);
            _data.insert(i,_removed.at(numItemsToRemove));
            endInsertRows();

            numItemsToRemove++;
        }
    }

    _removed.clear();
}

void LocalComicListModel::moveSelectionUp(const QList<QModelIndex> &selectedIndexes)
{
	QModelIndex mi = selectedIndexes.first();
	QModelIndex lastMi = selectedIndexes.last();
	int sourceRow = mi.row();
	int sourceLastRow = lastMi.row();
	int destRow = sourceRow - 1;

	if(destRow < 0)
		return;

	beginMoveRows(mi.parent(),sourceRow,sourceLastRow,mi.parent(),destRow);

	for(int i = sourceRow; i <= sourceLastRow; i++)
		_data.swap(i, i-1);

	endMoveRows();
}

void LocalComicListModel::moveSelectionDown(const QList<QModelIndex> &selectedIndexes)
{
	QModelIndex mi = selectedIndexes.first();
	QModelIndex lastMi = selectedIndexes.last();
	int sourceRow = mi.row();
	int sourceLastRow = lastMi.row();
	int destRow = sourceLastRow + 1;

	if(destRow >= _data.count())
		return;

	beginMoveRows(mi.parent(),sourceRow,sourceLastRow,mi.parent(),destRow+1);

	for(int i = sourceLastRow; i >= sourceRow; i--)
		_data.swap(i, i+1);

	endMoveRows();
}

void LocalComicListModel::addExtraRows(int numRows)
{
	numExtraRows = numRows;
	for(int i = 0; i<numExtraRows; i++)
		_data.append(ComicDB());
}

