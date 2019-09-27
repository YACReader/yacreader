#include "edit_shortcuts_dialog.h"

#include "actions_groups_model.h"
#include "actions_shortcuts_model.h"
#include "edit_shortcut_item_delegate.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QListView>
#include <QTableView>
#include <QPushButton>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>

#include "QsLog.h"

EditShortcutsDialog::EditShortcutsDialog(QWidget *parent)
    : QDialog(parent)
{
    QPushButton *resetButton = new QPushButton(tr("Restore defaults"), this);
    QLabel *infoLabel = new QLabel(tr("To change a shortcut, double click in the key combination and type the new keys."));
    QVBoxLayout *layout = new QVBoxLayout(this);
    QSplitter *splitter = new QSplitter(this);
    actionsGroupsListView = new QListView(this);

    actionsTableView = new QTableView(this);
    actionsTableView->verticalHeader()->setHidden(true);
    actionsTableView->horizontalHeader()->setHidden(true);
    splitter->addWidget(actionsGroupsListView);
    splitter->addWidget(actionsTableView);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes(QList<int>() << 200 << 400);

    layout->addWidget(infoLabel, 0);
    layout->addWidget(splitter, 1);
    layout->addWidget(resetButton, 0, Qt::AlignRight);

    setLayout(layout);

    groupsModel = new ActionsGroupsModel();
    actionsModel = new ActionsShortcutsModel();
    actionsGroupsListView->setModel(groupsModel);
    actionsGroupsListView->setFocus();
    actionsTableView->setModel(actionsModel);
    actionsTableView->setColumnWidth(0, 30);
    actionsTableView->setColumnWidth(1, 360);
    //actionsTableView->horizontalHeader()->sectionResizeMode(QHeaderView::Custom);
    actionsTableView->horizontalHeader()->setStretchLastSection(true);
    actionsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    actionsTableView->setShowGrid(false);
    actionsTableView->setItemDelegateForColumn(ActionsShortcutsModel::KEYS, new EditShortcutItemDelegate(this));
    actionsTableView->installEventFilter(this);
    /*actionsTableView->setStyleSheet("QTableView {outline: 0px;}"
                                      "QTableView::item {outline: 0px;}");
                                      "QTableView {border:0px;}"
                                      "QTableView::item:selected {outline: 0px; border: 0px;}"
                                      "");*/

    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetToDefaults()));
    connect(actionsGroupsListView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(loadShortcuts(QModelIndex, QModelIndex))); //clicked(QModelIndex) doesn't work :S
    connect(actionsModel, SIGNAL(conflict(QString)), this, SLOT(processConflict(QString)));

#ifdef Q_OS_MAC
    setFixedSize(760, 500);
#else
    setFixedSize(804, 500); //extra width for modifiers
#endif
    setWindowTitle(tr("Shortcuts settings"));

    setModal(true);
}

void EditShortcutsDialog::addActionsGroup(const QString &name, const QIcon &ico, QList<QAction *> &group)
{
    groupsModel->addActionsGroup(ActionsGroup(name, ico, group));
    if (actionsTableView->model()->rowCount() == 0) //first group added
        actionsGroupsListView->selectionModel()->select(groupsModel->index(0, 0), QItemSelectionModel::Select);
}

void EditShortcutsDialog::resetToDefaults()
{
}

void EditShortcutsDialog::loadShortcuts(const QModelIndex &mi, const QModelIndex &mi2)
{
    Q_UNUSED(mi2);

    actionsModel->addActions(groupsModel->getActions(mi));
}

void EditShortcutsDialog::processConflict(const QString &shortcutInConflict)
{
    QMessageBox::warning(this, tr("Shortcut in use"), QString(tr("The shortcut \"%1\" is already assigned to other function")).arg(shortcutInConflict));
}
