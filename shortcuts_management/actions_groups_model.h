#ifndef ACTIONS_GROUPS_MODEL_H
#define ACTIONS_GROUPS_MODEL_H

#include <QAbstractItemModel>
#include <QIcon>

class QAction;

class ActionsGroup
{
public:
    ActionsGroup(const QString &name, const QIcon &icon, QList<QAction *> &actions);
    QString getName() const;
    QIcon getIcon() const;
    QList<QAction *> getActions() const;

protected:
    QString name;
    QIcon icon;
    QList<QAction *> actions;
};

class ActionsGroupsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ActionsGroupsModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex parent(const QModelIndex &index) const;

    void addActionsGroup(const ActionsGroup &group);
    QList<QAction *> getActions(const QModelIndex &mi);
signals:

public slots:

protected:
    QList<ActionsGroup> groups;
};

#endif // ACTIONS_GROUPS_MODEL_H
