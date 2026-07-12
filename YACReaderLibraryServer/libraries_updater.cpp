#include "libraries_updater.h"

#include "data_base_management.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <utility>

LibrariesUpdater::LibrariesUpdater()
{
}

void LibrariesUpdater::updateIfNeeded()
{
    YACReaderLibraries libraries;

    libraries.load();

    for (const QString &name : libraries.getNames()) {
        QString libraryPath = libraries.getPath(name);
        QString recoveryError;
        if (!DataBaseManagement::recoverInterruptedRestore(libraryPath, &recoveryError)) {
            qWarning() << "Unable to recover interrupted restore for" << libraryPath << recoveryError;
            continue;
        }
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
