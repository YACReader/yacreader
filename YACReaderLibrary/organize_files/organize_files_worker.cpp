#include "organize_files_worker.h"

#include "organize_files_journal.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QPair>
#include <QSet>

#include <algorithm>
#include <utility>

namespace {

QString translated(const char *text)
{
    return QCoreApplication::translate("OrganizeFiles", text);
}

QString temporaryNameFor(const QString &source)
{
    QString candidate = source + QStringLiteral(".yacreader-organize");
    int counter = 1;
    while (QFileInfo::exists(candidate))
        candidate = source + QStringLiteral(".yacreader-organize-") + QString::number(counter++);

    return candidate;
}

bool renameThroughTemporary(const QString &source, const QString &destination, QString *reason)
{
    const QString temporary = temporaryNameFor(source);

    QFile sourceFile(source);
    if (!sourceFile.rename(temporary)) {
        *reason = sourceFile.errorString();
        return false;
    }

    QFile temporaryFile(temporary);
    if (!temporaryFile.rename(destination)) {
        *reason = temporaryFile.errorString();
        // Put it back, so a failure leaves nothing behind under a name the library
        // does not know about.
        temporaryFile.rename(source);
        return false;
    }

    return true;
}

}

namespace OrganizeFiles {

bool moveFile(const QString &source, const QString &destination, QString *reason)
{
    if (source == destination)
        return true;

    // A rename that only changes the capitalisation of the name has a destination
    // that "already exists" on Windows and macOS, so it has to go around.
    if (source.compare(destination, Qt::CaseInsensitive) == 0)
        return renameThroughTemporary(source, destination, reason);

    QFile sourceFile(source);
    if (sourceFile.rename(destination))
        return true;

    const QString renameError = sourceFile.errorString();

    // Only a cross-volume rename is worth a copy. When the destination is taken,
    // the copy fails for the same reason and the rename error is the useful one.
    if (QFileInfo::exists(destination)) {
        *reason = renameError;
        return false;
    }

    if (!QFile::copy(source, destination)) {
        QFile copyTarget(destination);
        *reason = translated("%1 (copy also failed: %2)").arg(renameError, copyTarget.errorString());
        return false;
    }

    if (QFileInfo(destination).size() != QFileInfo(source).size()) {
        QFile::remove(destination);
        *reason = translated("the copy did not have the same size as the original");
        return false;
    }

    if (!QFile::remove(source)) {
        QFile::remove(destination);
        *reason = translated("the original could not be deleted after it was copied");
        return false;
    }

    return true;
}

QList<OrderedMove> orderMoves(const QList<FileMove> &moves)
{
    // Sources and destinations are unique, so every move has at most one blocker
    // and the graph is a set of chains and simple cycles.
    QHash<QString, int> ownerOfSource;
    for (int i = 0; i < moves.size(); ++i)
        ownerOfSource.insert(pathKey(moves.at(i).source), i);

    QList<int> blocker(moves.size(), -1);
    QList<int> blocked(moves.size(), -1);

    for (int i = 0; i < moves.size(); ++i) {
        const int owner = ownerOfSource.value(pathKey(moves.at(i).destination), -1);
        if (owner < 0 || owner == i)
            continue;

        blocker[i] = owner;
        blocked[owner] = i;
    }

    QList<OrderedMove> ordered;
    ordered.reserve(moves.size());

    QList<bool> emitted(moves.size(), false);

    const auto emitChainFrom = [&](int start) {
        for (int i = start; i >= 0 && !emitted.at(i); i = blocked.at(i)) {
            emitted[i] = true;
            ordered.append({ moves.at(i), false });
        }
    };

    // Chains first: an unblocked move goes straight away, its waiters follow.
    for (int i = 0; i < moves.size(); ++i) {
        if (blocker.at(i) < 0)
            emitChainFrom(i);
    }

    // Whatever is left is a cycle. Parking one member under a temporary name first
    // frees its source for the rest of the ring.
    for (int i = 0; i < moves.size(); ++i) {
        if (emitted.at(i))
            continue;

        emitted[i] = true;
        ordered.append({ moves.at(i), true });

        emitChainFrom(blocked.at(i));
    }

    return ordered;
}

QStringList removeEmptyDirectories(const QStringList &directories, const QString &boundary)
{
    const QString boundaryKey = pathKey(QDir::cleanPath(boundary));

    QStringList ordered = directories;
    ordered.removeDuplicates();
    std::sort(ordered.begin(), ordered.end(), [](const QString &a, const QString &b) {
        return a.count(QLatin1Char('/')) > b.count(QLatin1Char('/'));
    });

    QStringList removed;

    for (const QString &candidate : std::as_const(ordered)) {
        QString directory = QDir::cleanPath(candidate);

        // pathKey: base and directory can arrive with different capitalisation on
        // Windows, and a plain prefix test would then remove nothing at all.
        while (pathKey(directory).startsWith(boundaryKey + QLatin1Char('/'))) {
            // Hidden entries count: a directory holding only a desktop.ini is not empty.
            if (!QDir(directory).isEmpty(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot))
                break;

            const QString parent = QFileInfo(directory).absolutePath();
            if (!QDir().rmdir(directory))
                break;

            removed.append(directory);
            directory = parent;
        }
    }

    return removed;
}

QStringList removeCreatedDirectories(const QStringList &directories)
{
    QStringList ordered = directories;
    ordered.removeDuplicates();
    // Deepest first, so a parent is already empty by the time its turn comes.
    std::sort(ordered.begin(), ordered.end(), [](const QString &a, const QString &b) {
        return a.count(QLatin1Char('/')) > b.count(QLatin1Char('/'));
    });

    QStringList removed;

    for (const QString &directory : std::as_const(ordered)) {
        if (!QDir(directory).exists())
            continue;

        if (!QDir(directory).isEmpty(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot))
            continue;

        if (QDir().rmdir(directory))
            removed.append(directory);
    }

    return removed;
}

MoveWorker::MoveWorker(const QString &libraryPath, const QString &base, const QList<FileMove> &moves, bool removeEmptyFolders)
    : libraryPath(QDir::cleanPath(libraryPath)), base(QDir::cleanPath(base)), moves(moves), removeEmptyFolders(removeEmptyFolders)
{
}

void MoveWorker::setApplier(Applier applier)
{
    this->applier = std::move(applier);
}

QList<FileMove> MoveWorker::completedMoves() const
{
    return completed;
}

QList<FileFailure> MoveWorker::failures() const
{
    return failed;
}

QStringList MoveWorker::removedDirectories() const
{
    return removed;
}

QString MoveWorker::journalPath() const
{
    return journal;
}

QString MoveWorker::startError() const
{
    return journalError;
}

QString MoveWorker::recordError() const
{
    return journalBreak;
}

int MoveWorker::notAttempted() const
{
    return moves.size() - completed.size() - failed.size();
}

bool MoveWorker::databaseUpdated() const
{
    return applied;
}

QString MoveWorker::databaseError() const
{
    return applyError;
}

void MoveWorker::process()
{
    Journal journalFile(libraryPath);

    // A run with no record cannot be undone, so it must not start.
    if (!journalFile.begin(base)) {
        journalError = journalFile.errorString();
        emit finished();
        return;
    }

    journal = journalFile.filePath();

    const auto ordered = orderMoves(moves);
    const int total = ordered.size();
    int done = 0;

    QHash<int, QPair<FileMove, QString>> deferred;
    QSet<QString> createdDirectories;

    for (int i = 0; i < ordered.size(); ++i) {
        const auto &step = ordered.at(i);
        const auto &move = step.move;

        const QString target = step.viaTemporary ? temporaryNameFor(move.source) : move.destination;
        const QString targetDirectory = QFileInfo(target).absolutePath();

        // Noted before mkpath; afterwards there is no way to tell what the run made
        // from what was already there.
        QStringList aboutToCreate;
        for (QString level = targetDirectory;
             !level.isEmpty() && !QFileInfo::exists(level) && pathKey(level).startsWith(pathKey(base) + QLatin1Char('/'));
             level = QFileInfo(level).absolutePath()) {
            aboutToCreate.prepend(level);
        }

        if (!QDir().mkpath(targetDirectory)) {
            failed.append({ move.source, translated("The destination folder could not be created.") });
        } else {
            for (const QString &level : std::as_const(aboutToCreate)) {
                if (!createdDirectories.contains(level)) {
                    createdDirectories.insert(level);
                    journalFile.appendCreatedDirectory(level);
                }
            }

            // Stop before the move, not after it: a file moved with no record
            // could never come back.
            if (!journalFile.healthy()) {
                journalBreak = journalFile.errorString();
                break;
            }

            QString reason;
            if (moveFile(move.source, target, &reason)) {
                journalFile.appendMove(move.comicId, move.source, target);

                if (step.viaTemporary)
                    deferred.insert(i, { move, target });
                else
                    completed.append(move);
            } else {
                failed.append({ move.source, reason });
            }
        }

        if (!journalFile.healthy()) {
            journalBreak = journalFile.errorString();
            break;
        }

        emit progress(++done, total, QDir(base).relativeFilePath(move.destination));
    }

    // The parked cycle members reach their destinations. Runs even after a journal
    // break: a file must not survive the run under a temporary name.
    for (auto it = deferred.constBegin(); it != deferred.constEnd(); ++it) {
        const auto &move = it.value().first;
        const QString &temporary = it.value().second;

        QString reason;
        if (moveFile(temporary, move.destination, &reason)) {
            journalFile.appendMove(move.comicId, temporary, move.destination);
            completed.append(move);
        } else {
            // Put it back: at its source the file reads as already restored during
            // an undo; at the temporary name it would be lost to the library.
            QString backReason;
            if (moveFile(temporary, move.source, &backReason))
                failed.append({ move.source, reason });
            else
                failed.append({ move.source, translated("%1 (the file was left at %2)").arg(reason, QDir::toNativeSeparators(temporary)) });
        }
    }

    if (removeEmptyFolders) {
        QStringList sourceDirectories;
        for (const auto &move : std::as_const(completed))
            sourceDirectories << QFileInfo(move.source).absolutePath();

        removed = removeEmptyDirectories(sourceDirectories, base);
    }

    for (const QString &directory : std::as_const(removed))
        journalFile.appendRemovedDirectory(directory);

    // A directory created for a move that then failed is empty and unknown to the
    // database; only the empty ones are deleted, so used directories are untouched.
    removeCreatedDirectories(createdDirectories.values());

    // The database work reopens this file to append the folder rows it changes.
    journalFile.finish();
    Journal::prune(libraryPath, 10);

    if (applier && !completed.isEmpty()) {
        emit updatingLibrary();
        applied = applier(completed, removed, journal, &applyError);
    }

    emit finished();
}

UndoWorker::UndoWorker(Runner runner)
    : runner(std::move(runner))
{
}

bool UndoWorker::succeeded() const
{
    return success;
}

QList<FileFailure> UndoWorker::failures() const
{
    return failed;
}

QString UndoWorker::errorString() const
{
    return error;
}

void UndoWorker::process()
{
    success = runner(
            &failed, &error,
            [this](int done, int total, const QString &currentFile) { emit progress(done, total, currentFile); },
            [this] { emit updatingLibrary(); });

    emit finished();
}

PlanWorker::PlanWorker(const QList<ComicEntry> &entries, const QString &base, Mode mode)
    : builder(entries, base, mode)
{
}

void PlanWorker::build(const QString &pattern, const QString &base, const OrganizeFiles::Overrides &overrides, quint64 generation)
{
    builder.setBase(base);
    emit built(builder.build(pattern, overrides), generation);
}

}
