#include "console_ui_library_creator.h"

#include "comic_info_repairer.h"
#include "data_base_management.h"
#include "library_creator.h"
#include "xml_info_library_scanner.h"
#include "yacreader_libraries.h"

#include <iostream>
#include <utility>

using namespace YACReader;

namespace {
void printLockRecoveryHint()
{
    std::cout << "Wait for active maintenance to finish, or close the application holding the lock. If maintenance was interrupted, check that no application is using this library on any computer, then retry with --remove-stale-lock." << std::endl;
}

bool recoverMaintenanceIfRequested(const QString &path, bool requested)
{
    if (!requested)
        return true;
    LibraryMaintenanceLock lock(path);
    QString error;
    if (!lock.tryLock(true)) {
        std::cout << "Recovery failed: " << lock.errorString().toStdString() << std::endl;
        return false;
    }
    if (!DataBaseManagement::recoverInterruptedRestore(path, &error, true)) {
        std::cout << "Recovery failed: " << error.toStdString() << std::endl;
        return false;
    }
    return true;
}
}

ConsoleUILibraryCreator::ConsoleUILibraryCreator(QSettings *settings, QObject *parent)
    : QObject(parent), numComicsProcessed(0), settings(settings)
{
}

bool ConsoleUILibraryCreator::createLibrary(const QString &name, const QString &path, bool recoverLock)
{
    operationFailed = false;
    QDir pathDir(path);
    if (!pathDir.exists()) {
        std::cout << "Directory not found." << std::endl;
        return false;
    }

    QEventLoop eventLoop;
    LibraryCreator worker(settings);
    auto libraryCreator = &worker;
    QString cleanPath = QDir::cleanPath(pathDir.absolutePath());

    YACReaderLibraries yacreaderLibraries;
    yacreaderLibraries.load();
    if (yacreaderLibraries.contains(name)) {
        std::cout << "A Library named \"" << name.toUtf8().constData() << "\" already exists in database." << std::endl;
        return false;
    }

    libraryCreator->createLibrary(cleanPath, LibraryPaths::libraryDataPath(cleanPath));
    if (QDir(LibraryPaths::libraryDataPath(cleanPath)).exists() && !recoverMaintenanceIfRequested(cleanPath, recoverLock))
        return false;

    connect(libraryCreator, &LibraryCreator::finished, this, &ConsoleUILibraryCreator::done);
    connect(libraryCreator, &LibraryCreator::comicAdded, this, &ConsoleUILibraryCreator::newComic);
    connect(libraryCreator, &LibraryCreator::failedCreatingDB, this, &ConsoleUILibraryCreator::manageCreatingError);
    connect(libraryCreator, &LibraryCreator::failedOpeningDB, this, &ConsoleUILibraryCreator::manageCreatingError);
    bool completed = false;
    connect(libraryCreator, &LibraryCreator::created, &eventLoop, [&] { completed = true; });
    connect(libraryCreator, &LibraryCreator::updated, &eventLoop, [&] { completed = true; });

    connect(libraryCreator, &LibraryCreator::finished, &eventLoop, &QEventLoop::quit);

    std::cout << "Processing comics";

    libraryCreator->start();
    eventLoop.exec();
    libraryCreator->wait();

    if (operationFailed || !completed) {
        if (libraryCreator->maintenanceFailure().error == QLockFile::LockFailedError)
            printLockRecoveryHint();
        return false;
    }

    yacreaderLibraries.addLibrary(name, cleanPath);
    if (!yacreaderLibraries.save()) {
        std::cout << "The library was created, but its registration could not be saved." << std::endl;
        return false;
    }
    return true;
}

bool ConsoleUILibraryCreator::updateLibrary(const QString &path, bool recoverLock)
{
    operationFailed = false;
    QDir pathDir(path);
    if (!pathDir.exists()) {
        std::cout << "Directory not found." << std::endl;
        return false;
    }

    QEventLoop eventLoop;
    LibraryCreator worker(settings);
    auto libraryCreator = &worker;
    QString cleanPath = QDir::cleanPath(pathDir.absolutePath());
    if (!recoverMaintenanceIfRequested(cleanPath, recoverLock))
        return false;

    libraryCreator->updateLibrary(cleanPath, LibraryPaths::libraryDataPath(cleanPath));

    connect(libraryCreator, &LibraryCreator::finished, this, &ConsoleUILibraryCreator::done);
    connect(libraryCreator, &LibraryCreator::comicAdded, this, &ConsoleUILibraryCreator::newComic);
    connect(libraryCreator, &LibraryCreator::failedOpeningDB, this, &ConsoleUILibraryCreator::manageUpdatingError);

    connect(libraryCreator, &LibraryCreator::finished, &eventLoop, &QEventLoop::quit);
    bool completed = false;
    connect(libraryCreator, &LibraryCreator::updated, &eventLoop, [&] { completed = true; });

    std::cout << "Processing comics";

    libraryCreator->start();
    eventLoop.exec();
    libraryCreator->wait();
    if (libraryCreator->maintenanceFailure().error == QLockFile::LockFailedError)
        printLockRecoveryHint();
    return !operationFailed && completed;
}

void ConsoleUILibraryCreator::addExistingLibrary(const QString &name, const QString &path)
{
    QDir pathDir(path);
    if (!pathDir.exists()) {
        std::cout << "Directory not found." << std::endl;
        return;
    }
    QString cleanPath = QDir::cleanPath(pathDir.absolutePath());

    if (!QDir(LibraryPaths::libraryDataPath(cleanPath)).exists()) {
        std::cout << "No data folder found in path: " << cleanPath.toStdString() << std::endl;
        return;
    }

    YACReaderLibraries yacreaderLibraries;
    yacreaderLibraries.load();
    if (yacreaderLibraries.contains(name)) {
        std::cout << "A Library named \"" << name.toUtf8().constData() << "\" already exists in the database." << std::endl;
        return;
    }
    yacreaderLibraries.addLibrary(name, cleanPath);
    yacreaderLibraries.save();

    std::cout << "Library added : " << name.toUtf8().constData() << " at " << cleanPath.toUtf8().constData() << std::endl;
}

void ConsoleUILibraryCreator::removeLibrary(const QString &name)
{
    // TODO add error handling
    YACReaderLibraries yacreaderLibraries;
    yacreaderLibraries.load();
    if (!yacreaderLibraries.contains(name)) {
        std::cout << "No Library named \"" << name.toUtf8().constData() << "\" in database." << std::endl;
        return;
    }
    yacreaderLibraries.remove(name);
    yacreaderLibraries.save();

    std::cout << "Library removed : " << name.toUtf8().constData() << std::endl;
}

bool ConsoleUILibraryCreator::rescanXMLInfoLibrary(const QString &path, bool recoverLock)
{
    operationFailed = false;
    QDir pathDir(path);
    if (!pathDir.exists()) {
        std::cout << "Directory not found." << std::endl;
        return false;
    }

    QEventLoop eventLoop;
    XMLInfoLibraryScanner worker;
    auto scanner = &worker;
    QString cleanPath = QDir::cleanPath(pathDir.absolutePath());
    if (!recoverMaintenanceIfRequested(cleanPath, recoverLock))
        return false;

    connect(scanner, &XMLInfoLibraryScanner::finished, this, &ConsoleUILibraryCreator::done);
    connect(scanner, &XMLInfoLibraryScanner::comicScanned, this, &ConsoleUILibraryCreator::newComic);
    connect(scanner, &XMLInfoLibraryScanner::failed, this, [this](const QString &error) {
        operationFailed = true;
        std::cout << "XML rescan failed: " << error.toStdString() << std::endl;
    });

    connect(scanner, &XMLInfoLibraryScanner::finished, &eventLoop, &QEventLoop::quit);

    std::cout << "Scanning comics";
    scanner->scanLibrary(cleanPath, LibraryPaths::libraryDataPath(cleanPath));

    eventLoop.exec();
    scanner->wait();
    if (scanner->maintenanceFailure().error == QLockFile::LockFailedError)
        printLockRecoveryHint();
    return !operationFailed && scanner->completedSuccessfully();
}

int ConsoleUILibraryCreator::repairLibrary(const QString &path, bool recoverLock)
{
    QDir pathDir(path);
    if (!pathDir.exists()) {
        std::cout << "Directory not found." << std::endl;
        return 1;
    }

    QEventLoop eventLoop;
    ComicInfoRepairer *repairer = new ComicInfoRepairer(settings);
    const auto cleanPath = QDir::cleanPath(pathDir.absolutePath());
    if (!recoverMaintenanceIfRequested(cleanPath, recoverLock)) {
        delete repairer;
        return 1;
    }

    connect(repairer, &ComicInfoRepairer::comicProcessed, this, &ConsoleUILibraryCreator::newComic);
    connect(repairer, &QThread::finished, &eventLoop, &QEventLoop::quit);

    auto runRepair = [&](bool removeStaleLock) {
        std::cout << "Repairing comics";
        repairer->repairLibrary(cleanPath, LibraryPaths::libraryDataPath(cleanPath), removeStaleLock);
        eventLoop.exec();
        return repairer->summary();
    };

    auto summary = runRepair(false);

    if (summary.lockedByAnotherProcess) {
        std::cout << std::endl
                  << summary.error.toStdString() << std::endl;
        printLockRecoveryHint();
        delete repairer;
        return 1;
    }

    if (!summary.error.isEmpty()) {
        std::cout << std::endl
                  << "Repair failed: " << summary.error.toStdString() << std::endl;
        delete repairer;
        return 1;
    }
    std::cout << std::endl
              << "Repaired: " << summary.repaired << std::endl
              << "Failed: " << summary.failed << std::endl
              << "Missing files: " << summary.missingFiles << std::endl;
    for (const auto &failedPath : std::as_const(summary.failedFilePaths)) {
        std::cout << "  " << failedPath.toStdString() << std::endl;
    }
    delete repairer;
    return 0;
}

void ConsoleUILibraryCreator::newComic(const QString & /*relativeComicPath*/, const QString & /*coverPath*/)
{
    numComicsProcessed++;
    std::cout << ".";
}

void ConsoleUILibraryCreator::manageCreatingError(const QString &error)
{
    operationFailed = true;
    std::cout << std::endl
              << "Error creating library! " << error.toUtf8().constData() << std::endl;
}

void ConsoleUILibraryCreator::manageUpdatingError(const QString &error)
{
    operationFailed = true;
    std::cout << std::endl
              << "Error updating library! " << error.toUtf8().constData() << std::endl;
}

void ConsoleUILibraryCreator::done()
{
    std::cout << (operationFailed ? "Failed." : "Done!") << std::endl;

    if (numComicsProcessed > 0)
        std::cout << "Number of comics processed = " << numComicsProcessed << std::endl;
}
