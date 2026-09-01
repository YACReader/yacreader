#include "organize_files_coordinator.h"

#include "QsLog.h"
#include "comic_model.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "folder_model.h"
#include "organize_files_dialog.h"
#include "organize_files_journal.h"
#include "yacreader_global.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMessageBox>
#include <QSet>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QWidget>

#include <algorithm>
#include <utility>

using OrganizeFiles::ComicEntry;
using OrganizeFiles::FileMove;

namespace {

void collectComicsRecursively(qulonglong libraryId, qulonglong folderId, QList<ComicDB> &out)
{
    const auto comics = DBHelper::getFolderComicsFromLibrary(libraryId, folderId);
    for (auto *item : comics) {
        if (auto *comic = static_cast<ComicDB *>(item))
            out.append(*comic);
    }
    qDeleteAll(comics);

    const auto subfolders = DBHelper::getFolderSubfoldersFromLibrary(libraryId, folderId);
    for (auto *item : subfolders)
        collectComicsRecursively(libraryId, item->id, out);
    qDeleteAll(subfolders);
}

QList<ComicEntry> buildEntries(const QList<ComicDB> &comics, const QString &libraryRoot)
{
    QList<ComicEntry> entries;
    entries.reserve(comics.size());

    for (const ComicDB &comic : comics) {
        ComicEntry entry;
        entry.comicId = comic.id;
        entry.sourceAbsolute = QDir::cleanPath(libraryRoot + comic.path);

        const QFileInfo info(entry.sourceAbsolute);
        entry.missing = !info.exists();
        entry.baseName = info.completeBaseName();
        entry.extension = info.suffix().isEmpty() ? QString() : QLatin1Char('.') + info.suffix();

        entry.publisher = comic.info.publisher.toString();
        entry.imprint = comic.info.imprint.toString();
        entry.series = comic.info.series.toString();
        entry.volume = comic.info.volume.toString();
        entry.number = comic.info.number.toString();
        entry.count = comic.info.count.toString();
        entry.title = comic.info.title.toString();
        OrganizeFiles::applyPublicationDate(entry, comic.info.date.toString());
        entry.storyArc = comic.info.storyArc.toString();
        entry.arcNumber = comic.info.arcNumber.toString();
        entry.writer = comic.info.writer.toString();

        entries.append(entry);
    }

    return entries;
}

QString libraryRelativePath(const QString &libraryRoot, const QString &absolutePath)
{
    return QLatin1Char('/') + QDir(libraryRoot).relativeFilePath(absolutePath);
}

}

OrganizeFilesCoordinator::OrganizeFilesCoordinator(QSettings *settings,
                                                   QWidget *window,
                                                   ComicModel *comicsModel,
                                                   FolderModel *foldersModel,
                                                   SelectionProvider selectionProvider,
                                                   CurrentFolderProvider currentFolderProvider,
                                                   CurrentLibraryProvider currentLibraryProvider)
    : QObject(window), settings(settings), window(window), comicsModel(comicsModel), foldersModel(foldersModel), selectionProvider(std::move(selectionProvider)), currentFolderProvider(std::move(currentFolderProvider)), currentLibraryProvider(std::move(currentLibraryProvider))
{
}

void OrganizeFilesCoordinator::renameCurrentFolder()
{
    runOnFolder(OrganizeFiles::Mode::Rename, currentFolderProvider());
}

void OrganizeFilesCoordinator::organizeCurrentFolder()
{
    runOnFolder(OrganizeFiles::Mode::Organize, currentFolderProvider());
}

void OrganizeFilesCoordinator::renameFolder(const QModelIndex &folderIndex)
{
    runOnFolder(OrganizeFiles::Mode::Rename, folderIndex);
}

void OrganizeFilesCoordinator::organizeFolder(const QModelIndex &folderIndex)
{
    runOnFolder(OrganizeFiles::Mode::Organize, folderIndex);
}

void OrganizeFilesCoordinator::renameSelectedComics()
{
    runOnSelectedComics(OrganizeFiles::Mode::Rename);
}

void OrganizeFilesCoordinator::organizeSelectedComics()
{
    runOnSelectedComics(OrganizeFiles::Mode::Organize);
}

void OrganizeFilesCoordinator::runOnFolder(OrganizeFiles::Mode mode, const QModelIndex &folderIndex)
{
    if (!folderIndex.isValid())
        return;

    const auto library = currentLibraryProvider();
    const auto folder = foldersModel->getFolder(folderIndex);
    const auto folderPath = QDir::cleanPath(library.rootPath + foldersModel->getFolderPath(folderIndex));

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QList<ComicDB> comics;
    collectComicsRecursively(library.id, folder.id, comics);
    QApplication::restoreOverrideCursor();

    if (comics.isEmpty()) {
        QMessageBox::information(window, tr("Organize files"), tr("This folder does not contain any comics."));
        return;
    }

    organizeComics(mode, comics, library.rootPath, folderPath);
}

void OrganizeFilesCoordinator::runOnSelectedComics(OrganizeFiles::Mode mode)
{
    const auto selection = selectionProvider();
    if (selection.isEmpty())
        return;

    const auto comics = comicsModel->getComics(selection);
    if (comics.isEmpty())
        return;

    const auto folderIndex = currentFolderProvider();
    const auto library = currentLibraryProvider();
    const auto folderPath = folderIndex.isValid()
            ? QDir::cleanPath(library.rootPath + foldersModel->getFolderPath(folderIndex))
            : QString();

    organizeComics(mode, comics, library.rootPath, folderPath);
}

void OrganizeFilesCoordinator::organizeComics(OrganizeFiles::Mode mode, const QList<ComicDB> &comics, const QString &libraryRoot, const QString &folderPath)
{
    const QString cleanLibraryRoot = QDir::cleanPath(libraryRoot);

    LibraryMaintenanceLock maintenanceLock(cleanLibraryRoot);
    if (!maintenanceLock.tryLock()) {
        QMessageBox::warning(window, tr("Organize files"),
                             tr("This library is busy: %1").arg(maintenanceLock.errorString()));
        return;
    }

    OrganizeFilesDialog::Context context;
    context.mode = mode;
    context.libraryPath = cleanLibraryRoot;
    const QString cleanFolderPath = folderPath.isEmpty() ? QString() : QDir::cleanPath(folderPath);
    context.folderPath = cleanFolderPath == cleanLibraryRoot ? QString() : cleanFolderPath;
    context.entries = buildEntries(comics, cleanLibraryRoot);

    OrganizeFilesDialog dialog(context, settings, window);
    dialog.setApplier([this, cleanLibraryRoot](const QList<FileMove> &moves, const QStringList &removedDirectories, const QString &journalPath, QString *error) {
        return applyToDatabase(moves, removedDirectories, cleanLibraryRoot, journalPath, { }, { }, error);
    });
    dialog.setUndoer([this, cleanLibraryRoot](const QString &journalPath, QList<OrganizeFiles::FileFailure> *failures, QString *error,
                                              const std::function<void(int, int, const QString &)> &fileProgress,
                                              const std::function<void()> &databasePhase) {
        return undo(journalPath, cleanLibraryRoot, failures, error, fileProgress, databasePhase);
    });

    dialog.exec();

    if (dialog.libraryChanged())
        emit libraryContentChanged();
}

bool OrganizeFilesCoordinator::applyToDatabase(const QList<FileMove> &moves,
                                               const QStringList &removedDirectories,
                                               const QString &libraryRoot,
                                               const QString &journalPath,
                                               const QList<QVariantMap> &foldersToRestore,
                                               const QList<qulonglong> &createdFolderIdsToRemove,
                                               QString *error)
{
    bool success = true;
    QString connectionName;
    QList<QVariantMap> removedFolders;
    QList<qulonglong> createdFolders;

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraryRoot));
        if (!db.isOpen()) {
            *error = tr("the library database could not be opened");
            return false;
        }

        connectionName = db.connectionName();

        if (!db.transaction()) {
            *error = tr("the library database could not be locked for writing");
            db = QSqlDatabase();
            QSqlDatabase::removeDatabase(connectionName);
            return false;
        }

        // Restored before anything is repointed, so ensureFolderPath() finds the
        // original rows instead of creating new ids (covers are keyed by id).
        if (!foldersToRestore.isEmpty() && !DBHelper::restoreFolderRows(foldersToRestore, db)) {
            *error = tr("a folder entry could not be restored");
            success = false;
        }

        if (success) {
            for (const auto &move : moves) {
                const QString relativePath = libraryRelativePath(libraryRoot, move.destination);
                const QString relativeDirectory = relativePath.left(relativePath.lastIndexOf(QLatin1Char('/')));
                const auto parentId = DBHelper::ensureFolderPath(relativeDirectory, db, &createdFolders);

                if (!DBHelper::moveComic(move.comicId, parentId, QFileInfo(move.destination).fileName(), relativePath, db)) {
                    *error = tr("a comic entry could not be updated");
                    success = false;
                    break;
                }
            }
        }

        if (success) {
            QStringList removedPaths;
            for (const auto &directory : removedDirectories)
                removedPaths << libraryRelativePath(libraryRoot, directory);

            DBHelper::removeEmptyFolderPaths(removedPaths, db, &removedFolders);
            // Undo: drop the rows the run created that are empty again.
            DBHelper::removeEmptyFolderRows(createdFolderIdsToRemove, db);
            DBHelper::syncFolderAddedFromContents(createdFolders, db);
            DBHelper::updateChildrenInfo(db);

            if (!db.commit()) {
                *error = tr("the library database could not be saved: %1").arg(db.lastError().text());
                db.rollback();
                removedFolders.clear();
                createdFolders.clear();
                success = false;
            }
        } else {
            db.rollback();
        }

        db = QSqlDatabase();
    }

    QSqlDatabase::removeDatabase(connectionName);

    // Written only after the transaction is on disk, so the record never claims a
    // folder was deleted that is still there.
    if (success && (!removedFolders.isEmpty() || !createdFolders.isEmpty()) && !journalPath.isEmpty()) {
        OrganizeFiles::Journal journal(libraryRoot);
        if (journal.reopen(journalPath)) {
            for (const auto &row : std::as_const(removedFolders))
                journal.appendRemovedFolder(row);
            for (const auto id : std::as_const(createdFolders))
                journal.appendCreatedFolder(id);
            journal.finish();
        } else {
            // Nothing to roll back; without this record an undo recreates the
            // deleted folders with new ids and their custom covers are lost.
            QLOG_ERROR() << "organize: could not reopen the journal" << journalPath
                         << "to record the folder rows:" << journal.errorString();
        }
    }

    return success;
}

bool OrganizeFilesCoordinator::undo(const QString &journalPath,
                                    const QString &libraryRoot,
                                    QList<OrganizeFiles::FileFailure> *failures,
                                    QString *error,
                                    const std::function<void(int done, int total, const QString &currentFile)> &fileProgress,
                                    const std::function<void()> &databasePhase)
{
    OrganizeFiles::JournalData data;
    if (!OrganizeFiles::Journal::read(libraryRoot, journalPath, &data)) {
        *error = tr("the record of the last organize run could not be read");
        return false;
    }

    QList<FileMove> restored;

    const int total = data.moves.size();
    int done = 0;

    for (int i = data.moves.size() - 1; i >= 0; --i) {
        const auto &journalMove = data.moves.at(i);

        FileMove move;
        move.comicId = journalMove.comicId;
        move.source = OrganizeFiles::absoluteFromRelative(libraryRoot, journalMove.to);
        move.destination = OrganizeFiles::absoluteFromRelative(libraryRoot, journalMove.from);

        fileProgress(++done, total, QDir(libraryRoot).relativeFilePath(move.destination));

        // An earlier attempt already put this one back. Repoint the row again
        // anyway, because that attempt may have failed after the file had moved.
        const bool alreadyBack = !QFileInfo::exists(move.source) && QFileInfo::exists(move.destination);

        if (!alreadyBack) {
            if (!QDir().mkpath(QFileInfo(move.destination).absolutePath())) {
                failures->append({ move.source, tr("the folder %1 could not be created").arg(QDir::toNativeSeparators(QFileInfo(move.destination).absolutePath())) });
                continue;
            }

            QString reason;
            if (!OrganizeFiles::moveFile(move.source, move.destination, &reason)) {
                failures->append({ move.source, reason });
                continue;
            }
        }

        restored.append(move);
    }

    // A cycle in the plan was broken with a temporary name, so the journal holds two
    // steps for one comic. Only the last step undone carries the original path.
    QHash<qulonglong, int> lastStepForComic;
    for (int i = 0; i < restored.size(); ++i)
        lastStepForComic.insert(restored.at(i).comicId, i);

    QList<FileMove> collapsed;
    collapsed.reserve(restored.size());
    for (int i = 0; i < restored.size(); ++i) {
        if (lastStepForComic.value(restored.at(i).comicId) == i)
            collapsed.append(restored.at(i));
    }

    // Only the directories the run created; one it merely filled is not the undo's
    // to delete, even when the undo leaves it empty.
    QStringList createdDirectories;
    for (const auto &relative : std::as_const(data.createdDirectories))
        createdDirectories << OrganizeFiles::absoluteFromRelative(libraryRoot, relative);

    const auto removedDirectories = OrganizeFiles::removeCreatedDirectories(createdDirectories);

    // Recorded parents first; reversed, a created branch deletes bottom-up.
    QList<qulonglong> createdFolderIds = data.createdFolders;
    std::reverse(createdFolderIds.begin(), createdFolderIds.end());

    if (!collapsed.isEmpty() || !data.removedFolders.isEmpty() || !createdFolderIds.isEmpty()) {
        databasePhase();
        if (!applyToDatabase(collapsed, removedDirectories, libraryRoot, QString(), data.removedFolders, createdFolderIds, error))
            return false;
    }

    // The journal is deleted only when every file is back; otherwise the user
    // keeps a way to try again.
    if (!failures->isEmpty()) {
        *error = tr("%n file(s) could not be moved back", "", failures->size());
        return false;
    }

    QFile::remove(journalPath);

    return true;
}
