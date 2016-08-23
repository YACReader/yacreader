#include "yacreader_server_data_helper.h"

QString YACReaderServerDataHelper::folderToYSFormat(const qulonglong libraryId, const Folder & folder)
{
    return QString("f\t%1\t%2\t%3\t%4\t%5\r\n")
            .arg(libraryId)
            .arg(folder.id)
            .arg(folder.name)
            .arg(folder.getNumChildren())
            .arg(folder.getFirstChildHash());
}

QString YACReaderServerDataHelper::comicToYSFormat(const qulonglong libraryId,const ComicDB & comic)
{
    return QString("c\t%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\r\n")
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
