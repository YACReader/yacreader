#include "reading_list_management_coordinator.h"

#include "add_label_dialog.h"
#include "cbl_import_dialog.h"
#include "cbl_list_match.h"
#include "comic_model.h"
#include "move_to_folder_dialog.h"
#include "new_list_folder_dialog.h"
#include "reading_list_model.h"
#include "smart_list_dialog.h"
#include "yacreader_global.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>
#include <QWidget>

#include <utility>

ReadingListManagementCoordinator::ReadingListManagementCoordinator(QWidget *dialogParent,
                                                                   ReadingListModel *listsModel,
                                                                   ComicModel *comicsModel,
                                                                   CurrentListProvider currentListProvider,
                                                                   LibraryPathProvider libraryPathProvider,
                                                                   QSettings *settings)
    : QObject(dialogParent), dialogParent(dialogParent), listsModel(listsModel), currentListProvider(std::move(currentListProvider)), libraryPathProvider(std::move(libraryPathProvider)), settings(settings)
{
    connect(listsModel, &ReadingListModel::addComicsToFavorites, comicsModel, QOverload<const QList<qulonglong> &>::of(&ComicModel::addComicsToFavorites));
    connect(listsModel, &ReadingListModel::addComicsToLabel, comicsModel, QOverload<const QList<ComicRowId> &, qulonglong>::of(&ComicModel::addComicsToLabel));
    connect(listsModel, &ReadingListModel::addComicsToReadingList, comicsModel, QOverload<const QList<ComicRowId> &, qulonglong>::of(&ComicModel::addComicsToReadingList));
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
        listsModel->addReadingList(name, currentList);
}

void ReadingListManagementCoordinator::importReadingLists()
{
    const auto filePaths = QFileDialog::getOpenFileNames(dialogParent,
                                                         tr("Import reading lists"),
                                                         { },
                                                         tr("Reading lists (*.cbl *.json);;All files (*)"));
    if (filePaths.isEmpty())
        return;

    CblImportDialog dialog(filePaths, libraryPathProvider(), dialogParent);
    connect(&dialog, &CblImportDialog::readingListImported, listsModel, &ReadingListModel::appendReadingList);
    dialog.exec();
}

void ReadingListManagementCoordinator::matchListToLibraryComics(ListKind listKind, qulonglong listId, const QString &name)
{
    const auto libraryPath = libraryPathProvider();
    ComicModel listModel;
    if (listKind == ListKind::Label)
        listModel.setupLabelModelData(listId, YACReader::LibraryPaths::libraryDataPath(libraryPath));
    else
        listModel.setupReadingListModelData(listId, YACReader::LibraryPaths::libraryDataPath(libraryPath));

    QList<CblListEntry> entries;
    for (int row = 0; row < listModel.rowCount(); ++row) {
        const auto index = listModel.index(row, 0);
        if (const auto entry = listModel.listEntry(index))
            entries.append(cblListEntry(listModel.record(index), *entry));
    }

    CblImportDialog dialog(name, entries, libraryPath, dialogParent);
    connect(&dialog, &CblImportDialog::listEntriesChanged, this, &ReadingListManagementCoordinator::listEntriesChanged);
    dialog.exec();
}

void ReadingListManagementCoordinator::addSmartList(const QString &query)
{
    const QPersistentModelIndex currentList(currentListProvider());
    SmartListDialog dialog(settings, QString(), query, dialogParent);
    connect(&dialog, &SmartListDialog::searchChanged, this, &ReadingListManagementCoordinator::searchRequested);
    if (!query.isEmpty())
        emit searchRequested(query);
    const auto accepted = dialog.exec() == QDialog::Accepted;
    emit searchFinished();
    if (!accepted)
        return;
    if (!listsModel->addSmartList(dialog.name(), dialog.query(), currentList).isValid())
        QMessageBox::critical(dialogParent, tr("New smart list"), tr("The smart list could not be saved."));
}

void ReadingListManagementCoordinator::editSmartList(const QModelIndex &list)
{
    const auto smartList = listsModel->smartList(list);
    if (!smartList)
        return;

    const QPersistentModelIndex index(list);
    SmartListDialog dialog(settings, smartList->name, smartList->query, dialogParent);
    connect(&dialog, &SmartListDialog::searchChanged, this, &ReadingListManagementCoordinator::searchRequested);
    emit searchRequested(smartList->query);
    const auto accepted = dialog.exec() == QDialog::Accepted;
    emit searchFinished();
    if (!accepted || !index.isValid())
        return;
    if (!listsModel->updateSmartList(index, dialog.name(), dialog.query())) {
        QMessageBox::critical(dialogParent, tr("Edit smart list"), tr("The smart list could not be saved."));
        return;
    }
    emit listEntriesChanged();
}

void ReadingListManagementCoordinator::deleteCurrentList()
{
    deleteList(currentListProvider());
}

void ReadingListManagementCoordinator::deleteList(const QModelIndex &currentList)
{
    if (!currentList.isValid() || !listsModel->isEditable(currentList))
        return;

    const auto answer = listsModel->listFolder(currentList)
            ? QMessageBox::question(dialogParent,
                                    tr("Delete folder"),
                                    tr("Delete the folder \"%1\" and everything in it? Its folders and lists are deleted; your comic files are not.").arg(listsModel->name(currentList)),
                                    QMessageBox::Yes,
                                    QMessageBox::No)
            : QMessageBox::question(dialogParent,
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
    renameList(currentListProvider());
}

void ReadingListManagementCoordinator::renameList(const QModelIndex &currentList)
{
    if (!currentList.isValid() || !listsModel->isEditable(currentList))
        return;
    // the name and the search of a smart list change together
    if (listsModel->smartList(currentList)) {
        editSmartList(currentList);
        return;
    }

    const bool folder = listsModel->listFolder(currentList).has_value();
    const QPersistentModelIndex list(currentList);
    bool accepted = false;
    const auto name = QInputDialog::getText(dialogParent,
                                            folder ? tr("Rename folder") : tr("Rename list name"),
                                            folder ? tr("Folder name:") : tr("List name:"),
                                            QLineEdit::Normal,
                                            listsModel->name(currentList),
                                            &accepted);
    if (accepted && list.isValid())
        listsModel->rename(list, name);
}

void ReadingListManagementCoordinator::addListFolder()
{
    const auto currentList = currentListProvider();
    if (listsModel->section(currentList)) {
        addListFolderAt(currentList);
        return;
    }

    NewListFolderDialog dialog(dialogParent);
    if (dialog.exec() == QDialog::Accepted && !listsModel->addListFolder(dialog.section(), QModelIndex(), dialog.name()).isValid())
        QMessageBox::critical(dialogParent, tr("New folder"), tr("The folder could not be created."));
}

void ReadingListManagementCoordinator::addListFolderAt(const QModelIndex &list)
{
    const auto section = listsModel->section(list);
    if (!section)
        return;

    // inside a folder only when the folder itself is chosen
    const QPersistentModelIndex parentFolder(listsModel->listFolder(list) ? list : QModelIndex());
    bool accepted = false;
    const auto name = QInputDialog::getText(dialogParent, tr("New folder"), tr("Folder name:"), QLineEdit::Normal, { }, &accepted).trimmed();
    if (!accepted || name.isEmpty())
        return;
    if (!listsModel->addListFolder(*section, parentFolder, name).isValid())
        QMessageBox::critical(dialogParent, tr("New folder"), tr("The folder could not be created."));
}

void ReadingListManagementCoordinator::exportReadingList(const QModelIndex &list)
{
    if (!listsModel->isReadingList(list))
        return;

    const auto listId = list.data(ReadingListModel::IDRole).toULongLong();
    const auto name = listsModel->name(list);
    auto fileName = name;
    // characters a file name cannot have on Windows
    fileName.replace(QRegularExpression(QStringLiteral(R"([<>:"/\\|?*])")), QStringLiteral("_"));
    if (fileName.trimmed().isEmpty())
        fileName = tr("Reading list");

    const auto xmlFilter = tr("CBL reading list (*.cbl)");
    const auto jsonFilter = tr("JSON reading list (*.json)");
    QString selectedFilter = xmlFilter;
    auto path = QFileDialog::getSaveFileName(dialogParent,
                                             tr("Export reading list"),
                                             QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).filePath(fileName + ".cbl"),
                                             xmlFilter + ";;" + jsonFilter,
                                             &selectedFilter);
    if (path.isEmpty())
        return;

    const auto suffix = QFileInfo(path).suffix().toLower();
    const auto format = suffix == QLatin1StringView("json") || (suffix != QLatin1StringView("cbl") && selectedFilter == jsonFilter) ? CblReadingList::Json : CblReadingList::Xml;
    if (suffix != QLatin1StringView("json") && suffix != QLatin1StringView("cbl"))
        path += format == CblReadingList::Json ? ".json" : ".cbl";

    // the rows the list shows: its entries, then the entries of its child lists
    ComicModel listModel;
    listModel.setupReadingListModelData(listId, YACReader::LibraryPaths::libraryDataPath(libraryPathProvider()));
    QList<CblExportBook> books;
    for (int row = 0; row < listModel.rowCount(); ++row) {
        const auto index = listModel.index(row, 0);
        if (const auto entry = listModel.listEntry(index))
            books.append(cblExportBook(listModel.record(index), *entry));
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(writeCbl(name, books, format, QUuid::createUuid().toString(QUuid::WithoutBraces))) < 0 || !file.commit())
        QMessageBox::critical(dialogParent, tr("Export reading list"), tr("The file could not be written."));
}

void ReadingListManagementCoordinator::moveToFolder(const QModelIndex &list)
{
    if (!listsModel->section(list) || listsModel->isReadingSubList(list))
        return;

    const QPersistentModelIndex item(list);
    MoveToFolderDialog dialog(listsModel, list, dialogParent);
    if (dialog.exec() != QDialog::Accepted || !item.isValid())
        return;
    if (!listsModel->moveToFolder(item, dialog.folder()))
        QMessageBox::critical(dialogParent, tr("Move to folder"), tr("\"%1\" could not be moved.").arg(listsModel->name(item)));
}
