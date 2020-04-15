#include "console_ui_library_creator.h"

#include <iostream>

#include "library_creator.h"
#include "yacreader_libraries.h"

ConsoleUILibraryCreator::ConsoleUILibraryCreator(QObject *parent)
    : QObject(parent), numComicsProcessed(0)
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
    LibraryCreator *libraryCreator = new LibraryCreator();
    QString cleanPath = QDir::cleanPath(pathDir.absolutePath());

    YACReaderLibraries yacreaderLibraries;
    yacreaderLibraries.load();
    if (yacreaderLibraries.contains(name)) {
        std::cout << "A Library named \"" << name.toUtf8().constData() << "\" already exists in database." << std::endl;
        delete libraryCreator;
        return;
    }

    libraryCreator->createLibrary(cleanPath, QDir::cleanPath(pathDir.absolutePath() + "/.yacreaderlibrary"));

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
    LibraryCreator *libraryCreator = new LibraryCreator();
    QString cleanPath = QDir::cleanPath(pathDir.absolutePath());

    libraryCreator->updateLibrary(cleanPath, QDir::cleanPath(pathDir.absolutePath() + "/.yacreaderlibrary"));

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

    if (!QDir(cleanPath + "/.yacreaderlibrary").exists()) {
        std::cout << "No library database found in directory." << std::endl;
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
    //TODO add error handling
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
