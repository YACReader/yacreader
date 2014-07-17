#ifndef SHORTCUTS_DIALOG_H
#define SHORTCUTS_DIALOG_H

#include <QDialog>
#include <QModelIndex>

class QListView;
class QTableView;

class ActionsGroupsModel;
class ActionsShortcutsModel;

class ShortcutsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ShortcutsDialog(QWidget * parent = 0);
    void addActionsGroup(const QString & name, const QIcon & ico, QList<QAction *> & group);
signals:

public slots:
    void resetToDefaults();
    void loadShortcuts(const QModelIndex & mi,const QModelIndex &mi2);

protected:
    QListView * actionsGroupsListView;
    QTableView * actionsTableView;
    ActionsGroupsModel * groupsModel;
    ActionsShortcutsModel * actionsModel;
};

#endif // SHORTCUTS_DIALOG_H
