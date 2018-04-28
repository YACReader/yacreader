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

    foreach(QString name, libraries.getNames()) {
        QString path = libraries.getPath(name) + "/.yacreaderlibrary";

        QDir d;

        QString dbVersion;
        if(d.exists(path) && d.exists(path+"/library.ydb") && (dbVersion = DataBaseManagement::checkValidDB(path+"/library.ydb")) != "")
        {
            int comparation = DataBaseManagement::compareVersions(dbVersion,VERSION);

            if(comparation < 0)
            {
                bool updated = DataBaseManagement::updateToCurrentVersion(path);
                if(!updated) {
                    //TODO log error
                }

            }
        }
    }
}
