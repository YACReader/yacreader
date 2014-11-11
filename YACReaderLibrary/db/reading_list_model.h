#ifndef READING_LIST_MODEL_H
#define READING_LIST_MODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlDatabase>

#include "yacreader_global.h"

class LabelItem;
class SpecialListItem;
class ReadingListItem;
class ReadingListSeparatorItem;

class ReadingListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ReadingListModel(QObject *parent = 0);

    //QAbstractItemModel methods
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    //Convenience methods
    void setupModelData(QString path);

    void addNewLabel(const QString & name, YACReader::LabelColors color);

signals:

public slots:
    void deleteItem(const QModelIndex & mi);

private:
    void cleanAll();
    void setupModelData(QSqlQuery &sqlquery, ReadingListItem *parent);
    QList<SpecialListItem *> setupSpecialLists(QSqlDatabase &db);
    QList<LabelItem *> setupLabels(QSqlDatabase &db);
    void setupReadingLists(QSqlDatabase &db);
    int addLabelIntoList(LabelItem *item);

    //Special lists
    QList<SpecialListItem *> specialLists;

    //Label
    QList<LabelItem *> labels;

    //Reading lists
    ReadingListItem * rootItem; //
    QMap<unsigned long long int, ReadingListItem *> items; //lists relationship

    //separators
    ReadingListSeparatorItem * separator1;
    ReadingListSeparatorItem * separator2;

    QString _databasePath;

};

#endif // READING_LIST_MODEL_H
