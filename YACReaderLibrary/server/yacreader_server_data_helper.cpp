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

QJsonObject YACReaderServerDataHelper::folderToJSON(const qulonglong libraryId, const Folder & folder)
{
    QJsonObject json;

    json["type"] = "folder";
    json["id"] = QString::number(folder.id);
    json["library_id"] = QString::number(libraryId);
    json["folder_name"] = folder.name;
    json["num_children"] = folder.getNumChildren();
    json["first_comic_hash"] = folder.getFirstChildHash();

    return json;
}

QJsonObject YACReaderServerDataHelper::comicToJSON(const qulonglong libraryId, const ComicDB & comic)
{
    QJsonObject json;

    json["type"] = "comic";
    json["id"] = QString::number(comic.id);
    json["library_id"] = QString::number(libraryId);
    json["file_name"] = comic.name;
    json["file_size"] = QString::number(comic.getFileSize());
    json["hash"] = comic.info.hash;
    json["current_page"] = comic.info.currentPage;
    json["num_pages"] = comic.info.numPages.toInt();
    json["read"] = comic.info.read;
    json["cover_size_ratio"] = comic.info.coverSizeRatio.toFloat();
    json["title"] = comic.info.title.toString();
    json["number"] = comic.info.number.toInt();

    return json;
}

QJsonObject YACReaderServerDataHelper::fullComicToJSON(const qulonglong libraryId, const ComicDB & comic)
{
    QJsonObject json = comicToJSON(libraryId, comic);

    json["volume"] = comic.info.volume.toString();
    json["total_volume_count"] = comic.info.count.toInt();
    json["genre"] = comic.info.genere.toString();
    json["date"] = comic.info.date.toString();

    json["synopsis"] = comic.info.synopsis.toString();

    return json;
}

QJsonObject YACReaderServerDataHelper::readingListToJSON(const qulonglong libraryId, const ReadingList &readingList)
{
    QJsonObject json;

    json["type"] = "reading_list";
    json["id"] = QString::number(readingList.getId());
    json["library_id"] = QString::number(libraryId);
    json["reading_list_name"] = readingList.getName();

    //TODO
    //json["num_children"] = readingList.getNumChildren();
    //json["first_comic_hash"] = readingList.getFirstChildHash();

    return json;
}

QJsonObject YACReaderServerDataHelper::labelToJSON(const qulonglong libraryId, const Label &label)
{
    QJsonObject json;

    json["type"] = "label";
    json["id"] = QString::number(label.getId());
    json["library_id"] = QString::number(libraryId);
    json["label_list_name"] = label.getName();
    json["color_id"] = label.getColorID();

    //TODO
    //json["num_children"] = label.getNumChildren();

    return json;
}

YACReaderServerDataHelper::YACReaderServerDataHelper() {}
