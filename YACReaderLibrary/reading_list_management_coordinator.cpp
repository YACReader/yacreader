#include "reading_list_management_coordinator.h"

#include "add_label_dialog.h"
#include "cbl_import_dialog.h"
#include "cbl_list_match.h"
#include "comic_model.h"
#include "delayed_progress_dialog.h"
#include "move_to_folder_dialog.h"
#include "new_list_folder_dialog.h"
#include "reading_list_model.h"
#include "reading_list_page.h"
#include "smart_list_dialog.h"
#include "yacreader_global.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>
#include <QWidget>

#include <algorithm>
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

    // the file types, with their suffixes
    const QList<std::pair<QString, QStringList>> types {
        { tr("CBL reading list (*.cbl)"), { "cbl" } },
        { tr("JSON reading list (*.json)"), { "json" } },
        { tr("Web page (*.html)"), { "html", "htm" } },
    };
    QStringList filters;
    for (const auto &type : types)
        filters.append(type.first);
    QString selectedFilter = filters.first();
    auto path = QFileDialog::getSaveFileName(dialogParent,
                                             tr("Export reading list"),
                                             QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).filePath(fileName + ".cbl"),
                                             filters.join(";;"),
                                             &selectedFilter);
    if (path.isEmpty())
        return;

    // the suffix gives the type, else the chosen filter (and its suffix is added)
    const auto suffix = QFileInfo(path).suffix().toLower();
    auto type = std::find_if(types.cbegin(), types.cend(), [&](const auto &type) { return type.second.contains(suffix); });
    if (type == types.cend()) {
        type = std::find_if(types.cbegin(), types.cend(), [&](const auto &type) { return type.first == selectedFilter; });
        if (type == types.cend())
            type = types.cbegin();
        path += "." + type->second.first();
    }

    // the rows the list shows: its entries, then the entries of its child lists
    ComicModel listModel;
    listModel.setupReadingListModelData(listId, YACReader::LibraryPaths::libraryDataPath(libraryPathProvider()));
    if (type->second.first() == QLatin1StringView("html")) {
        QList<ReadingListPageEntry> entries;
        QStringList coverPaths;
        for (int row = 0; row < listModel.rowCount(); ++row) {
            const auto index = listModel.index(row, 0);
            const auto &metadata = recordMetadata(listModel.record(index));
            const auto cover = index.data(ComicModel::CoverPathRole).toUrl();
            entries.append({ index.data(ComicModel::TitleRole).toString(),
                             readingListPageDetails(metadata.series.toString(), metadata.number.toString(), metadata.volume.toString(), metadata.date.toString()),
                             QByteArray(),
                             !index.data(ComicModel::AvailableRole).toBool() });
            coverPaths.append(cover.isLocalFile() ? cover.toLocalFile() : QString());
        }
        exportReadingListPage(path, name, entries, coverPaths);
        return;
    }

    QList<CblExportBook> books;
    for (int row = 0; row < listModel.rowCount(); ++row) {
        const auto index = listModel.index(row, 0);
        if (const auto entry = listModel.listEntry(index))
            books.append(cblExportBook(listModel.record(index), *entry));
    }
    const auto format = type->second.first() == QLatin1StringView("json") ? CblReadingList::Json : CblReadingList::Xml;
    writeExportFile(path, writeCbl(name, books, format, QUuid::createUuid().toString(QUuid::WithoutBraces)));
}

void ReadingListManagementCoordinator::exportReadingListPage(const QString &path, const QString &name, const QList<ReadingListPageEntry> &entries, const QStringList &coverPaths)
{
    auto covers = std::make_shared<ReadingListPageCovers>(coverPaths);
    auto *progress = new DelayedProgressDialog(tr("Exporting reading list..."), covers->count(), dialogParent);
    auto *watcher = new QFutureWatcher<QByteArray>(progress);
    connect(watcher, &QFutureWatcher<QByteArray>::progressValueChanged, progress, &DelayedProgressDialog::setValue);
    connect(progress, &DelayedProgressDialog::canceled, watcher, &QFutureWatcher<QByteArray>::cancel);
    connect(watcher, &QFutureWatcher<QByteArray>::finished, this, [this, covers, progress, watcher, path, name, entries, coverPaths] {
        const auto canceled = watcher->isCanceled();
        progress->finish([this, covers, canceled, path, name, entries, coverPaths] {
            if (canceled)
                return;
            auto pageEntries = entries;
            for (qsizetype i = 0; i < pageEntries.size(); ++i)
                pageEntries[i].cover = covers->cover(coverPaths.at(i));
            writeExportFile(path, writeReadingListPage(name, pageEntries, QDate::currentDate()));
        });
    });
    watcher->setFuture(covers->start());
}

void ReadingListManagementCoordinator::writeExportFile(const QString &path, const QByteArray &content)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(content) < 0 || !file.commit())
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
