#include "yacreader_server_data_helper.h"

QString YACReaderServerDataHelper::folderToYSFormat(const qulonglong libraryId, const Folder &folder)
{
    return QString("f\x1F\t%1\x1F\t%2\x1F\t%3\x1F\t%4\x1F\t%5\x1E\r\n")
            .arg(libraryId)
            .arg(folder.id)
            .arg(folder.name)
            .arg(folder.numChildren)
            .arg(folder.firstChildHash);
}

QString YACReaderServerDataHelper::comicToYSFormat(const qulonglong libraryId, const ComicDB &comic)
{
    return QString("c\x1F\t%1\x1F\t%2\x1F\t%3\x1F\t%4\x1F\t%5\x1F\t%6\x1F\t%7\x1F\t%8\x1E\r\n")
            .arg(libraryId)
            .arg(comic.id)
            .arg(comic.getFileName())
            .arg(comic.getFileSize())
            .arg(comic.info.hash)
            .arg(comic.info.currentPage)
            .arg(comic.info.numPages.toInt())
            .arg(comic.info.read ? 1 : 0);
}

QJsonObject YACReaderServerDataHelper::folderToJSON(const qulonglong libraryId, const Folder &folder)
{
    QJsonObject json;

    json["type"] = "folder";
    json["id"] = QString::number(folder.id);
    json["library_id"] = QString::number(libraryId);
    json["folder_name"] = folder.name;
    json["num_children"] = folder.numChildren;
    json["first_comic_hash"] = folder.firstChildHash;
    // 9.13
    json["finished"] = folder.finished;
    json["completed"] = folder.completed;
    json["custom_image"] = folder.customImage;
    json["file_type"] = static_cast<typename std::underlying_type<YACReader::FileType>::type>(folder.type);
    json["added"] = folder.added;
    json["updated"] = folder.updated;

    return json;
}

QJsonObject YACReaderServerDataHelper::comicToJSON(const qulonglong libraryId, const ComicDB &comic)
{
    QJsonObject json;

    json["type"] = "comic";
    json["id"] = QString::number(comic.id);
    json["library_id"] = QString::number(libraryId);
    json["file_name"] = comic.name;
    json["file_size"] = QString::number(comic.getFileSize());
    json["hash"] = comic.info.hash;
    json["cover_page"] = comic.info.coverPage.toInt(); // 9.13
    json["current_page"] = comic.info.currentPage;
    json["num_pages"] = comic.info.numPages.toInt();
    json["read"] = comic.info.read;
    json["cover_size_ratio"] = comic.info.coverSizeRatio.toFloat();
    json["title"] = comic.info.title.toString();
    json["number"] = comic.info.number.toInt(); // 9.13 legacy, kept for compatibility with old clients
    json["universal_number"] = comic.info.number.toString(); // 9.13, text based number
    json["last_time_opened"] = comic.info.lastTimeOpened.toLongLong();
    auto type = comic.info.type.value<YACReader::FileType>();
    json["manga"] = type == YACReader::FileType::Manga; // legacy, kept for compatibility with old clients
    json["file_type"] = comic.info.type.toInt(); // 9.13

    return json;
}

QJsonObject YACReaderServerDataHelper::fullComicToJSON(const qulonglong libraryId, const ComicDB &comic)
{
    QJsonObject json = comicToJSON(libraryId, comic);

    json["volume"] = comic.info.volume.toString();
    json["total_volume_count"] = comic.info.count.toInt();
    json["genre"] = comic.info.genere.toString();
    json["date"] = comic.info.date.toString();

    json["synopsis"] = comic.info.synopsis.toString();

    // 9.13
    json["count"] = comic.info.count.toInt();
    json["story_arc"] = comic.info.storyArc.toString();
    json["arc_number"] = comic.info.arcNumber.toString();
    json["arc_count"] = comic.info.arcCount.toInt();
    json["writer"] = comic.info.writer.toString();
    json["penciller"] = comic.info.penciller.toString();
    json["inker"] = comic.info.inker.toString();
    json["colorist"] = comic.info.colorist.toString();
    json["letterer"] = comic.info.letterer.toString();
    json["cover_artist"] = comic.info.coverArtist.toString();
    json["publisher"] = comic.info.publisher.toString();
    json["format"] = comic.info.format.toString();
    json["color"] = comic.info.color.toBool();
    json["age_rating"] = comic.info.ageRating.toString();
    json["editor"] = comic.info.editor.toString();
    json["characters"] = comic.info.characters.toString();
    json["notes"] = comic.info.notes.toString();
    json["added"] = comic.info.added.toLongLong();
    json["editor"] = comic.info.editor.toString();
    json["imprint"] = comic.info.imprint.toString();
    json["teams"] = comic.info.teams.toString();
    json["locations"] = comic.info.locations.toString();
    json["series"] = comic.info.series.toString();
    json["alternate_series"] = comic.info.alternateSeries.toString();
    json["alternate_number"] = comic.info.alternateNumber.toString();
    json["alternate_count"] = comic.info.alternateCount.toInt();
    json["language_iso"] = comic.info.languageISO.toString();
    json["series_group"] = comic.info.seriesGroup.toString();
    json["main_character_or_team"] = comic.info.mainCharacterOrTeam.toString();
    json["review"] = comic.info.review.toString();
    json["tags"] = comic.info.tags.toString();
    auto type = comic.info.type.value<YACReader::FileType>();
    json["manga"] = type == YACReader::FileType::Manga; // legacy, kept for compatibility with old clients
    json["file_type"] = comic.info.type.toInt(); // 9.13

    return json;
}

QJsonObject YACReaderServerDataHelper::readingListToJSON(const qulonglong libraryId, const ReadingList &readingList)
{
    QJsonObject json;

    json["type"] = "reading_list";
    json["id"] = QString::number(readingList.getId());
    json["library_id"] = QString::number(libraryId);
    json["reading_list_name"] = readingList.getName();

    // TODO
    // json["num_children"] = readingList.getNumChildren();
    // json["first_comic_hash"] = readingList.getFirstChildHash();

    return json;
}

QJsonObject YACReaderServerDataHelper::labelToJSON(const qulonglong libraryId, const Label &label)
{
    QJsonObject json;

    json["type"] = "label";
    json["id"] = QString::number(label.getId());
    json["library_id"] = QString::number(libraryId);
    json["label_name"] = label.getName();
    json["color_id"] = label.getColorID();

    // TODO
    // json["num_children"] = label.getNumChildren();

    return json;
}

YACReaderServerDataHelper::YACReaderServerDataHelper() { }
