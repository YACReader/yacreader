#include "reading_list_management_coordinator.h"

#include "add_label_dialog.h"
#include "comic_model.h"
#include "reading_list_model.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QWidget>

#include <utility>

ReadingListManagementCoordinator::ReadingListManagementCoordinator(QWidget *dialogParent,
                                                                   ReadingListModel *listsModel,
                                                                   ComicModel *comicsModel,
                                                                   CurrentListProvider currentListProvider)
    : QObject(dialogParent), dialogParent(dialogParent), listsModel(listsModel), currentListProvider(std::move(currentListProvider))
{
    connect(listsModel, &ReadingListModel::addComicsToFavorites, comicsModel, QOverload<const QList<qulonglong> &>::of(&ComicModel::addComicsToFavorites));
    connect(listsModel, &ReadingListModel::addComicsToLabel, comicsModel, QOverload<const QList<qulonglong> &, qulonglong>::of(&ComicModel::addComicsToLabel));
    connect(listsModel, &ReadingListModel::addComicsToReadingList, comicsModel, QOverload<const QList<qulonglong> &, qulonglong>::of(&ComicModel::addComicsToReadingList));
}

void ReadingListManagementCoordinator::addReadingList()
{
    const auto currentList = currentListProvider();
    if (currentList.isValid() && listsModel->isReadingSubList(currentList))
        return;

    bool accepted = false;
    const auto name = QInputDialog::getText(dialogParent,
                                            tr("Add new reading lists"),
                                            tr("List name:"),
                                            QLineEdit::Normal,
                                            { },
                                            &accepted);
    if (!accepted)
        return;

    if (currentList.isValid() && listsModel->isReadingList(currentList))
        listsModel->addReadingListAt(name, currentList);
    else
        listsModel->addReadingList(name);
}

void ReadingListManagementCoordinator::deleteCurrentList()
{
    const auto currentList = currentListProvider();
    if (!currentList.isValid() || !listsModel->isEditable(currentList))
        return;

    const auto answer = QMessageBox::question(dialogParent,
                                              tr("Delete list/label"),
                                              tr("The selected item will be deleted, your comics or folders will NOT be deleted from your disk. Are you sure?"),
                                              QMessageBox::Yes,
                                              QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    listsModel->deleteItem(currentList);
    emit currentListReselectionRequested();
}

void ReadingListManagementCoordinator::addLabel()
{
    AddLabelDialog dialog(dialogParent);
    if (dialog.exec() == QDialog::Accepted)
        listsModel->addNewLabel(dialog.name(), dialog.selectedColor());
}

void ReadingListManagementCoordinator::renameCurrentList()
{
    const auto currentList = currentListProvider();
    if (!currentList.isValid() || !listsModel->isEditable(currentList))
        return;

    bool accepted = false;
    const auto name = QInputDialog::getText(dialogParent,
                                            tr("Rename list name"),
                                            tr("List name:"),
                                            QLineEdit::Normal,
                                            listsModel->name(currentList),
                                            &accepted);
    if (accepted)
        listsModel->rename(currentList, name);
}
