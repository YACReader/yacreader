#ifndef YACREADERSERVERDATAHELPER_H
#define YACREADERSERVERDATAHELPER_H

#include <QtCore>
#include "folder.h"
#include "comic_db.h"

class YACReaderServerDataHelper
{
public:
    static QString folderToYSFormat(const qulonglong libraryId, const Folder & folder);
    static QString comicToYSFormat(const qulonglong libraryId, const ComicDB & comic);

private:
    YACReaderServerDataHelper();

};

#endif // YACREADERSERVERDATAHELPER_H
