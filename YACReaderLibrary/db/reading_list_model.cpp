#include "reading_list_model.h"

#include "reading_list_item.h"

#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "db_helper.h"

#include "QsLog.h"

#include <typeinfo>

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
        int separatorsCount = 2;//labels.isEmpty()?1:2;
        return specialLists.count() + labels.count() + rootItem->childCount() + separatorsCount;
    }
    else
    {
        ListItem * item = static_cast<ListItem*>(parent.internalPointer());

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

    if (role == ReadingListModel::TypeListsRole)
    {
        if(typeid(*item) == typeid(SpecialListItem))
            return QVariant(ReadingListModel::SpecialList);

        if(typeid(*item) == typeid(LabelItem))
            return QVariant(ReadingListModel::Label);

        if(typeid(*item) == typeid(ReadingListItem))
            return QVariant(ReadingListModel::ReadingList);

        if(typeid(*item) == typeid(ReadingListSeparatorItem))
            return QVariant(ReadingListModel::Separator);
    }

    if (role == ReadingListModel::LabelColorRole && typeid(*item) == typeid(LabelItem) )
    {
        LabelItem * labelItem = static_cast<LabelItem*>(item);
        return QVariant(labelItem->colorid());
    }

    if (role == ReadingListModel::IDRole)
    {
        QLOG_DEBUG() << "getting role";
        return item->getId();
}

    if (role == ReadingListModel::SpecialListTypeRole && typeid(*item) == typeid(SpecialListItem))
    {
        SpecialListItem * specialListItem = static_cast<SpecialListItem*>(item);
        return QVariant(specialListItem->getType());
    }

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

    if(typeid(*item) == typeid(ReadingListItem) && static_cast<ReadingListItem *>(item)->parent->getId()!=0)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled; //only sublists are dragable

    return  Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
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
        int separatorsCount = 2;//labels.isEmpty()?1:2;

        if(rowIsSpecialList(row,parent))
            return createIndex(row, column, specialLists.at(row));

        if(row == specialLists.count())
            return createIndex(row,column,separator1);

        if(rowIsLabel(row,parent))
            return createIndex(row,column,labels.at(row-specialLists.count()-1));

        if(separatorsCount == 2)
        if(row == specialLists.count() + labels.count() + 1)
            return createIndex(row,column,separator2);

        if(rowIsReadingList(row,parent))
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
        if(parent->getId() != 0)
            return createIndex(parent->row()+specialLists.count()+labels.count()+2, 0, parent);
    }

    return QModelIndex();
}

bool ReadingListModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    QLOG_DEBUG() << "trying to drop into row = " << row << "column column = " << column << "parent" << parent;

    if(row == -1)
        return false;

    if(!parent.isValid()) //top level items
    {
        if(row == -1) //no list
            return false;

        if(row == 1) //reading is just an smart list
            return false;

        if(rowIsSeparator(row,parent))
            return false;
    }

    if(data->formats().contains(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat))
        return true;

    if(rowIsReadingList(row,parent))// TODO avoid droping in a different parent
        if(!parent.isValid())
            return false;
        else
        {
            QList<QPair<int,int> > sublistsRows;
            QByteArray rawData = data->data(YACReader::YACReaderLibrarSubReadingListMimeDataFormat);
            QDataStream in(&rawData,QIODevice::ReadOnly);
            in  >> sublistsRows; //deserialize the list of indentifiers
            if(parent.row()!= sublistsRows.at(0).second)
                return false;
            return data->formats().contains(YACReader::YACReaderLibrarSubReadingListMimeDataFormat);

        }

    return false;
}

bool ReadingListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    QLOG_DEBUG() << "drop mimedata into row = " << row << " column = " << column << "parent" << parent;
    if(data->formats().contains(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat))
        return dropComics(data, action, row, column, parent);

    if(data->formats().contains(YACReader::YACReaderLibrarSubReadingListMimeDataFormat))
        return dropSublist(data, action, row, column, parent);
}

bool ReadingListModel::dropComics(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    QList<qulonglong> comicIds = YACReader::mimeDataToComicsIds(data);

    QLOG_DEBUG() << "dropped : " << comicIds;

    QModelIndex dest;
    QModelIndex parentDest;

    if(row == -1)
    {
        dest = parent;
    }
    else
        dest = index(row,column,parent);

    parentDest = dest.parent();

    if(rowIsSpecialList(dest.row(),parentDest)) {
        if(dest.row() == 0) //add to favorites
        {
            QLOG_DEBUG() << "-------addComicsToFavorites : " << comicIds << " to " << dest.data(IDRole).toULongLong();
            emit addComicsToFavorites(comicIds);
            return true;
        }
    }

    if(rowIsLabel(dest.row(),parentDest)) {
        QLOG_DEBUG() << "+++++++++++addComicsToLabel : " << comicIds << " to " << dest.data(IDRole).toULongLong();
        emit addComicsToLabel(comicIds,  dest.data(IDRole).toULongLong());
        return true;
    }

    if(rowIsReadingList(dest.row(),parentDest)) {
        QLOG_DEBUG() << "///////////addComicsToReadingList : " << comicIds << " to " << dest.data(IDRole).toULongLong();
        emit addComicsToReadingList(comicIds,  dest.data(IDRole).toULongLong());
        return true;
    }

    return false;
}

bool ReadingListModel::dropSublist(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    QList<QPair<int,int> > sublistsRows;
    QByteArray rawData = data->data(YACReader::YACReaderLibrarSubReadingListMimeDataFormat);
    QDataStream in(&rawData,QIODevice::ReadOnly);
    in  >> sublistsRows; //deserialize the list of indentifiers

    QLOG_DEBUG() << "dropped : " << sublistsRows;

    int sourceRow = sublistsRows.at(0).first;
    int destRow = row;
    QModelIndex destParent = parent;
    if(row == -1)
    {
        QLOG_DEBUG() << "droping inside parent";
        destRow = parent.row();
        destParent = parent.parent();
    }
    QLOG_DEBUG() << "move " << sourceRow << "-" << destRow;

    if(sourceRow == destRow)
        return false;

    //beginMoveRows(destParent,sourceRow,sourceRow,destParent,destRow);

    ReadingListItem * parentItem = static_cast<ReadingListItem *>(destParent.internalPointer());
    ReadingListItem * child = parentItem->child(sourceRow);
    parentItem->removeChild(child);
    parentItem->appendChild(child,destRow);

    reorderingChildren(parentItem->children());
    //endMoveRows();

    return true;
}

QMimeData *ReadingListModel::mimeData(const QModelIndexList &indexes) const
{
    QLOG_DEBUG() << "mimeData requested" << indexes;

    if(indexes.length() == 0)
    {
        QLOG_ERROR() << "mimeData requested: indexes is empty";
        return new QMimeData();//TODO what happens if 0 is returned?
    }

    if(indexes.length() > 1)
        QLOG_DEBUG() << "mimeData requested for more than one index, this shouldn't be possible";

    QModelIndex modelIndex = indexes.at(0);

    QList<QPair<int,int> > rows;
    rows << QPair<int,int>(modelIndex.row(),modelIndex.parent().row());
    QLOG_DEBUG() << "mimeData requested for row : " << modelIndex.row();

    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out << rows; //serialize the list of identifiers

    QMimeData * mimeData = new QMimeData();
    mimeData->setData(YACReader::YACReaderLibrarSubReadingListMimeDataFormat, data);

    return mimeData;
}

void ReadingListModel::setupReadingListsData(QString path)
{
    beginResetModel();

    cleanAll();

    _databasePath = path;
    QSqlDatabase db = DataBaseManagement::loadDatabase(path);

    //setup special lists
    specialLists = setupSpecialLists(db);

    //separator--------------------------------------------

    //setup labels
    setupLabels(db);

    //separator--------------------------------------------

    //setup reading list
    setupReadingLists(db);

    endResetModel();
}

void ReadingListModel::addNewLabel(const QString &name, YACReader::LabelColors color)
{
    QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
    qulonglong id = DBHelper::insertLabel(name, color, db);

    beginInsertRows(QModelIndex(),0, 0);

    int newPos = addLabelIntoList(new LabelItem(QList<QVariant>() << name << YACReader::colorToName(color) << id << color));

    //beginInsertRows(QModelIndex(),specialLists.count()+1+newPos+1, specialLists.count()+1+newPos+1);
    endInsertRows();

    QSqlDatabase::removeDatabase(_databasePath);
}

void ReadingListModel::addReadingList(const QString &name)
{
    QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

    beginInsertRows(QModelIndex(), 0, 0); //TODO calculate the right coordinates before inserting

    qulonglong id = DBHelper::insertReadingList(name,db);
    ReadingListItem * newItem;
    rootItem->appendChild(newItem = new ReadingListItem(QList<QVariant>()
                                              << name
                                              << id
                                              << false
                                              << true
                                              << 0));

    items.insert(id, newItem);

    /*int pos = rootItem->children().indexOf(newItem);

    pos += specialLists.count()+1+labels.count()+labels.count()>0?1:0;*/


    endInsertRows();

    QSqlDatabase::removeDatabase(_databasePath);
}

void ReadingListModel::addReadingListAt(const QString &name, const QModelIndex &mi)
{
    QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

    beginInsertRows(mi, 0, 0); //TODO calculate the right coordinates before inserting

    ReadingListItem * readingListParent = static_cast<ReadingListItem*>(mi.internalPointer());
    qulonglong id = DBHelper::insertReadingSubList(name,mi.data(IDRole).toULongLong(),readingListParent->childCount(),db);
    ReadingListItem * newItem;

    readingListParent->appendChild(newItem = new ReadingListItem(QList<QVariant>()
                                              << name
                                              << id
                                              << false
                                              << true
                                              << readingListParent->childCount()));

    items.insert(id, newItem);

    /*int pos = readingListParent->children().indexOf(newItem);

    pos += specialLists.count()+1+labels.count()+labels.count()>0?1:0;*/


    endInsertRows();

    QSqlDatabase::removeDatabase(_databasePath);
}

bool ReadingListModel::isEditable(const QModelIndex &mi)
{
    if(!mi.isValid())
        return false;
    ListItem * item = static_cast<ListItem*>(mi.internalPointer());
    return typeid(*item) != typeid(SpecialListItem);
}

bool ReadingListModel::isReadingList(const QModelIndex &mi)
{
    if(!mi.isValid())
        return false;
    ListItem * item = static_cast<ListItem*>(mi.internalPointer());
    return typeid(*item) == typeid(ReadingListItem);
}

bool ReadingListModel::isReadingSubList(const QModelIndex &mi)
{
    if(!mi.isValid())
        return false;
    ListItem * item = static_cast<ListItem*>(mi.internalPointer());
    if(typeid(*item) == typeid(ReadingListItem))
    {
        ReadingListItem * readingListItem = static_cast<ReadingListItem *>(item);
        if(readingListItem->parent == rootItem)
            return false;
        else
            return true;
    }
    else
        return false;
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

    ListItem * item = static_cast<ListItem*>(mi.internalPointer());

    if(typeid(*item) == typeid(ReadingListItem))
    {
        ReadingListItem * rli = static_cast<ReadingListItem*>(item);
        rli->setName(name);
        DBHelper::renameList(item->getId(), name, db);

        if(rli->parent->getId()!=0)
        {
            //TODO
            //move row depending on the name
        }else
            emit dataChanged(index(mi.row(), 0), index(mi.row(), 0));
    }
    else if(typeid(*item) == typeid(LabelItem))
    {
        LabelItem * li = static_cast<LabelItem*>(item);
        li->setName(name);
        DBHelper::renameLabel(item->getId(), name, db);
        emit dataChanged(index(mi.row(), 0), index(mi.row(), 0));
    }

    QSqlDatabase::removeDatabase(_databasePath);
}

void ReadingListModel::deleteItem(const QModelIndex &mi)
{
    if(isEditable(mi))
    {
        QLOG_DEBUG() << "parent row :" << mi.parent().data() << "-" << mi.row();
        beginRemoveRows(mi.parent(),mi.row(),mi.row());

        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        ListItem * item = static_cast<ListItem*>(mi.internalPointer());

        if(typeid(*item) == typeid(ReadingListItem))
        {
            ReadingListItem * rli = static_cast<ReadingListItem*>(item);
            QLOG_DEBUG() << "num children : " << rli->parent->childCount();
            rli->parent->removeChild(rli);
            QLOG_DEBUG() << "num children : " << rli->parent->childCount();
            DBHelper::removeListFromDB(item->getId(), db);
            if(rli->parent->getId()!=0)
            {
                reorderingChildren(rli->parent->children());
            }
            QLOG_DEBUG() << "num children : " << rli->parent->childCount();
        }
        else if(typeid(*item) == typeid(LabelItem))
        {
            LabelItem * li = static_cast<LabelItem*>(item);
            labels.removeOne(li);
            DBHelper::removeLabelFromDB(item->getId(), db);
        }

        QSqlDatabase::removeDatabase(_databasePath);

        endRemoveRows();
    }
}

const QList<LabelItem *> ReadingListModel::getLabels()
{
    return labels;
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

void ReadingListModel::setupReadingListsData(QSqlQuery &sqlquery, ReadingListItem *parent)
{
    items.insert(parent->getId(),parent);

    while (sqlquery.next())
    {
        QSqlRecord record = sqlquery.record();
        ReadingListItem * rli = new ReadingListItem(QList<QVariant>()
                                                    << record.value("name")
                                                    << record.value("id")
                                                    << record.value("finished")
                                                    << record.value("completed")
                                                    << record.value("ordering"));

        ReadingListItem * currentParent;
        if(record.value("parentId").isNull())
            currentParent = rootItem;
        else
            currentParent = items.value(record.value("parentId").toULongLong());

        currentParent->appendChild(rli);

        items.insert(rli->getId(),rli);
    }
}

QList<SpecialListItem *> ReadingListModel::setupSpecialLists(QSqlDatabase & db)
{
    QList<SpecialListItem *> list;

    QSqlQuery selectQuery("SELECT * FROM default_reading_list ORDER BY id,name",db);
    while(selectQuery.next()) {
        QSqlRecord record = selectQuery.record();
        list << new SpecialListItem(QList<QVariant>()
                                    << record.value("name")
                                    << record.value("id"));
    }

    //Reading after Favorites, Why? Because I want to :P
    list.insert(1,new SpecialListItem(QList<QVariant>()  << "Reading" << 0));

    return list;
}

void ReadingListModel::setupLabels(QSqlDatabase & db)
{
    QSqlQuery selectQuery("SELECT * FROM label ORDER BY ordering,name",db); //TODO add some kind of
    while(selectQuery.next()) {
        QSqlRecord record = selectQuery.record();
        addLabelIntoList(new LabelItem(QList<QVariant>()
                                       << record.value("name")
                                       << record.value("color")
                                       << record.value("id")
                                       << record.value("ordering")));
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

}

void ReadingListModel::setupReadingLists(QSqlDatabase & db)
{
    //setup root item
    rootItem = new ReadingListItem(QList<QVariant>() << "ROOT" << 0 << true << false);

    QSqlQuery selectQuery("select * from reading_list order by parentId IS NULL DESC",db);

    //setup reading lists
    setupReadingListsData(selectQuery,rootItem);

    //TEST
//    ReadingListItem * node1;
//    rootItem->appendChild(node1 = new ReadingListItem(QList<QVariant>() /*<< 0*/ << "My reading list" << "atr"));
//    rootItem->appendChild(new ReadingListItem(QList<QVariant>() /*<< 0*/ << "X timeline" << "atr"));

//    node1->appendChild(new ReadingListItem(QList<QVariant>() /*<< 0*/ << "sublist" << "atr",node1));
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
                while( i < labels.count() && labels.at(i)->colorid() == item->colorid() && naturalSortLessThanCI(labels.at(i)->name(),item->name()))
                    i++;
            }
        }


        if(i >= labels.count())
        {
            QLOG_DEBUG() << "insertando label al final " << item->name();
            labels << item;
        }
        else
        {
            QLOG_DEBUG() << "insertando label en  " << i << "-" << item->name();
            labels.insert(i,item);
        }

        return i;
    }

    return 0;
}

void ReadingListModel::reorderingChildren(QList<ReadingListItem *> children)
{
    QList<qulonglong> childrenIds;
    int i = 0;
    foreach (ReadingListItem * item, children) {
        item->setOrdering(i++);
        childrenIds << item->getId();
    }

    QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
    DBHelper::reasignOrderToSublists(childrenIds, db);
    QSqlDatabase::removeDatabase(_databasePath);
}

bool ReadingListModel::rowIsSpecialList(int row, const QModelIndex &parent) const
{
    if(parent.isValid())
        return false; //by now no sublists in special list

    if(row >=0 && row < specialLists.count())
        return true;

    return false;
}

bool ReadingListModel::rowIsLabel(int row, const QModelIndex &parent) const
{
    if(parent.isValid())
        return false; //by now no sublists in labels

    if(row > specialLists.count()  && row <= specialLists.count() + labels.count())
        return true;

    return false;
}

bool ReadingListModel::rowIsReadingList(int row, const QModelIndex &parent) const
{
    if(parent.isValid())
        return true; //only lists with sublists

    int separatorsCount = labels.isEmpty()?1:2;

    if(row >= specialLists.count() + labels.count() + separatorsCount)
        return true;

    return false;
}

bool ReadingListModel::rowIsSeparator(int row, const QModelIndex &parent) const
{
    if(parent.isValid())
        return false; //only separators at top level

    if(row == specialLists.count())
        return true;

     int separatorsCount = labels.isEmpty()?1:2;
     if(separatorsCount == 2 && row == specialLists.count() + labels.count() + 1)
         return true;

     return false;
}

ReadingListModelProxy::ReadingListModelProxy(QObject *parent)
    :QSortFilterProxyModel(parent)
{

}
