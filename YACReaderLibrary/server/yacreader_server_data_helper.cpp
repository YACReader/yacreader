#include "yacreader_server_data_helper.h"

QString YACReaderServerDataHelper::folderToYSFormat(const qulonglong libraryId, const Folder & folder)
{
    return QString("f\x1F\t%1\x1F\t%2\x1F\t%3\x1F\t%4\x1F\t%5\x1E\r\n")
            .arg(libraryId)
            .arg(folder.id)
            .arg(folder.name)
            .arg(folder.getNumChildren())
            .arg(folder.getFirstChildHash());
}

QString YACReaderServerDataHelper::comicToYSFormat(const qulonglong libraryId,const ComicDB & comic)
{
    return QString("c\x1F\t%1\x1F\t%2\x1F\t%3\x1F\t%4\x1F\t%5\x1F\t%6\x1F\t%7\x1F\t%8\x1E\r\n")
            .arg(libraryId)
            .arg(comic.id)
            .arg(comic.getFileName())
            .arg(comic.getFileSize())
            .arg(comic.info.hash)
            .arg(comic.info.currentPage)
            .arg(comic.info.numPages.toInt())
            .arg(comic.info.read?1:0);
}

YACReaderServerDataHelper::YACReaderServerDataHelper() {}
