#include "reading_list_model.h"

#include "reading_list_item.h"

ReadingListModel::ReadingListModel(QObject *parent) :
    QAbstractItemModel(parent),rootItem(0)
{
}

int ReadingListModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid()) //TOP
        return specialLists.count() + labels.count() + rootItem->childCount();
    else
        return 0;
}

int ReadingListModel::columnCount(const QModelIndex &parent) const
{
    return 1;
    /*if (parent.isValid())
        return static_cast<ListItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();*/
}

QVariant ReadingListModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    ListItem * item = static_cast<ListItem*>(index.internalPointer());

    if (role == Qt::DecorationRole)
    {
        return QVariant(item->getIcon());
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->data(index.column());
}

Qt::ItemFlags ReadingListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
}

QVariant ReadingListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex ReadingListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ListItem *parentItem;

    /*if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ListItem*>(parent.internalPointer());*/


    if(!parent.isValid())
    {
        if(row >= 0 && row < specialLists.count())
            return createIndex(row, column, specialLists.at(row));

        if(row >= specialLists.count() && row < specialLists.count() + labels.count())
            return createIndex(row,column,labels.at(row-specialLists.count()));

        if(row >= specialLists.count() + labels.count())
            return createIndex(row,column,rootItem->child(row - (specialLists.count() + labels.count())));

    }
    /*FolderItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else*/
        return QModelIndex();

}

QModelIndex ReadingListModel::parent(const QModelIndex &index) const
{
    //if (!index.isValid())
        return QModelIndex();

    /*FolderItem *childItem = static_cast<FolderItem*>(index.internalPointer());
    FolderItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);    */
}

void ReadingListModel::setupModelData(QString path)
{
    beginResetModel();

    if(rootItem != 0)
    {
        delete rootItem;

        qDeleteAll(specialLists);
        qDeleteAll(labels);

        specialLists.clear();
        labels.clear();

        items.clear();
    }

    rootItem = 0;

    //setup special lists
    specialLists << new SpecialListItem(QList<QVariant>() /*<< 0*/ << "Favorites");
    specialLists << new SpecialListItem(QList<QVariant>() /*<< 1*/ << "Reading");

    //setup labels
    labels << new LabelItem(QList<QVariant>() /*<< 0*/ << "Oh Oh" << "red");
    labels << new LabelItem(QList<QVariant>() /*<< 1*/ << "lalala" << "orange");
    labels << new LabelItem(QList<QVariant>() /*<< 2*/ << "we are not sorry" << "yellow");
    labels << new LabelItem(QList<QVariant>() /*<< 3*/ << "there we go" << "green");
    labels << new LabelItem(QList<QVariant>() /*<< 4*/ << "oklabunga" << "cyan");
    labels << new LabelItem(QList<QVariant>() /*<< 5*/ << "hailer mailer" << "blue");
    labels << new LabelItem(QList<QVariant>() /*<< 6*/ << "lol" << "violet");
    labels << new LabelItem(QList<QVariant>() /*<< 7*/ << "problems" << "purple");
    labels << new LabelItem(QList<QVariant>() /*<< 8*/ << "me gussssta" << "pink");
    labels << new LabelItem(QList<QVariant>() /*<< 9*/ << ":D" << "white");
    labels << new LabelItem(QList<QVariant>() /*<< 10*/ << "ainsss" << "light");
    labels << new LabelItem(QList<QVariant>() /*<< 11*/ << "put a smile on my face" << "dark");

    //setup root item
    rootItem = new ReadingListItem(QList<QVariant>() /*<< 0*/ << "ROOT" << "atr");

    //setup reading lists
    rootItem->appendChild(new ReadingListItem(QList<QVariant>() /*<< 0*/ << "My reading list" << "atr"));
    rootItem->appendChild(new ReadingListItem(QList<QVariant>() /*<< 0*/ << "X timeline" << "atr"));

    endResetModel();
}

void ReadingListModel::deleteItem(const QModelIndex &mi)
{

}

void ReadingListModel::setupModelData(QSqlQuery &sqlquery, ReadingListItem *parent)
{

}


