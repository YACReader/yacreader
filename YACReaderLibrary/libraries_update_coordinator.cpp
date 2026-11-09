
#include "libraries_update_coordinator.h"

#include "QsLog.h"
#include "data_base_management.h"
#include "library_creator.h"
#include "xml_info_library_scanner.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

using namespace YACReader;

LibrariesUpdateCoordinator::~LibrariesUpdateCoordinator()
{
    stop();
    if (updateFuture.valid())
        updateFuture.wait();
}

QJsonArray LibrariesUpdateCoordinator::lockProblems() const
{
    QMutexLocker locker(&problemsMutex);
    QJsonArray list;
    for (const auto &library : libraries.getLibraries()) {
        if (!problems.contains(library.getPath()))
            continue;
        auto problem = problems.value(library.getPath());
        problem.insert("library_id", library.getLegacyId());
        list.append(problem);
    }
    return list;
}

void LibrariesUpdateCoordinator::clearLockProblem(const QString &path)
{
    QMutexLocker locker(&problemsMutex);
    problems.remove(path);
}

void LibrariesUpdateCoordinator::recordLockProblem(const QString &path, const QString &operation, const LibraryMaintenanceLockFailure &lockFailure)
{
    QJsonObject problem {
        { "operation", operation },
        { "state", lockFailure.ownerIsRunning ? "busy" : "stale" },
        { "owner", lockFailure.owner },
    };
    if (!lockFailure.ownerIsRunning)
        problem.insert("recovery_token", lockFailure.recoveryToken);
    QMutexLocker locker(&problemsMutex);
    problems.insert(path, problem);
}

void LibrariesUpdateCoordinator::recordFailure(const QString &path, const QString &operation, const QString &error, const LibraryMaintenanceLockFailure &lockFailure)
{
    QLOG_ERROR() << "Library" << operation << "failed:" << path << error;
    if (lockFailure.error == QLockFile::LockFailedError)
        recordLockProblem(path, operation, lockFailure);
}

LibrariesUpdateCoordinator::LibrariesUpdateCoordinator(QSettings *settings, YACReaderLibraries &libraries, const std::function<bool()> &canStartUpdateProvider, QObject *parent)
    : QObject(parent), libraries(libraries), canStartUpdateProvider(canStartUpdateProvider)
{
    libraries.load();

    this->settings = settings;

    timer.setInterval(1000 * 60);

    connect(&timer, &QTimer::timeout, this, &LibrariesUpdateCoordinator::checkUpdatePolicy);
}

void LibrariesUpdateCoordinator::init()
{
    timer.start();
    elapsedTimer.start();

    if (settings->value(UPDATE_LIBRARIES_AT_STARTUP, false).toBool()) {
        updateLibraries();
    }
}

void LibrariesUpdateCoordinator::checkUpdatePolicy()
{
    if (settings->value(UPDATE_LIBRARIES_PERIODICALLY, false).toBool()) {
        auto variant = settings->value(UPDATE_LIBRARIES_PERIODICALLY_INTERVAL, static_cast<typename std::underlying_type<YACReader::LibrariesUpdateInterval>::type>(YACReader::LibrariesUpdateInterval::Hours2));

        bool itIsDue = false;
        auto interval = static_cast<YACReader::LibrariesUpdateInterval>(variant.toInt());
        switch (interval) {
        case YACReader::LibrariesUpdateInterval::Minutes30:
            itIsDue = elapsedTimer.elapsed() >= 1000 * 60 * 30;
            break;
        case YACReader::LibrariesUpdateInterval::Hourly:
            itIsDue = elapsedTimer.elapsed() >= 1000 * 60 * 60;
            break;
        case YACReader::LibrariesUpdateInterval::Hours2:
            itIsDue = elapsedTimer.elapsed() >= 1000 * 60 * 60 * 2;
            break;
        case YACReader::LibrariesUpdateInterval::Hours4:
            itIsDue = elapsedTimer.elapsed() >= 1000 * 60 * 60 * 4;
            break;
        case YACReader::LibrariesUpdateInterval::Hours8:
            itIsDue = elapsedTimer.elapsed() >= 1000 * 60 * 60 * 8;
            break;
        case YACReader::LibrariesUpdateInterval::Hours12:
            itIsDue = elapsedTimer.elapsed() >= 1000 * 60 * 60 * 12;
            break;
        case YACReader::LibrariesUpdateInterval::Daily:
            itIsDue = elapsedTimer.elapsed() >= 1000 * 60 * 60 * 24;
            break;
        }

        if (itIsDue) {
            elapsedTimer.restart();
            updateLibraries();
            return;
        }
    }

    if (settings->value(UPDATE_LIBRARIES_AT_CERTAIN_TIME, false).toBool()) {
        QTime time = settings->value(UPDATE_LIBRARIES_AT_CERTAIN_TIME_TIME, "00:00").toTime();
        QTime currentTime = QTime::currentTime();

        if (currentTime.hour() == time.hour() && currentTime.minute() == time.minute()) {
            updateLibraries();
            return;
        }
    }
}

void LibrariesUpdateCoordinator::updateLibraries()
{
    requestLibrariesUpdate();
}

void LibrariesUpdateCoordinator::updateSingleLibrary(int id)
{
    requestSingleLibraryUpdate(id);
}

LibrariesUpdateCoordinator::UpdateRequestResult LibrariesUpdateCoordinator::requestLibrariesUpdate()
{
    if (isRunning()) {
        return UpdateRequestResult::AlreadyRunning;
    }

    if (!canStartUpdateProvider()) {
        return UpdateRequestResult::NotAllowed;
    }

    return startUpdate({ });
}

LibrariesUpdateCoordinator::UpdateRequestResult LibrariesUpdateCoordinator::requestSingleLibraryUpdate(int id)
{
    if (isRunning()) {
        return UpdateRequestResult::AlreadyRunning;
    }

    const QString path = libraries.getPath(id);
    if (path.isEmpty()) {
        return UpdateRequestResult::LibraryNotFound;
    }

    if (!canStartUpdateProvider()) {
        return UpdateRequestResult::NotAllowed;
    }

    return startUpdate({ path });
}

LibrariesUpdateCoordinator::UpdateRequestResult LibrariesUpdateCoordinator::requestSingleLibraryXmlRescan(int id)
{
    if (isRunning()) {
        return UpdateRequestResult::AlreadyRunning;
    }

    const QString path = libraries.getPath(id);
    if (path.isEmpty()) {
        return UpdateRequestResult::LibraryNotFound;
    }

    if (!canStartUpdateProvider()) {
        return UpdateRequestResult::NotAllowed;
    }

    return startXmlRescan(path);
}

bool LibrariesUpdateCoordinator::isRunning() const
{
    QMutexLocker locker(&futureMutex);
    return updateFuture.valid() && updateFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
}

LibrariesUpdateCoordinator::RecoveryResult LibrariesUpdateCoordinator::recoverLibraryLock(int id, const QString &recoveryToken, QString *error)
{
    if (isRunning())
        return RecoveryResult::AlreadyRunning;
    const auto path = libraries.getPath(id);
    if (path.isEmpty())
        return RecoveryResult::LibraryNotFound;
    if (!canStartUpdateProvider())
        return RecoveryResult::NotAllowed;

    LibraryMaintenanceLock lock(path);
    if (!lock.tryLock()) {
        if (lock.error() != QLockFile::LockFailedError)
            return RecoveryResult::NotRecoverable;
        if (lock.holderIsRunning() || lock.recoveryToken() != recoveryToken) {
            // the owner changed: show the current one, never the token that was rejected
            QString operation;
            {
                QMutexLocker locker(&problemsMutex);
                operation = problems.value(path).value("operation").toString("update");
            }
            recordLockProblem(path, operation, lock.failure());
            return RecoveryResult::NotRecoverable;
        }
        if (!lock.tryLock(true)) {
            if (error)
                *error = lock.errorString();
            return RecoveryResult::Failed;
        }
    }
    if (!DataBaseManagement::recoverInterruptedRestore(path, error, true))
        return RecoveryResult::Failed;
    QLOG_INFO() << "Library maintenance lock recovered:" << path;
    clearLockProblem(path);
    return RecoveryResult::Recovered;
}

LibrariesUpdateCoordinator::UpdateRequestResult LibrariesUpdateCoordinator::startUpdate(const QStringList &paths)
{
    QMutexLocker locker(&futureMutex);

    if (updateFuture.valid() && updateFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        return UpdateRequestResult::AlreadyRunning;
    }

    canceled = false;

    QStringList targets = paths;
    if (targets.isEmpty()) {
        const auto availableLibraries = libraries.getLibraries();
        for (const auto &library : availableLibraries) {
            targets.append(library.getPath());
        }
    }

    updateFuture = std::async(std::launch::async, [this, targets] {
        emit updateStarted();
        for (const auto &path : targets) {
            if (!canceled) {
                updateLibrary(path);
            }
        }
        emit updateEnded();
    });

    return UpdateRequestResult::Started;
}

LibrariesUpdateCoordinator::UpdateRequestResult LibrariesUpdateCoordinator::startXmlRescan(const QString &path)
{
    QMutexLocker locker(&futureMutex);

    if (updateFuture.valid() && updateFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        return UpdateRequestResult::AlreadyRunning;
    }

    canceled = false;
    updateFuture = std::async(std::launch::async, [this, path] {
        emit updateStarted();
        if (!canceled) {
            rescanLibraryXml(path);
        }
        emit updateEnded();
    });

    return UpdateRequestResult::Started;
}

void LibrariesUpdateCoordinator::updateLibrary(const QString &path)
{
    clearLockProblem(path);
    QDir pathDir(path);
    if (!pathDir.exists()) {
        QLOG_ERROR() << "Library update failed: folder not found:" << path;
        return;
    }

    QEventLoop eventLoop;
    auto libraryCreator = new LibraryCreator(settings);
    std::shared_ptr<LibraryCreator> sharedPtr(libraryCreator);

    QString cleanPath = QDir::cleanPath(pathDir.absolutePath());

    libraryCreator->updateLibrary(cleanPath, LibraryPaths::libraryDataPath(cleanPath));
    {
        QMutexLocker locker(&workersMutex);
        currentLibraryCreator = sharedPtr;
        if (canceled)
            libraryCreator->cancel();
    }
    QString error;
    bool completed = false;
    connect(libraryCreator, &LibraryCreator::failedOpeningDB, &eventLoop, [&](const QString &message) { error = message; });
    connect(libraryCreator, &LibraryCreator::updated, &eventLoop, [&] { completed = true; });

    connect(libraryCreator, &LibraryCreator::finished, &eventLoop, &QEventLoop::quit);

    libraryCreator->start();
    eventLoop.exec();
    libraryCreator->wait();
    {
        QMutexLocker locker(&workersMutex);
        currentLibraryCreator.reset();
    }
    if (!libraryCreator->wasCanceled() && !canceled && !completed)
        recordFailure(path, "update", error.isEmpty() ? QString("the update did not complete") : error, libraryCreator->maintenanceFailure());
}

void LibrariesUpdateCoordinator::rescanLibraryXml(const QString &path)
{
    clearLockProblem(path);
    QDir pathDir(path);
    if (!pathDir.exists()) {
        QLOG_ERROR() << "Library XML rescan failed: folder not found:" << path;
        return;
    }

    QEventLoop eventLoop;
    auto scanner = new XMLInfoLibraryScanner();
    std::shared_ptr<XMLInfoLibraryScanner> sharedPtr(scanner);

    const QString cleanPath = QDir::cleanPath(pathDir.absolutePath());
    connect(scanner, &XMLInfoLibraryScanner::finished, &eventLoop, &QEventLoop::quit);

    {
        QMutexLocker locker(&workersMutex);
        currentXmlInfoLibraryScanner = sharedPtr;
        scanner->scanLibrary(cleanPath, LibraryPaths::libraryDataPath(cleanPath));
        if (canceled)
            scanner->cancel();
    }
    eventLoop.exec();
    scanner->wait();
    {
        QMutexLocker locker(&workersMutex);
        currentXmlInfoLibraryScanner.reset();
    }
    if (!canceled && !scanner->wasCanceled() && !scanner->errorString().isEmpty())
        recordFailure(path, "rescan-xml", scanner->errorString(), scanner->maintenanceFailure());
}

void LibrariesUpdateCoordinator::stop()
{
    canceled = true;
    QMutexLocker locker(&workersMutex);

    if (auto libraryCreator = currentLibraryCreator.lock()) {
        libraryCreator->stop();
    }

    if (auto scanner = currentXmlInfoLibraryScanner.lock()) {
        scanner->stop();
    }
}

void LibrariesUpdateCoordinator::cancel()
{
    canceled = true;
    QMutexLocker locker(&workersMutex);

    if (auto libraryCreator = currentLibraryCreator.lock()) {
        libraryCreator->cancel();
    }

    if (auto scanner = currentXmlInfoLibraryScanner.lock()) {
        scanner->cancel();
    }
}
