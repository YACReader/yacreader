#include "console_ui_library_creator.h"

#include "comic_info_repairer.h"
#include "library_creator.h"
#include "xml_info_library_scanner.h"
#include "yacreader_libraries.h"

#include <iostream>

using namespace YACReader;

ConsoleUILibraryCreator::ConsoleUILibraryCreator(QSettings *settings, QObject *parent)
    : QObject(parent), numComicsProcessed(0), settings(settings)
{
}

void ConsoleUILibraryCreator::createLibrary(const QString &name, const QString &path)
{
    QDir pathDir(path);
    if (!pathDir.exists()) {
        std::cout << "Directory not found." << std::endl;
        return;
    }

    QEventLoop eventLoop;
    LibraryCreator *libraryCreator = new LibraryCreator(settings);
    QString cleanPath = QDir::cleanPath(pathDir.absolutePath());

    YACReaderLibraries yacreaderLibraries;
    yacreaderLibraries.load();
    if (yacreaderLibraries.contains(name)) {
        std::cout << "A Library named \"" << name.toUtf8().constData() << "\" already exists in database." << std::endl;
        delete libraryCreator;
        return;
    }

    libraryCreator->createLibrary(cleanPath, LibraryPaths::libraryDataPath(cleanPath));

    connect(libraryCreator, &LibraryCreator::finished, this, &ConsoleUILibraryCreator::done);
    connect(libraryCreator, &LibraryCreator::comicAdded, this, &ConsoleUILibraryCreator::newComic);
    connect(libraryCreator, &LibraryCreator::failedCreatingDB, this, &ConsoleUILibraryCreator::manageCreatingError);

    connect(libraryCreator, &LibraryCreator::finished, &eventLoop, &QEventLoop::quit);

    std::cout << "Processing comics";

    libraryCreator->start();
    eventLoop.exec();

    yacreaderLibraries.addLibrary(name, cleanPath);
    yacreaderLibraries.save();
}

void ConsoleUILibraryCreator::updateLibrary(const QString &path)
{
    QDir pathDir(path);
    if (!pathDir.exists()) {
        std::cout << "Directory not found." << std::endl;
        return;
    }

    QEventLoop eventLoop;
    LibraryCreator *libraryCreator = new LibraryCreator(settings);
    QString cleanPath = QDir::cleanPath(pathDir.absolutePath());

    libraryCreator->updateLibrary(cleanPath, LibraryPaths::libraryDataPath(cleanPath));

    connect(libraryCreator, &LibraryCreator::finished, this, &ConsoleUILibraryCreator::done);
    connect(libraryCreator, &LibraryCreator::comicAdded, this, &ConsoleUILibraryCreator::newComic);
    connect(libraryCreator, &LibraryCreator::failedOpeningDB, this, &ConsoleUILibraryCreator::manageUpdatingError);

    connect(libraryCreator, &LibraryCreator::finished, &eventLoop, &QEventLoop::quit);

    std::cout << "Processing comics";

    libraryCreator->start();
    eventLoop.exec();
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

void ConsoleUILibraryCreator::rescanXMLInfoLibrary(const QString &path)
{
    QDir pathDir(path);
    if (!pathDir.exists()) {
        std::cout << "Directory not found." << std::endl;
        return;
    }

    QEventLoop eventLoop;
    XMLInfoLibraryScanner *scanner = new XMLInfoLibraryScanner();
    QString cleanPath = QDir::cleanPath(pathDir.absolutePath());

    connect(scanner, &XMLInfoLibraryScanner::finished, this, &ConsoleUILibraryCreator::done);
    connect(scanner, &XMLInfoLibraryScanner::comicScanned, this, &ConsoleUILibraryCreator::newComic);

    connect(scanner, &XMLInfoLibraryScanner::finished, &eventLoop, &QEventLoop::quit);

    std::cout << "Scanning comics";
    scanner->scanLibrary(cleanPath, LibraryPaths::libraryDataPath(cleanPath));

    eventLoop.exec();
}

int ConsoleUILibraryCreator::repairLibrary(const QString &path)
{
    QDir pathDir(path);
    if (!pathDir.exists()) {
        std::cout << "Directory not found." << std::endl;
        return 1;
    }

    QEventLoop eventLoop;
    ComicInfoRepairer *repairer = new ComicInfoRepairer(settings);
    const auto cleanPath = QDir::cleanPath(pathDir.absolutePath());

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
        if (summary.lockHolderIsRunningLocally) {
            std::cout << std::endl
                      << "A repair of this library is already running (" << summary.lockHolderInfo.toStdString() << "). Wait for it to finish." << std::endl;
            delete repairer;
            return 1;
        }

        std::cout << std::endl;
        if (summary.lockHolderInfo.isEmpty()) {
            std::cout << "The library is locked by a repair that did not finish." << std::endl;
        } else {
            std::cout << "The library is locked by a repair started by " << summary.lockHolderInfo.toStdString() << "." << std::endl;
        }
        std::cout << "If you are sure that no other repair is running, the lock can be removed." << std::endl
                  << "Remove the lock and continue? [y/N] " << std::flush;

        std::string answer;
        std::getline(std::cin, answer);
        // piped input can keep the trailing carriage return on Windows
        const auto trimmedAnswer = QString::fromStdString(answer).trimmed().toLower();
        if (trimmedAnswer != "y" && trimmedAnswer != "yes") {
            delete repairer;
            return 1;
        }

        summary = runRepair(true);
        if (summary.lockedByAnotherProcess) {
            std::cout << std::endl
                      << "The library is still locked, another process took the lock." << std::endl;
            delete repairer;
            return 1;
        }
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
    for (const auto &failedPath : summary.failedFilePaths) {
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
    std::cout << std::endl
              << "Error creating library! " << error.toUtf8().constData() << std::endl;
}

void ConsoleUILibraryCreator::manageUpdatingError(const QString &error)
{
    std::cout << std::endl
              << "Error updating library! " << error.toUtf8().constData() << std::endl;
}

void ConsoleUILibraryCreator::done()
{
    std::cout << "Done!" << std::endl;

    if (numComicsProcessed > 0)
        std::cout << "Number of comics processed = " << numComicsProcessed << std::endl;
}
