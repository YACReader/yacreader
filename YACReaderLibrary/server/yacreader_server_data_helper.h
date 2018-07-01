#ifndef YACREADERSERVERDATAHELPER_H
#define YACREADERSERVERDATAHELPER_H

#include <QtCore>
#include "folder.h"
#include "comic_db.h"
#include "reading_list.h"

class YACReaderServerDataHelper
{
public:
    static QString folderToYSFormat(const qulonglong libraryId, const Folder & folder);
    static QString comicToYSFormat(const qulonglong libraryId, const ComicDB & comic);

    static QJsonObject folderToJSON(const qulonglong libraryId, const Folder & folder);    
    static QJsonObject comicToJSON(const qulonglong libraryId, const ComicDB & comic);
    static QJsonObject fullComicToJSON(const qulonglong libraryId, const ComicDB & comic);
    static QJsonObject readingListToJSON(const qulonglong libraryId, const ReadingList & readingList);
    static QJsonObject labelToJSON(const qulonglong libraryId, const Label & label);


private:
    YACReaderServerDataHelper();

};

#endif // YACREADERSERVERDATAHELPER_H
