#include "shortcuts_dialog.h"

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

#include "QsLog.h"

ShortcutsDialog::ShortcutsDialog(QWidget *parent) :
    QDialog(parent)
{
    QPushButton * resetButton = new QPushButton(tr("Restore defaults"),this);
    QLabel * infoLabel = new QLabel(tr("To change a shortcut, select it, click in the key combination and type the new keys."));
    QVBoxLayout * layout = new QVBoxLayout(this);
    QSplitter * splitter = new QSplitter(this);
    actionsGroupsListView = new QListView(this);

    actionsTableView = new QTableView(this);
    actionsTableView->verticalHeader()->setHidden(true);
    actionsTableView->horizontalHeader()->setHidden(true);
    splitter->addWidget(actionsGroupsListView);
    splitter->addWidget(actionsTableView);
    splitter->setStretchFactor(1,1);
    splitter->setSizes(QList<int>() << 200 << 400);

    layout->addWidget(infoLabel,0);
    layout->addWidget(splitter,1);
    layout->addWidget(resetButton,0,Qt::AlignRight);

    setLayout(layout);

    groupsModel = new ActionsGroupsModel();
    actionsModel = new ActionsShortcutsModel();
    actionsGroupsListView->setModel(groupsModel);
    actionsTableView->setModel(actionsModel);
    actionsTableView->setColumnWidth(0,30);
    actionsTableView->setColumnWidth(1,270);
    actionsTableView->horizontalHeader()->setStretchLastSection(true);
    actionsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    actionsTableView->setShowGrid(false);
    actionsTableView->setItemDelegateForColumn(ActionsShortcutsModel::KEYS,new EditShortcutItemDelegate(this));
    actionsTableView->installEventFilter(this);
    /*actionsTableView->setStyleSheet("QTableView {outline: 0px;}"
                                      "QTableView::item {outline: 0px;}");
                                      "QTableView {border:0px;}"
                                      "QTableView::item:selected {outline: 0px; border: 0px;}"
                                      "");*/


    connect(resetButton,SIGNAL(clicked()),this,SLOT(resetToDefaults()));
    connect(actionsGroupsListView->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(loadShortcuts(QModelIndex,QModelIndex))); //clicked(QModelIndex) doesn't work :S

    setFixedSize(640,480);
    setWindowTitle(tr("Shortcuts settings"));

    setModal(true);
}

void ShortcutsDialog::addActionsGroup(const QString &name, const QIcon &ico, QList<QAction *> &group)
{
    //TODO
    //groups model add
    groupsModel->addActionsGroup(ActionsGroup(name,ico,group));
}

void ShortcutsDialog::resetToDefaults()
{

}

void ShortcutsDialog::loadShortcuts(const QModelIndex &mi,const QModelIndex &mi2)
{
    actionsModel->addActions(groupsModel->getActions(mi));
}
