#include "comic_management_coordinator.h"

#include "api_key_dialog.h"
#include "cbl_import_dialog.h"
#include "cbl_match_comic_dialog.h"
#include "comic_files_manager.h"
#include "comic_vine_dialog.h"
#include "comics_remover.h"
#include "cover_utils.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "folder_model.h"
#include "initial_comic_info_extractor.h"
#include "library_comic_match.h"
#include "library_comic_opener.h"
#include "placeholder_cover.h"
#include "properties_dialog.h"
#include "reading_list_model.h"
#include "staged_cover.h"
#include "yacreader_global_gui.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QThread>
#include <QUrl>
#include <QWidget>
#include <QsLog.h>

#include <algorithm>
#include <utility>

#ifdef Q_OS_WIN
#include <qt_windows.h>

#include <shellapi.h>
#endif

namespace {
template<class Remover>
void moveAndConnectRemoverToThread(Remover *remover, QThread *thread)
{
    Q_ASSERT(remover);
    Q_ASSERT(thread);
    remover->moveToThread(thread);
    QObject::connect(thread, &QThread::started, remover, &Remover::process);
    QObject::connect(remover, &Remover::finished, remover, &QObject::deleteLater);
    QObject::connect(remover, &Remover::finished, thread, &QThread::quit);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
}
}

ComicManagementCoordinator::ComicManagementCoordinator(QWidget *window,
                                                       QSettings *settings,
                                                       ComicModel *comicsModel,
                                                       FolderModel *foldersModel,
                                                       FolderModelProxy *foldersModelProxy,
                                                       PropertiesDialog *propertiesDialog,
                                                       ComicVineDialog *comicVineDialog,
                                                       SelectionProvider selectionProvider,
                                                       CurrentListProvider currentListProvider,
                                                       CurrentFolderProvider currentFolderProvider,
                                                       CurrentComicProvider currentComicProvider,
                                                       ComicOpeningAllowedProvider comicOpeningAllowedProvider,
                                                       LibraryIdProvider libraryIdProvider,
                                                       LibraryPathProvider libraryPathProvider)
    : QObject(window), window(window), settings(settings), comicsModel(comicsModel), foldersModel(foldersModel), foldersModelProxy(foldersModelProxy), propertiesDialog(propertiesDialog), comicVineDialog(comicVineDialog), selectionProvider(std::move(selectionProvider)), currentListProvider(std::move(currentListProvider)), currentFolderProvider(std::move(currentFolderProvider)), currentComicProvider(std::move(currentComicProvider)), comicOpeningAllowedProvider(std::move(comicOpeningAllowedProvider)), libraryIdProvider(std::move(libraryIdProvider)), libraryPathProvider(std::move(libraryPathProvider))
{
    connect(propertiesDialog, &PropertiesDialog::coverChangedSignal, comicsModel, &ComicModel::notifyCoverChange);
    connect(propertiesDialog, &PropertiesDialog::placeholderCoverChanged, comicsModel, &ComicModel::notifyPlaceholderCoverChange);
    connect(propertiesDialog, &QDialog::accepted, this, &ComicManagementCoordinator::currentSourceRefreshAccepted);
    connect(propertiesDialog, &QDialog::rejected, this, &ComicManagementCoordinator::currentSourceRefreshCancelled);
    connect(comicVineDialog, &QDialog::accepted, this, &ComicManagementCoordinator::currentSourceRefreshAccepted, Qt::QueuedConnection);
    connect(comicVineDialog, &QDialog::rejected, this, &ComicManagementCoordinator::currentSourceRefreshCancelled);
}

void ComicManagementCoordinator::copyAndImportComicsToCurrentFolder(const QList<QPair<QString, QString>> &comics)
{
    copyAndImportComics(comics, currentFolderProvider(), libraryPathProvider());
}

void ComicManagementCoordinator::moveAndImportComicsToCurrentFolder(const QList<QPair<QString, QString>> &comics)
{
    moveAndImportComics(comics, currentFolderProvider(), libraryPathProvider());
}

void ComicManagementCoordinator::copyAndImportComicsToFolder(const QList<QPair<QString, QString>> &comics, const QModelIndex &folder)
{
    const auto destinationFolder = foldersModelProxy->mapToSource(folder);
    if (destinationFolder.isValid())
        copyAndImportComics(comics, destinationFolder, libraryPathProvider());
}

void ComicManagementCoordinator::moveAndImportComicsToFolder(const QList<QPair<QString, QString>> &comics, const QModelIndex &folder)
{
    const auto destinationFolder = foldersModelProxy->mapToSource(folder);
    if (destinationFolder.isValid())
        moveAndImportComics(comics, destinationFolder, libraryPathProvider());
}

void ComicManagementCoordinator::addSelectedComicsToFavorites()
{
    comicsModel->addComicsToFavorites(comicsModel->supportedIndexes(selectionProvider(), ComicAction::AddToFavorites));
}

void ComicManagementCoordinator::addSelectedComicsToLabel(qulonglong labelId)
{
    comicsModel->addComicsToLabel(comicsModel->supportedIndexes(selectionProvider(), ComicAction::AddToList), labelId);
}

void ComicManagementCoordinator::openCurrentComic()
{
    if (!comicOpeningAllowedProvider())
        return;

    const auto currentComic = currentComicProvider();
    if (!comicsModel->supports(currentComic, ComicAction::Open))
        return;

    const auto comic = comicsModel->getComic(currentComic);
    if (!comic)
        return;

    const auto rowId = comicsModel->rowId(currentComic);
    const auto entry = std::get_if<ListItemId>(&rowId);
    startReader(*comic, comicsModel->getMode(), entry != nullptr && entry->listKind == ListKind::ReadingList ? entry->id : 0);
}

void ComicManagementCoordinator::openComic(const ComicDB &comic, ComicModel::Mode mode)
{
    startReader(comic, mode, 0);
}

void ComicManagementCoordinator::startReader(const ComicDB &comic, ComicModel::Mode mode, qulonglong entryId)
{
    const auto source = mode == ComicModel::ReadingList
            ? OpenComicSource::Source::ReadingList
            : OpenComicSource::Source::Folder;
    const auto libraryPath = libraryPathProvider();
    const auto thirdPartyReaderCommand = settings->value(THIRD_PARTY_READER_COMMAND, "").toString();

    if (thirdPartyReaderCommand.isEmpty()) {
        const auto yacreaderFound = YACReader::openComic(comic, libraryIdProvider(), libraryPath, OpenComicSource { source, comicsModel->getSourceId(), mode == ComicModel::ReadingList ? entryId : 0 });
        if (!yacreaderFound) {
#ifdef Q_OS_WIN
            QMessageBox::critical(window, tr("YACReader not found"), tr("YACReader not found. YACReader should be installed in the same folder as YACReaderLibrary."));
#else
            QMessageBox::critical(window, tr("YACReader not found"), tr("YACReader not found. There might be a problem with your YACReader installation."));
#endif
        }
        return;
    }

    if (!YACReader::openComicInThirdPartyApp(thirdPartyReaderCommand, QDir::cleanPath(libraryPath + comic.file.path)))
        QMessageBox::critical(window, tr("Error"), tr("Error opening comic with third party reader."));
}

void ComicManagementCoordinator::openContainingFolderOfCurrentComic()
{
    const auto currentComic = currentComicProvider();
    if (!comicsModel->supports(currentComic, ComicAction::OpenContainingFolder))
        return;

    const QFileInfo file(QDir::cleanPath(libraryPathProvider() + comicsModel->getComicPath(currentComic)));
#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    QDesktopServices::openUrl(QUrl("file:///" + file.absolutePath(), QUrl::TolerantMode));
#endif

#ifdef Q_OS_MACOS
    // `open -R` reveals and selects the file in Finder without sending an Apple
    // Event, so it doesn't trigger the macOS automation permission prompt.
    QStringList arguments;
    arguments << "-R";
    arguments << file.absoluteFilePath();
    QProcess::startDetached("open", arguments);
#endif

#ifdef Q_OS_WIN
    const auto cmdArgs = QString("/select,\"") + QDir::toNativeSeparators(file.absoluteFilePath()) + QStringLiteral("\"");
    ShellExecuteW(0, L"open", L"explorer.exe", reinterpret_cast<LPCWSTR>(cmdArgs.utf16()), 0, SW_NORMAL);
#endif
}

void ComicManagementCoordinator::showComicVineScraper()
{
    QSettings comicVineSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat); // TODO unificar la creación del fichero de config con el servidor
    comicVineSettings.beginGroup("ComicVine");

    if (!comicVineSettings.contains(COMIC_VINE_API_KEY)) {
        ApiKeyDialog dialog;
        dialog.exec();
    }

    if (!comicVineSettings.contains(COMIC_VINE_API_KEY))
        return;

    QList<ComicRecord> comics;
    for (const auto &index : comicsModel->supportedIndexes(selectionProvider(), ComicAction::SearchComicVine))
        comics.append(comicsModel->record(index));
    if (comics.isEmpty())
        return;

    comicVineDialog->databasePath = foldersModel->getDatabase();
    comicVineDialog->basePath = libraryPathProvider();
    comicVineDialog->setComics(comics);

    emit currentSourceRefreshStarted();
    comicVineDialog->show();
}

void ComicManagementCoordinator::copyAndImportComics(const QList<QPair<QString, QString>> &comics,
                                                     const QModelIndex &destinationFolder,
                                                     const QString &libraryPath)
{
    if (comics.isEmpty())
        return;

    const auto destinationPath = QDir::cleanPath(libraryPath + foldersModel->getFolderPath(destinationFolder));
    const auto destinationFolderId = destinationFolder.data(FolderModel::IdRole).toULongLong();
    QLOG_DEBUG() << "Copying comics to" << destinationPath;
    auto progressDialog = newProgressDialog(QCoreApplication::translate("LibraryWindow", "Copying comics..."), comics.size());
    auto comicFilesManager = new ComicFilesManager;
    comicFilesManager->copyComicsTo(comics, destinationPath, destinationFolderId);
    processComicFiles(comicFilesManager, progressDialog);
}

void ComicManagementCoordinator::moveAndImportComics(const QList<QPair<QString, QString>> &comics,
                                                     const QModelIndex &destinationFolder,
                                                     const QString &libraryPath)
{
    if (comics.isEmpty())
        return;

    const auto destinationPath = QDir::cleanPath(libraryPath + foldersModel->getFolderPath(destinationFolder));
    const auto destinationFolderId = destinationFolder.data(FolderModel::IdRole).toULongLong();
    QLOG_DEBUG() << "Moving comics to" << destinationPath;
    auto progressDialog = newProgressDialog(QCoreApplication::translate("LibraryWindow", "Moving comics..."), comics.size());
    auto comicFilesManager = new ComicFilesManager;
    comicFilesManager->moveComicsTo(comics, destinationPath, destinationFolderId);
    processComicFiles(comicFilesManager, progressDialog);
}

void ComicManagementCoordinator::showProperties()
{
    const auto indexList = comicsModel->supportedIndexes(selectionProvider(), ComicAction::EditMetadata);
    if (indexList.isEmpty())
        return;

    propertiesDialog->databasePath = foldersModel->getDatabase();
    propertiesDialog->basePath = libraryPathProvider();

    if (indexList.length() > 1) {
        QList<ComicRecord> records;
        for (const auto &index : indexList)
            records.append(comicsModel->record(index));
        propertiesDialog->setComics(records);
    } else {
        // sequential editing goes through the rows that can be edited, in view order
        QList<ComicRecord> records;
        int start = 0;
        for (int row = 0; row < comicsModel->rowCount(); row++) {
            const auto index = comicsModel->index(row, 0);
            if (!comicsModel->supports(index, ComicAction::EditMetadata))
                continue;
            if (row == indexList.constFirst().row())
                start = records.size();
            records.append(comicsModel->record(index));
        }
        if (!propertiesDialog->setComicsForSequentialEditing(start, records))
            return;
    }

    emit currentSourceRefreshStarted();
    propertiesDialog->show();
}

void ComicManagementCoordinator::setSelectedComicsRead()
{
    comicsModel->setComicsRead(comicsModel->supportedIndexes(selectionProvider(), ComicAction::SetReadState), YACReader::Read);
    emit currentComicViewUpdateRequested();
}

void ComicManagementCoordinator::setSelectedComicsUnread()
{
    comicsModel->setComicsRead(comicsModel->supportedIndexes(selectionProvider(), ComicAction::SetReadState), YACReader::Unread);
    emit currentComicViewUpdateRequested();
}

void ComicManagementCoordinator::setComicUnread(qulonglong libraryId, const ComicDB &comic)
{
    auto info = comic.info;
    info.read = false;
    info.currentPage = 1;
    info.hasBeenOpened = false;
    info.lastTimeOpened = QVariant();
    DBHelper::update(libraryId, info);
    emit rootContinueReadingReloadRequested();
}

void ComicManagementCoordinator::setSelectedComicsType(YACReader::FileType type)
{
    comicsModel->setComicsType(comicsModel->supportedIndexes(selectionProvider(), ComicAction::SetType), type);
}

void ComicManagementCoordinator::resetSelectedComicRatings()
{
    const auto indexList = comicsModel->supportedIndexes(selectionProvider(), ComicAction::SetReadState);
    comicsModel->startTransaction();
    for (const auto &index : indexList)
        comicsModel->resetComicRating(index);
    comicsModel->finishTransaction();
}

void ComicManagementCoordinator::assignNumbers()
{
    const auto selectedIds = selectedRowIds(ComicAction::AssignNumbers);
    if (selectedIds.isEmpty())
        return;

    const auto source = currentSource();
    const auto initialIndexes = indexesForRows(selectedIds, source);
    if (initialIndexes.isEmpty())
        return;

    int startingNumber = initialIndexes.constFirst().row() + 1;
    if (initialIndexes.count() > 1) {
        bool accepted;
        startingNumber = QInputDialog::getInt(window,
                                              QCoreApplication::translate("LibraryWindow", "Assign comics numbers"),
                                              QCoreApplication::translate("LibraryWindow", "Assign numbers starting in:"),
                                              startingNumber,
                                              0,
                                              2147483647,
                                              1,
                                              &accepted);
        if (!accepted)
            return;
    }

    const auto indexList = indexesForRows(selectedIds, source);
    if (indexList.isEmpty())
        return;

    emit comicNumbersAssigned(comicsModel->asignNumbers(indexList, startingNumber));
}

void ComicManagementCoordinator::deleteMetadataFromSelectedComics()
{
    const auto indexes = comicsModel->supportedIndexes(selectionProvider(), ComicAction::DeleteMetadata);
    if (indexes.isEmpty())
        return;

    comicsModel->deleteMetadata(indexes);
}

void ComicManagementCoordinator::deleteSelectedComics()
{
    const auto listIndex = currentListProvider();
    const auto rowIds = selectedRowIds(listIndex.isValid() ? ComicAction::RemoveFromList : ComicAction::ChangeFile);
    if (rowIds.isEmpty())
        return;

    const auto source = currentSource();
    if (listIndex.isValid()) {
        deleteComicsFromList(rowIds,
                             source,
                             listIndex.data(ReadingListModel::TypeListsRole).toInt(),
                             listIndex.data(ReadingListModel::IDRole).toULongLong());
    } else {
        deleteComicsFromDisk(rowIds, source);
    }
}

void ComicManagementCoordinator::saveSelectedCoversTo()
{
    const auto rowIds = selectedRowIds(ComicAction::SaveCover);
    if (rowIds.isEmpty())
        return;

    const auto source = currentSource();
    const auto destinationFolder = QFileDialog::getExistingDirectory(window,
                                                                     QCoreApplication::translate("LibraryWindow", "Save covers"),
                                                                     QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    if (destinationFolder.isEmpty())
        return;

    const auto indexList = indexesForRows(rowIds, source);
    for (const auto &comic : indexList) {
        QString origin = comic.data(ComicModel::CoverPathRole).toString().remove("file:///").remove("file:");
        auto name = comic.data(ComicModel::FileNameRole).toString();
        if (name.isEmpty())
            name = comic.data(ComicModel::LastKnownNameRole).toString();
        if (name.isEmpty())
            name = comic.data(ComicModel::ReadableTitle).toString();
        const auto destination = QDir(destinationFolder).filePath(name + ".jpg");

        QLOG_DEBUG() << "From : " << origin;
        QLOG_DEBUG() << "To : " << destination;

        QFile::copy(origin, destination);
    }
}

void ComicManagementCoordinator::setCustomCover(const ComicRowId &row, const QString &imagePath)
{
    if (!comicOpeningAllowedProvider())
        return;

    const auto index = comicsModel->indexOf(row);
    if (!comicsModel->supports(index, ComicAction::SetCustomCover))
        return;

    const QImage cover(imagePath);
    if (cover.isNull()) {
        QMessageBox::warning(window,
                             QCoreApplication::translate("LibraryWindow", "Invalid image"),
                             QCoreApplication::translate("LibraryWindow", "The selected file is not a valid image."));
        return;
    }

    const auto libraryPath = libraryPathProvider();
    auto record = comicsModel->record(index);
    if (const auto *placeholder = recordPlaceholder(record)) {
        if (!changePlaceholderCover(*placeholder, cover))
            QMessageBox::warning(window,
                                 QCoreApplication::translate("LibraryWindow", "Error saving cover"),
                                 QCoreApplication::translate("LibraryWindow", "There was an error saving the cover image."));
        return;
    }
    auto &info = *recordInfo(record);
    info.coverPage = QVariant();
    info.originalCoverSize = QStringLiteral("%1x%2").arg(cover.width()).arg(cover.height());
    info.coverSizeRatio = static_cast<double>(cover.width()) / cover.height();
    info.lastTimeCoverSet = QDateTime::currentSecsSinceEpoch();
    info.usesExternalCover = true;

    // the previous cover stays when any step fails
    StagedCover staged(YACReader::LibraryPaths::coverPath(libraryPath, info.hash));
    bool saved = staged.stage(cover);
    if (saved) {
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraryPath));
            db.transaction();
            saved = DBHelper::loadRecord(record, db) && DBHelper::updateComicInfoCover(info, db) && staged.install() && db.commit();
            if (saved) {
                staged.finish();
            } else {
                db.rollback();
                staged.restore();
            }
            connectionName = db.connectionName();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }

    if (!saved) {
        QMessageBox::warning(window,
                             QCoreApplication::translate("LibraryWindow", "Error saving cover"),
                             QCoreApplication::translate("LibraryWindow", "There was an error saving the cover image."));
        return;
    }

    comicsModel->notifyCoverChange(info.id);
}

// a placeholder's cover is always its own image
bool ComicManagementCoordinator::hasCustomCoverInSelection() const
{
    const auto indexes = comicsModel->supportedIndexes(selectionProvider(), ComicAction::SetCustomCover);
    const auto hasPlaceholderCover = std::any_of(indexes.cbegin(), indexes.cend(), [this](const QModelIndex &index) {
        const auto record = comicsModel->record(index);
        const auto *placeholder = recordPlaceholder(record);
        return placeholder != nullptr && placeholder->coverKey.has_value();
    });
    if (hasPlaceholderCover)
        return true;

    const auto comics = comicsModel->getComics(indexes);
    return std::any_of(comics.cbegin(), comics.cend(), [](const ComicDB &comic) {
        return comic.info.usesExternalCover.toBool();
    });
}

bool ComicManagementCoordinator::canMatchSelectionToLibraryComic() const
{
    const auto selection = selectionProvider();
    if (selection.size() == 1)
        return comicsModel->supports(selection.constFirst(), ComicAction::MatchToLibraryComic);
    return selection.size() > 1 && std::all_of(selection.cbegin(), selection.cend(), [this](const QModelIndex &index) { return comicsModel->listEntry(index).has_value(); });
}

// the comic is given to the entries of the missing comic in every list
void ComicManagementCoordinator::matchSelectedToLibraryComic()
{
    if (!comicOpeningAllowedProvider() || !canMatchSelectionToLibraryComic())
        return;

    const auto selection = selectionProvider();
    if (selection.size() > 1) {
        matchSelectedEntriesToLibraryComics(selection);
        return;
    }

    const auto index = selection.constFirst();
    const auto record = comicsModel->record(index);
    const auto entry = comicsModel->listEntry(index);
    if (!entry)
        return;
    MissingComic missing;
    if (const auto *placeholder = recordPlaceholder(record))
        missing = placeholder->id;
    else if (const auto *info = recordInfo(record))
        missing = info->id;
    const auto source = currentSource();
    const auto dataPath = YACReader::LibraryPaths::libraryDataPath(source.libraryPath);

    emit currentSourceRefreshStarted();
    CblMatchComicDialog dialog(recordMetadata(record), source.libraryPath, window);
    if (dialog.exec() != QDialog::Accepted || !dialog.chosenComic() || !isCurrentSource(source)) {
        emit currentSourceRefreshCancelled();
        return;
    }
    const auto comicInfoId = dialog.chosenComic()->id;

    std::optional<QStringList> otherLists;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(dataPath);
        otherLists = otherListsWithEntriesOf({ missing }, entry->id.listKind, { entry->listId }, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (otherLists && !otherLists->isEmpty()) {
        const auto answer = QMessageBox::question(window,
                                                  QCoreApplication::translate("LibraryWindow", "Match to library comic"),
                                                  QCoreApplication::translate("LibraryWindow", "This also gives the comic to entries in %n other list(s): %1", "", otherLists->size()).arg(otherLists->join(QStringLiteral(", "))),
                                                  QMessageBox::Apply | QMessageBox::Cancel,
                                                  QMessageBox::Apply);
        if (answer != QMessageBox::Apply) {
            emit currentSourceRefreshCancelled();
            return;
        }
    }

    bool matched = false;
    ComicMatchFiles files;
    if (otherLists) {
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(dataPath);
            db.transaction();
            matched = std::visit(overloaded {
                                         [&](PlaceholderId id) { return matchPlaceholderToComic(id, comicInfoId, dataPath, db, files); },
                                         [&](ComicInfoId id) { return matchDeletedComicToComic(id, comicInfoId, db, files); },
                                 },
                                 missing) &&
                    db.commit();
            if (!matched)
                db.rollback();
            connectionName = db.connectionName();
        }
        QSqlDatabase::removeDatabase(connectionName);
        finishComicMatchFiles(dataPath, files, matched);
    }

    if (!matched) {
        emit currentSourceRefreshCancelled();
        QMessageBox::warning(window,
                             QCoreApplication::translate("LibraryWindow", "Match to library comic"),
                             QCoreApplication::translate("LibraryWindow", "The comic could not be matched. Nothing was changed."));
        return;
    }
    emit currentSourceRefreshAccepted();
}

void ComicManagementCoordinator::matchSelectedEntriesToLibraryComics(const QModelIndexList &selection)
{
    QList<CblListEntry> entries;
    for (const auto &index : selection) {
        if (const auto entry = comicsModel->listEntry(index))
            entries.append(cblListEntry(comicsModel->record(index), *entry));
    }
    const auto source = currentSource();

    emit currentSourceRefreshStarted();
    CblImportDialog dialog(currentListProvider().data().toString(), entries, source.libraryPath, window);
    bool changed = false;
    connect(&dialog, &CblImportDialog::listEntriesChanged, this, [&changed] { changed = true; });
    dialog.exec();
    if (changed)
        emit currentSourceRefreshAccepted();
    else
        emit currentSourceRefreshCancelled();
}

// a new image gets a new file; the old file is removed after the commit, the new one on a failure.
// A null `cover` clears the cover of the placeholder.
bool ComicManagementCoordinator::changePlaceholderCover(const Placeholder &placeholder, const QImage &cover)
{
    const auto dataPath = YACReader::LibraryPaths::libraryDataPath(libraryPathProvider());
    std::optional<QString> newKey;
    if (!cover.isNull()) {
        newKey = savePlaceholderCover(dataPath, cover);
        if (!newKey)
            return false;
    }

    std::optional<QString> oldKey;
    bool saved = false;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(dataPath);
        db.transaction();
        const auto stored = DBHelper::loadRecord(ComicRecordCase::Placeholder { placeholder }, db);
        if (stored)
            oldKey = recordPlaceholder(*stored)->coverKey;
        saved = stored && DBHelper::updatePlaceholderCover(placeholder.id, newKey, db) && db.commit();
        if (!saved)
            db.rollback();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!saved) {
        if (newKey)
            DBHelper::removePlaceholderCovers(dataPath, { *newKey });
        return false;
    }
    if (oldKey)
        DBHelper::removePlaceholderCovers(dataPath, { *oldKey });
    comicsModel->notifyPlaceholderCoverChange(placeholder.id, newKey);
    return true;
}

// a deleted comic keeps its custom cover: the original cannot be extracted without the file
void ComicManagementCoordinator::resetSelectedCustomCovers()
{
    if (!comicOpeningAllowedProvider())
        return;

    const auto libraryPath = libraryPathProvider();
    const auto libraryId = libraryIdProvider();
    const auto indexes = comicsModel->supportedIndexes(selectionProvider(), ComicAction::SetCustomCover);
    QSet<PlaceholderId> clearedPlaceholders;
    for (const auto &index : indexes) {
        // a placeholder goes back to no cover, which shows the default image
        const auto record = comicsModel->record(index);
        if (const auto *placeholder = recordPlaceholder(record)) {
            if (placeholder->coverKey && !clearedPlaceholders.contains(placeholder->id)) {
                clearedPlaceholders.insert(placeholder->id);
                changePlaceholderCover(*placeholder, QImage());
            }
            continue;
        }

        auto loaded = comicsModel->getComic(index);
        if (!loaded || !loaded->info.usesExternalCover.toBool())
            continue;
        auto &comic = *loaded;

        YACReader::InitialComicInfoExtractor extractor(QDir::cleanPath(libraryPath + comic.file.path), QString(), 1);
        extractor.extract();
        if (!extractor.hasValidCover())
            continue;

        const auto cover = extractor.getCoverImage();
        const auto coverPath = YACReader::LibraryPaths::coverPath(libraryPath, comic.info.hash);
        if (!YACReader::saveCover(coverPath, cover))
            continue;

        const auto originalCoverSize = extractor.getOriginalCoverSize();
        comic.info.coverPage = QVariant();
        comic.info.originalCoverSize = QStringLiteral("%1x%2").arg(originalCoverSize.first).arg(originalCoverSize.second);
        comic.info.coverSizeRatio = static_cast<double>(originalCoverSize.first) / originalCoverSize.second;
        comic.info.lastTimeCoverSet = QDateTime::currentSecsSinceEpoch();
        comic.info.usesExternalCover = false;
        DBHelper::update(libraryId, comic.info);
        comicsModel->notifyCoverChange(comic.info.id);
    }
}

QProgressDialog *ComicManagementCoordinator::newProgressDialog(const QString &label, int maximum)
{
    auto progressDialog = new QProgressDialog(label, QStringLiteral("Cancel"), 0, maximum, window);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumWidth(350);
    progressDialog->show();
    return progressDialog;
}

void ComicManagementCoordinator::processComicFiles(ComicFilesManager *comicFilesManager, QProgressDialog *progressDialog)
{
    connect(comicFilesManager, &ComicFilesManager::progress, progressDialog, &QProgressDialog::setValue);

    auto thread = new QThread;
    comicFilesManager->moveToThread(thread);

    connect(progressDialog, &QProgressDialog::canceled, comicFilesManager, &ComicFilesManager::cancel, Qt::DirectConnection);
    connect(thread, &QThread::started, comicFilesManager, &ComicFilesManager::process);
    connect(comicFilesManager, &ComicFilesManager::success, this, &ComicManagementCoordinator::importRequested);
    connect(comicFilesManager, &ComicFilesManager::finished, thread, &QThread::quit);
    connect(comicFilesManager, &ComicFilesManager::finished, comicFilesManager, &QObject::deleteLater);
    connect(comicFilesManager, &ComicFilesManager::finished, progressDialog, &QWidget::close);
    connect(comicFilesManager, &ComicFilesManager::finished, progressDialog, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
}

QList<ComicRowId> ComicManagementCoordinator::selectedRowIds(ComicAction action) const
{
    QList<ComicRowId> rowIds;
    const auto selection = comicsModel->supportedIndexes(selectionProvider(), action);
    rowIds.reserve(selection.size());
    for (const auto &index : selection)
        rowIds.append(comicsModel->rowId(index));
    return rowIds;
}

ComicManagementCoordinator::SourceContext ComicManagementCoordinator::currentSource() const
{
    return { libraryPathProvider(), static_cast<int>(comicsModel->getMode()), comicsModel->getSourceId() };
}

QModelIndexList ComicManagementCoordinator::indexesForRows(const QList<ComicRowId> &rowIds, const SourceContext &source) const
{
    if (!isCurrentSource(source))
        return { };

    auto indexes = comicsModel->indexesOf(rowIds);
    if (std::any_of(indexes.cbegin(), indexes.cend(), [](const QModelIndex &index) { return !index.isValid(); }))
        return { };

    std::sort(indexes.begin(), indexes.end(), [](const QModelIndex &left, const QModelIndex &right) {
        return left.row() < right.row();
    });
    return indexes;
}

bool ComicManagementCoordinator::isCurrentSource(const SourceContext &source) const
{
    return QDir::cleanPath(foldersModel->getDatabase()) == QDir::cleanPath(YACReader::LibraryPaths::libraryDataPath(source.libraryPath)) && static_cast<int>(comicsModel->getMode()) == source.mode && comicsModel->getSourceId() == source.sourceId;
}

void ComicManagementCoordinator::deleteComicsFromDisk(const QList<ComicRowId> &rowIds, const SourceContext &source)
{
    const auto answer = QMessageBox::question(window,
                                              QCoreApplication::translate("LibraryWindow", "Delete comics"),
                                              QCoreApplication::translate("LibraryWindow", "All the selected comics will be deleted from your disk. Are you sure?"),
                                              QMessageBox::Yes,
                                              QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    auto indexList = indexesForRows(rowIds, source);
    auto comics = comicsModel->getComics(indexList);
    if (comics.isEmpty())
        return;

    QList<QString> paths;
    paths.reserve(comics.size());
    for (const auto &comic : std::as_const(comics)) {
        paths.append(source.libraryPath + comic.file.path);
        QLOG_TRACE() << comic.file.path;
        QLOG_TRACE() << comic.file.id.value();
        QLOG_TRACE() << comic.file.folderId.value();
    }

    auto remover = new ComicsRemover(indexList, paths, comics.constFirst().file.folderId.value());
    auto thread = new QThread(this);
    moveAndConnectRemoverToThread(remover, thread);

    comicDeletionFailed = false;
    comicsModel->startTransaction();

    connect(remover, &ComicsRemover::remove, comicsModel, &ComicModel::remove);
    connect(remover, &ComicsRemover::removeError, this, [this] { comicDeletionFailed = true; });
    connect(remover, &ComicsRemover::finished, comicsModel, &ComicModel::finishTransaction);
    connect(remover, &ComicsRemover::removedItemsFromFolder, foldersModel, &FolderModel::updateFolderChildrenInfo);
    connect(remover, &ComicsRemover::finished, this, &ComicManagementCoordinator::finishComicDeletion);

    thread->start();
}

void ComicManagementCoordinator::deleteComicsFromList(const QList<ComicRowId> &rowIds, const SourceContext &source, int listType, qulonglong listId)
{
    auto question = QCoreApplication::translate("LibraryWindow", "Comics will only be deleted from the current label/list. Are you sure?");
    if (listType == ReadingListModel::SpecialList) {
        switch (static_cast<ReadingListModel::TypeSpecialList>(listId)) {
        case ReadingListModel::TypeSpecialList::Reading:
            question = QCoreApplication::translate("LibraryWindow", "Remove the comics from Reading? Their reading progress will be reset.");
            break;
        case ReadingListModel::TypeSpecialList::Favorites:
            question = QCoreApplication::translate("LibraryWindow", "The comics will be removed from Favorites. Are you sure?");
            break;
        case ReadingListModel::TypeSpecialList::Recent:
            break;
        }
    }
    const auto answer = QMessageBox::question(window,
                                              QCoreApplication::translate("LibraryWindow", "Remove comics"),
                                              question,
                                              QMessageBox::Yes,
                                              QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    const auto currentList = currentListProvider();
    if (!currentList.isValid() || currentList.data(ReadingListModel::TypeListsRole).toInt() != listType || currentList.data(ReadingListModel::IDRole).toULongLong() != listId)
        return;

    const auto indexList = indexesForRows(rowIds, source);
    if (indexList.isEmpty())
        return;

    switch (static_cast<ReadingListModel::TypeList>(listType)) {
    case ReadingListModel::SpecialList:
        comicsModel->deleteComicsFromSpecialList(indexList, listId);
        break;
    case ReadingListModel::Label:
    case ReadingListModel::ReadingList:
        comicsModel->deleteComicsFromLists(indexList);
        break;
    case ReadingListModel::Separator:
        break;
    }
}

void ComicManagementCoordinator::finishComicDeletion()
{
    emit comicDeletionFinished();
    if (comicDeletionFailed) {
        QMessageBox::critical(window,
                              QCoreApplication::translate("LibraryWindow", "Unable to delete"),
                              QCoreApplication::translate("LibraryWindow", "There was an issue trying to delete the selected comics. Please, check for write permissions in the selected files or containing folder."));
    }
    comicDeletionFailed = false;
}
