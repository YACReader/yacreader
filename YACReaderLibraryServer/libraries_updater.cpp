#include "libraries_updater.h"

#include "yacreader_libraries.h"
#include "data_base_management.h"

LibrariesUpdater::LibrariesUpdater()
{
}

void LibrariesUpdater::updateIfNeeded()
{
    YACReaderLibraries libraries;

    libraries.load();

    foreach (QString name, libraries.getNames()) {
        QString libraryPath = libraries.getPath(name);
        QString libraryDataPath = YACReader::LibraryPaths::libraryDataPath(libraryPath);
        QString databasePath = YACReader::LibraryPaths::libraryDatabasePath(libraryPath);

        QDir d;

        QString dbVersion;
        if (d.exists(libraryDataPath) && d.exists(databasePath) && (dbVersion = DataBaseManagement::checkValidDB(databasePath)) != "") {
            int comparation = DataBaseManagement::compareVersions(dbVersion, DB_VERSION);

            if (comparation < 0) {
                bool updated = DataBaseManagement::updateToCurrentVersion(libraryPath);
                if (!updated) {
                    // TODO log error
                }
            }
        }
    }
}
