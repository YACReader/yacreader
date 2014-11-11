#include "reading_list_model.h"

#include "reading_list_item.h"

#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "db_helper.h"

#include "QsLog.h"

ReadingListModel::ReadingListModel(QObject *parent) :
    QAbstractItemModel(parent),rootItem(0)
{
    separator1 = new ReadingListSeparatorItem;
    separator2 = new ReadingListSeparatorItem;
}

int ReadingListModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid()) //TOP
    {
        int separatorsCount = labels.isEmpty()?1:2;
        return specialLists.count() + labels.count() + rootItem->childCount() + separatorsCount;
    }
    else
    {
        ListItem * item = static_cast<ListItem*>(parent.internalPointer());

        QLOG_DEBUG() << item->itemData;

        if(typeid(*item) == typeid(ReadingListItem))
        {
            ReadingListItem * item = static_cast<ReadingListItem*>(parent.internalPointer());
            return item->childCount();
        }
    }

    return 0;
}

int ReadingListModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        ListItem * item = static_cast<ListItem*>(parent.internalPointer());
        if(typeid(*item) == typeid(ReadingListSeparatorItem))
            return 0;
    }
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

    if(typeid(*item) == typeid(ReadingListSeparatorItem))
        return QVariant();

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

    ListItem * item = static_cast<ListItem*>(index.internalPointer());
    if(typeid(*item) == typeid(ReadingListSeparatorItem))
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

    if(!parent.isValid())
    {
        int separatorsCount = labels.isEmpty()?1:2;

        if(row >= 0 && row < specialLists.count())
            return createIndex(row, column, specialLists.at(row));

        if(row == specialLists.count())
            return createIndex(row,column,separator1);

        if(row > specialLists.count()  && row <= specialLists.count() + labels.count())
            return createIndex(row,column,labels.at(row-specialLists.count()-1));

        if(separatorsCount == 2)
        if(row == specialLists.count() + labels.count() + 1)
            return createIndex(row,column,separator2);

        if(row >= specialLists.count() + labels.count() + separatorsCount)
            return createIndex(row,column,rootItem->child(row - (specialLists.count() + labels.count() + separatorsCount)));

    } else //sublist
    {
        ReadingListItem *parentItem;

        if (!parent.isValid())
            parentItem = rootItem; //this should be impossible
        else
            parentItem = static_cast<ReadingListItem*>(parent.internalPointer());

        ReadingListItem *childItem = parentItem->child(row);
        return createIndex(row,column,childItem);
    }
    /*FolderItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else*/
        return QModelIndex();

}

QModelIndex ReadingListModel::parent(const QModelIndex &index) const
{

    if(!index.isValid())
        return QModelIndex();

    ListItem * item = static_cast<ListItem*>(index.internalPointer());

    if(typeid(*item) == typeid(ReadingListItem))
    {
        ReadingListItem * childItem = static_cast<ReadingListItem*>(index.internalPointer());
        ReadingListItem * parent = childItem->parent;
        if(parent != 0)
            return createIndex(parent->row(), 0, parent);
    }

    return QModelIndex();
}

void ReadingListModel::setupModelData(QString path)
{
    beginResetModel();

    cleanAll();

    _databasePath = path;
    QSqlDatabase db = DataBaseManagement::loadDatabase(path);

    //setup special lists
    specialLists = setupSpecialLists(db);

    //separator--------------------------------------------

    //setup labels
    labels = setupLabels(db);

    //separator--------------------------------------------

    //setup reading list
    setupReadingLists(db);

    endResetModel();
}

void ReadingListModel::addNewLabel(const QString &name, YACReader::LabelColors color)
{
    QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
    qulonglong id = DBHelper::insertLabel(name, color, db);

    int newPos = addLabelIntoList(new LabelItem(QList<QVariant>() << name << YACReader::colorToName(color) << id << color));

    beginInsertRows(QModelIndex(),specialLists.count()+1+newPos+1, specialLists.count()+1+newPos+1);
    endInsertRows();

    QSqlDatabase::removeDatabase(_databasePath);
}

bool ReadingListModel::isEditable(const QModelIndex &mi)
{
    return (mi.row() > specialLists.count());
}

QString ReadingListModel::name(const QModelIndex &mi)
{
    return data(mi,Qt::DisplayRole).toString();
}

void ReadingListModel::rename(const QModelIndex &mi, const QString &name)
{
    if(!isEditable(mi))
        return;

    QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

    //TODO


    emit dataChanged(index(mi.row(), 0), index(mi.row(), 0));

    QSqlDatabase::removeDatabase(_databasePath);
}

void ReadingListModel::deleteItem(const QModelIndex &mi)
{

}

void ReadingListModel::cleanAll()
{
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
}

void ReadingListModel::setupModelData(QSqlQuery &sqlquery, ReadingListItem *parent)
{

}

QList<SpecialListItem *> ReadingListModel::setupSpecialLists(QSqlDatabase & db)
{
    QList<SpecialListItem *> list;

    QSqlQuery selectQuery("SELECT * FROM default_reading_list ORDER BY id",db);
    while(selectQuery.next()) {
        QSqlRecord record = selectQuery.record();
        list << new SpecialListItem(QList<QVariant>()  << record.value("name") << record.value("id"));
    }

    //Reading after Favorites, Why? Because I want :P
    list.insert(1,new SpecialListItem(QList<QVariant>()  << "Reading" << 0));

    return list;
}

QList<LabelItem *> ReadingListModel::setupLabels(QSqlDatabase & db)
{
    QList<LabelItem *> list;

    QSqlQuery selectQuery("SELECT * FROM label ORDER BY ordering,name",db); //TODO add some kind of
    while(selectQuery.next()) {
        QSqlRecord record = selectQuery.record();
        list << new LabelItem(QList<QVariant>() << record.value("name") << record.value("color") << record.value("id") << record.value("ordering"));
    }

    //TEST

//    INSERT INTO label (name, color, ordering) VALUES ("Oh Oh", "red", 1);
//    INSERT INTO label (name, color, ordering) VALUES ("lalala", "orange", 2);
//    INSERT INTO label (name, color, ordering) VALUES ("we are not sorry", "yellow", 3);
//    INSERT INTO label (name, color, ordering) VALUES ("there we go", "green", 4);
//    INSERT INTO label (name, color, ordering) VALUES ("oklabunga", "cyan", 5);
//    INSERT INTO label (name, color, ordering) VALUES ("hailer mailer", "blue", 6);
//    INSERT INTO label (name, color, ordering) VALUES ("lol", "violet", 7);
//    INSERT INTO label (name, color, ordering) VALUES ("problems", "purple", 8);
//    INSERT INTO label (name, color, ordering) VALUES ("me gussssta", "pink", 9);
//    INSERT INTO label (name, color, ordering) VALUES (":D", "white", 10);
//    INSERT INTO label (name, color, ordering) VALUES ("ainsss", "light", 11);
//    INSERT INTO label (name, color, ordering) VALUES ("put a smile on my face", "dark", 12);

    return list;
}

void ReadingListModel::setupReadingLists(QSqlDatabase & db)
{
    //setup root item
    rootItem = new ReadingListItem(QList<QVariant>() /*<< 0*/ << "ROOT" << "atr");

    //setup reading lists
    ReadingListItem * node1;
    rootItem->appendChild(node1 = new ReadingListItem(QList<QVariant>() /*<< 0*/ << "My reading list" << "atr"));
    rootItem->appendChild(new ReadingListItem(QList<QVariant>() /*<< 0*/ << "X timeline" << "atr"));

    node1->appendChild(new ReadingListItem(QList<QVariant>() /*<< 0*/ << "sublist" << "atr",node1));
}

int ReadingListModel::addLabelIntoList(LabelItem *item)
{
    if(labels.isEmpty())
        labels << item;
    else
    {
        int i = 0;

        while (i < labels.count() && (labels.at(i)->colorid() < item->colorid()) )
            i++;

        if(i < labels.count())
        {
            if(labels.at(i)->colorid() == item->colorid()) //sort by name
            {
                while( i < labels.count() && naturalSortLessThanCI(labels.at(i)->name(),item->name()))
                    i++;
            }
        }


        if(i >= labels.count())
            labels << item;
        else
        {
            labels.insert(i,item);
        }

        return i;
    }

    return 0;
}


