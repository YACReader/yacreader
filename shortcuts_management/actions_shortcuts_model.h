#ifndef ACTIONS_SHORTCUTS_MODEL_H
#define ACTIONS_SHORTCUTS_MODEL_H

#include <QAbstractItemModel>

class QAction;

class ActionsShortcutsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ActionsShortcutsModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QModelIndex parent(const QModelIndex &index) const;

    void addActions(const QList<QAction *> actions);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    enum Columns {
        ICON = 0,
        NAME,
        KEYS
    };
signals:
    void conflict(QString);
public slots:

protected:
    QList<QAction *> actions;
};

#endif // ACTIONS_SHORTCUTS_MODEL_H
