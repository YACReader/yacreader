#ifndef EDIT_SHORTCUTS_DIALOG_H
#define EDIT_SHORTCUTS_DIALOG_H

#include <QDialog>
#include <QModelIndex>

#include "themable.h"

class QListView;
class QTableView;

class ActionsGroupsModel;
class ActionsShortcutsModel;

class EditShortcutsDialog : public QDialog, protected Themable
{
    Q_OBJECT
public:
    explicit EditShortcutsDialog(QWidget *parent = 0);
    void addActionsGroup(const QString &name, const QIcon &ico, QList<QAction *> &group);
    void setGroupIconMapping(const QMap<QString, std::function<QIcon(const Theme &)>> &mapping);
signals:

public slots:
    void resetToDefaults();
    void loadShortcuts(const QModelIndex &mi, const QModelIndex &mi2);
    void processConflict(const QString &shortcutInConflict);

protected:
    void applyTheme(const Theme &theme) override;
    void updateGroupIcons(const Theme &theme);

    QListView *actionsGroupsListView;
    QTableView *actionsTableView;
    ActionsGroupsModel *groupsModel;

    QMap<QString, std::function<QIcon(const Theme &)>> groupIconMapping;
    ActionsShortcutsModel *actionsModel;
};

#endif // EDIT_SHORTCUTS_DIALOG_H
