#include "console_ui_library_creator.h"

#include <iostream>

#include "library_creator.h"



ConsoleUILibraryCreator::ConsoleUILibraryCreator(QObject *parent) :
    QObject(parent), numComicsProcessed(0)
{

}

void ConsoleUILibraryCreator::createLibrary(const QString & name, const QString & path)
{
    QEventLoop eventLoop;
    LibraryCreator * libraryCreator = new LibraryCreator();

    libraryCreator->createLibrary(QDir::cleanPath(path),QDir::cleanPath(path)+"/.yacreaderlibrary");

    connect(libraryCreator, &LibraryCreator::finished, this, &ConsoleUILibraryCreator::done);
    connect(libraryCreator, &LibraryCreator::comicAdded, this, &ConsoleUILibraryCreator::newComic);
    connect(libraryCreator, &LibraryCreator::failedCreatingDB, this, &ConsoleUILibraryCreator::manageCreatingError);

    connect(libraryCreator, &LibraryCreator::finished, &eventLoop, &QEventLoop::quit);

    std::cout << "Processing comics";

    libraryCreator->start();
    eventLoop.exec();
}

void ConsoleUILibraryCreator::updateLibrary(const QString & path)
{
    //TODO
    //connect(libraryCreator,SIGNAL(failedUpdatingDB(QString)),this,SLOT(manageUpdatingError(QString)));
}

void ConsoleUILibraryCreator::newComic(const QString & relativeComicPath, const QString & coverPath)
{
    numComicsProcessed++;
    std::cout << ".";
}

void ConsoleUILibraryCreator::manageCreatingError(const QString & error)
{
    std::cout << std::endl << "Error creating library! " << error.toUtf8().constData();
}

void ConsoleUILibraryCreator::done()
{
   std::cout << "Done!" << std::endl;
   std::cout << "Number of comis processed = " << numComicsProcessed << std::endl;
}
