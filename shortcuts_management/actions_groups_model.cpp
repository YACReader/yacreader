#include "actions_groups_model.h"

ActionsGroupsModel::ActionsGroupsModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

int ActionsGroupsModel::rowCount(const QModelIndex &parent) const
{
    return groups.length();
}

int ActionsGroupsModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex ActionsGroupsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, &groups.at(row).getActions());
}

QVariant ActionsGroupsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DecorationRole)
        return QVariant(groups.at(index.row()).getIcon());

    if (role != Qt::DisplayRole)
        return QVariant();

    return QVariant(groups[index.row()].getName());
}

QModelIndex ActionsGroupsModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

void ActionsGroupsModel::addActionsGroup(const ActionsGroup &group)
{
    beginInsertRows(QModelIndex(),groups.length()-1,groups.length());
    groups.push_back(group);
    endInsertRows();
}

QList<QAction *> ActionsGroupsModel::getActions(const QModelIndex &mi)
{
    if(mi.isValid())
        return groups[mi.row()].getActions();
    return QList<QAction *>();
}

//-------------------------------------------------------------------

ActionsGroup::ActionsGroup(const QString &name, const QIcon &icon, QList<QAction *> &actions)
    :name(name), icon(icon), actions(actions)
{

}

QString ActionsGroup::getName() const
{
    return name;
}

QIcon ActionsGroup::getIcon() const
{
    return icon;
}

QList<QAction *> ActionsGroup::getActions() const
{
    return actions;
}
