#ifndef EDIT_SHORTCUTS_DIALOG_H
#define EDIT_SHORTCUTS_DIALOG_H

#include <QDialog>
#include <QModelIndex>

class QListView;
class QTableView;

class ActionsGroupsModel;
class ActionsShortcutsModel;

class EditShortcutsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditShortcutsDialog(QWidget * parent = 0);
    void addActionsGroup(const QString & name, const QIcon & ico, QList<QAction *> & group);
signals:

public slots:
    void resetToDefaults();
    void loadShortcuts(const QModelIndex & mi,const QModelIndex &mi2);
    void processConflict(const QString & shortcutInConflict);

protected:
    QListView * actionsGroupsListView;
    QTableView * actionsTableView;
    ActionsGroupsModel * groupsModel;
    ActionsShortcutsModel * actionsModel;
};

#endif // EDIT_SHORTCUTS_DIALOG_H
