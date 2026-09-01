#include "reading_list_management_coordinator.h"

#include "add_label_dialog.h"
#include "cbl_reader.h"
#include "comic_model.h"
#include "reading_list_model.h"

#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeySequence>
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

    // Phase 1: expose the importer immediately without touching the database.
    // The permanent toolbar action will replace this temporary shortcut once the
    // import preview and matching flow are settled.
    auto *importCblAction = new QAction(tr("Import CBL Reading List..."), dialogParent);
    importCblAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+I")));
    importCblAction->setShortcutContext(Qt::ApplicationShortcut);
    dialogParent->addAction(importCblAction);
    connect(importCblAction, &QAction::triggered, this, &ReadingListManagementCoordinator::importCblReadingList);
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

void ReadingListManagementCoordinator::importCblReadingList()
{
    const auto filePath = QFileDialog::getOpenFileName(dialogParent,
                                                       tr("Import CBL reading list"),
                                                       { },
                                                       tr("Comic Book Reading Lists (*.cbl);;XML files (*.xml);;All files (*)"));
    if (filePath.isEmpty())
        return;

    const auto result = CblReader::read(filePath);
    if (!result.success) {
        QString details = result.errorMessage;
        if (result.errorLine > 0)
            details += tr("\n\nLine %1, column %2").arg(result.errorLine).arg(result.errorColumn);

        QMessageBox::critical(dialogParent, tr("Unable to import CBL"), details);
        return;
    }

    QString preview;
    const int previewLimit = 20;
    const int count = result.readingList.books.size();

    for (int i = 0; i < count && i < previewLimit; ++i) {
        const auto &book = result.readingList.books.at(i);
        preview += QStringLiteral("%1. %2 #%3")
                           .arg(i + 1)
                           .arg(book.series.isEmpty() ? tr("Unknown series") : book.series)
                           .arg(book.number.isEmpty() ? QStringLiteral("?") : book.number);

        if (!book.volume.isEmpty())
            preview += tr(" (Vol. %1)").arg(book.volume);
        if (!book.year.isEmpty())
            preview += tr(" [%1]").arg(book.year);
        preview += QLatin1Char('\n');
    }

    if (count > previewLimit)
        preview += tr("\n...and %1 more entries.").arg(count - previewLimit);

    QMessageBox::information(dialogParent,
                             tr("CBL reading list parsed"),
                             tr("%1\n\n%2 entries found.\n\n%3\nThis is a preview only. No changes have been made to your library yet.")
                                     .arg(result.readingList.name)
                                     .arg(count)
                                     .arg(preview));
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
