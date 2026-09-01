#ifndef ORGANIZE_FILES_WORKER_H
#define ORGANIZE_FILES_WORKER_H

#include "organize_files_plan.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include <functional>

namespace OrganizeFiles {

struct FileMove {
    qulonglong comicId = 0;
    QString source;
    QString destination;
};

struct FileFailure {
    QString path;
    QString reason;
};

// Renames when it can, falls back to copy+verify+delete for cross-volume moves.
// Undo needs the same fallback, or a file that crossed a volume cannot get back.
bool moveFile(const QString &source, const QString &destination, QString *reason);

QStringList removeEmptyDirectories(const QStringList &directories, const QString &boundary);

// Removes exactly the listed directories, deepest first, only while each is empty.
QStringList removeCreatedDirectories(const QStringList &directories);

// Orders the moves so a path is vacated before another file is moved onto it.
struct OrderedMove {
    FileMove move;
    // Part of a cycle: parked under a temporary name, finished at the end.
    bool viaTemporary = false;
};

QList<OrderedMove> orderMoves(const QList<FileMove> &moves);

class MoveWorker : public QObject
{
    Q_OBJECT
public:
    // Runs on the worker thread; must not touch the GUI.
    using Applier = std::function<bool(const QList<FileMove> &completed, const QStringList &removedDirectories, const QString &journalPath, QString *error)>;

    MoveWorker(const QString &libraryPath, const QString &base, const QList<FileMove> &moves, bool removeEmptyFolders);

    void setApplier(Applier applier);

    QList<FileMove> completedMoves() const;
    QList<FileFailure> failures() const;
    QStringList removedDirectories() const;
    QString journalPath() const;
    // Set when the run never started: the journal could not be written.
    QString startError() const;
    // Set when the journal broke mid-run and stopped it.
    QString recordError() const;
    // Files the run never reached because the journal broke.
    int notAttempted() const;

    bool databaseUpdated() const;
    QString databaseError() const;

public slots:
    void process();

signals:
    void progress(int done, int total, const QString &currentFile);
    void updatingLibrary();
    void finished();

private:
    QString libraryPath;
    QString base;
    QList<FileMove> moves;
    bool removeEmptyFolders;
    Applier applier;
    QList<FileMove> completed;
    QList<FileFailure> failed;
    QStringList removed;
    QString journal;
    QString journalError;
    QString journalBreak;
    bool applied = true;
    QString applyError;
};

// Runs the undo callback off the GUI thread, with progress and a failure list.
class UndoWorker : public QObject
{
    Q_OBJECT
public:
    using Runner = std::function<bool(QList<FileFailure> *failures, QString *error,
                                      const std::function<void(int done, int total, const QString &currentFile)> &fileProgress,
                                      const std::function<void()> &databasePhase)>;

    explicit UndoWorker(Runner runner);

    bool succeeded() const;
    QList<FileFailure> failures() const;
    QString errorString() const;

public slots:
    void process();

signals:
    void progress(int done, int total, const QString &currentFile);
    void updatingLibrary();
    void finished();

private:
    Runner runner;
    bool success = false;
    QList<FileFailure> failed;
    QString error;
};

class PlanWorker : public QObject
{
    Q_OBJECT
public:
    PlanWorker(const QList<ComicEntry> &entries, const QString &base, Mode mode);

public slots:
    void build(const QString &pattern, const QString &base, const OrganizeFiles::Overrides &overrides, quint64 generation);

signals:
    void built(const QList<OrganizeFiles::PlannedMove> &moves, quint64 generation);

private:
    PlanBuilder builder;
};

}

Q_DECLARE_METATYPE(OrganizeFiles::FileMove)

#endif // ORGANIZE_FILES_WORKER_H
