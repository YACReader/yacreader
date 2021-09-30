#include "actions_shortcuts_model.h"
#include "shortcuts_manager.h"

#include <QAction>

ActionsShortcutsModel::ActionsShortcutsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

int ActionsShortcutsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return actions.length();
}

int ActionsShortcutsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 3;
}

QModelIndex ActionsShortcutsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, actions[row]);
}

Qt::ItemFlags ActionsShortcutsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};
    if (index.column() == KEYS)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ActionsShortcutsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DecorationRole && index.column() == ICON)
        return QVariant(actions[index.row()]->icon());

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case ICON:
            return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
        case NAME:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        case KEYS:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
    }

    if (role == Qt::ForegroundRole && index.column() == KEYS && actions[index.row()]->shortcut().isEmpty())
        return QBrush(QColor(0xAAAAAA));

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() == NAME)
        return QVariant(actions[index.row()]->toolTip());
    if (index.column() == KEYS) {
        QKeySequence ks = actions[index.row()]->shortcut();
        if (ks.isEmpty())
            return tr("None");
        return QVariant(ks.toString(QKeySequence::NativeText));
    }

    return QVariant();
}

bool ActionsShortcutsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role);

    if (index.column() == KEYS) {
        ShortcutsManager sm = ShortcutsManager::getShortcutsManager();
        if (sm.checkConflicts(value.toString(), actions[index.row()]))
            emit conflict(value.toString());
        else {
            actions[index.row()]->setShortcut(value.toString());
            ShortcutsManager::getShortcutsManager().saveShortcut(actions[index.row()]);
            return true;
        }
    }
    return false;
}

QModelIndex ActionsShortcutsModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);

    return QModelIndex();
}

void ActionsShortcutsModel::addActions(const QList<QAction *> actions)
{
    beginResetModel();
    this->actions = actions;
    endResetModel();
}
