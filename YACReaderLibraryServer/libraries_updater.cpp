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
        QString root = libraries.getPath(name);
        QString libraryDataPath = YACReader::LibraryPaths::libraryDataPath(root);
        QString databasePath = YACReader::LibraryPaths::libraryDatabasePath(root);

        QDir d;

        QString dbVersion;
        if (d.exists(libraryDataPath) && d.exists(databasePath) && (dbVersion = DataBaseManagement::checkValidDB(databasePath)) != "") {
            int comparation = DataBaseManagement::compareVersions(dbVersion, DB_VERSION);

            if (comparation < 0) {
                bool updated = DataBaseManagement::updateToCurrentVersion(libraryDataPath);
                if (!updated) {
                    // TODO log error
                }
            }
        }
    }
}
