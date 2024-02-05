#include "yacreader_server_data_helper.h"

void variantToJson(const QString &name, QMetaType::Type type, const QVariant &value, QJsonObject &json)
{
    if (value.isNull() || !value.isValid()) {
        return;
    }

    switch (type) {
    case QMetaType::Bool:
        json[name] = value.toBool();
        break;
    case QMetaType::Int:
        json[name] = value.toInt();
        break;
    case QMetaType::Float:
        json[name] = value.toFloat();
        break;
    case QMetaType::Double:
        json[name] = value.toDouble();
        break;
    case QMetaType::LongLong:
        json[name] = value.toLongLong();
        break;
    case QMetaType::QString:
        json[name] = value.toString();
        break;
    default:
        break;
    }
}

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

QJsonObject YACReaderServerDataHelper::folderToJSON(const qulonglong libraryId, const QUuid libraryUuid, const Folder &folder)
{
    QJsonObject json;

    json["type"] = "folder";
    json["id"] = QString::number(folder.id);
    json["library_id"] = QString::number(libraryId);
    if (!libraryUuid.isNull()) {
        json["library_uuid"] = libraryUuid.toString();
    }
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
    // 9.14
    json["parent_id"] = QString::number(folder.parentId);
    json["path"] = folder.path;

    return json;
}

QJsonObject YACReaderServerDataHelper::comicToJSON(const qulonglong libraryId, const QUuid libraryUuid, const ComicDB &comic)
{
    QJsonObject json;

    json["type"] = "comic";
    json["id"] = QString::number(comic.id);
    json["comic_info_id"] = QString::number(comic.info.id);
    json["parent_id"] = QString::number(comic.parentId); // 9.14
    json["library_id"] = QString::number(libraryId);
    if (!libraryUuid.isNull()) {
        json["library_uuid"] = libraryUuid.toString();
    }
    json["file_name"] = comic.name;
    json["file_size"] = QString::number(comic.getFileSize());
    json["hash"] = comic.info.hash;
    json["path"] = comic.path; // 9.14

    json["current_page"] = comic.info.currentPage;
    json["num_pages"] = comic.info.numPages.toInt();
    json["read"] = comic.info.read;
    auto type = comic.info.type.value<YACReader::FileType>();
    json["manga"] = type == YACReader::FileType::Manga; // legacy, kept for compatibility with old clients
    json["file_type"] = comic.info.type.toInt(); // 9.13
    json["cover_size_ratio"] = comic.info.coverSizeRatio.toFloat();

    // legacy
    json["number"] = comic.info.number.toInt();

    variantToJson("cover_page", QMetaType::Int, comic.info.coverPage, json);
    variantToJson("title", QMetaType::QString, comic.info.title, json);
    variantToJson("universal_number", QMetaType::QString, comic.info.number, json);
    variantToJson("last_time_opened", QMetaType::LongLong, comic.info.lastTimeOpened, json);
    json["has_been_opened"] = comic.info.hasBeenOpened;

    variantToJson("added", QMetaType::LongLong, comic.info.added, json);

    return json;
}

QJsonObject YACReaderServerDataHelper::fullComicToJSON(const qulonglong libraryId, const QUuid libraryUuid, const ComicDB &comic)
{
    QJsonObject json = comicToJSON(libraryId, libraryUuid, comic);

    variantToJson("volume", QMetaType::QString, comic.info.volume, json);
    variantToJson("total_volume_count", QMetaType::Int, comic.info.count, json);
    variantToJson("genre", QMetaType::QString, comic.info.genere, json);
    variantToJson("date", QMetaType::QString, comic.info.date, json);
    variantToJson("synopsis", QMetaType::QString, comic.info.synopsis, json);

    // 9.13
    variantToJson("count", QMetaType::Int, comic.info.count, json);
    variantToJson("story_arc", QMetaType::QString, comic.info.storyArc, json);
    variantToJson("arc_number", QMetaType::QString, comic.info.arcNumber, json);
    variantToJson("arc_count", QMetaType::Int, comic.info.arcCount, json);
    variantToJson("writer", QMetaType::QString, comic.info.writer, json);
    variantToJson("penciller", QMetaType::QString, comic.info.penciller, json);
    variantToJson("inker", QMetaType::QString, comic.info.inker, json);
    variantToJson("colorist", QMetaType::QString, comic.info.colorist, json);
    variantToJson("letterer", QMetaType::QString, comic.info.letterer, json);
    variantToJson("cover_artist", QMetaType::QString, comic.info.coverArtist, json);
    variantToJson("publisher", QMetaType::QString, comic.info.publisher, json);
    variantToJson("format", QMetaType::QString, comic.info.format, json);
    variantToJson("color", QMetaType::Bool, comic.info.color, json);
    variantToJson("age_rating", QMetaType::QString, comic.info.ageRating, json);
    variantToJson("editor", QMetaType::QString, comic.info.editor, json);
    variantToJson("characters", QMetaType::QString, comic.info.characters, json);
    variantToJson("notes", QMetaType::QString, comic.info.notes, json);
    variantToJson("imprint", QMetaType::QString, comic.info.imprint, json);
    variantToJson("teams", QMetaType::QString, comic.info.teams, json);
    variantToJson("locations", QMetaType::QString, comic.info.locations, json);
    variantToJson("series", QMetaType::QString, comic.info.series, json);
    variantToJson("alternate_series", QMetaType::QString, comic.info.alternateSeries, json);
    variantToJson("alternate_number", QMetaType::QString, comic.info.alternateNumber, json);
    variantToJson("alternate_count", QMetaType::Int, comic.info.alternateCount, json);
    variantToJson("language_iso", QMetaType::QString, comic.info.languageISO, json);
    variantToJson("series_group", QMetaType::QString, comic.info.seriesGroup, json);
    variantToJson("main_character_or_team", QMetaType::QString, comic.info.mainCharacterOrTeam, json);
    variantToJson("review", QMetaType::QString, comic.info.review, json);
    variantToJson("tags", QMetaType::QString, comic.info.tags, json);
    // 9.14
    json["rating"] = comic.info.rating;
    variantToJson("comic_vine_id", QMetaType::QString, comic.info.comicVineID, json);
    variantToJson("original_cover_size", QMetaType::QString, comic.info.originalCoverSize, json);
    json["edited"] = comic.info.edited;
    json["bookmark1"] = comic.info.bookmark1;
    json["bookmark2"] = comic.info.bookmark2;
    json["bookmark3"] = comic.info.bookmark3;
    json["brightness"] = comic.info.brightness;
    json["contrast"] = comic.info.contrast;
    json["gamma"] = comic.info.gamma;

    return json;
}

QJsonObject YACReaderServerDataHelper::readingListToJSON(const qulonglong libraryId, const QUuid libraryUuid, const ReadingList &readingList)
{
    QJsonObject json;

    json["type"] = "reading_list";
    json["id"] = QString::number(readingList.getId());
    json["library_id"] = QString::number(libraryId);
    if (!libraryUuid.isNull()) {
        json["library_uuid"] = libraryUuid.toString();
    }
    json["reading_list_name"] = readingList.getName();

    // TODO
    // json["num_children"] = readingList.getNumChildren();
    // json["first_comic_hash"] = readingList.getFirstChildHash();

    return json;
}

QJsonObject YACReaderServerDataHelper::labelToJSON(const qulonglong libraryId, const QUuid libraryUuid, const Label &label)
{
    QJsonObject json;

    json["type"] = "label";
    json["id"] = QString::number(label.getId());
    json["library_id"] = QString::number(libraryId);
    if (!libraryUuid.isNull()) {
        json["library_uuid"] = libraryUuid.toString();
    }
    json["label_name"] = label.getName();
    json["color_id"] = label.getColorID();

    // TODO
    // json["num_children"] = label.getNumChildren();

    return json;
}

YACReaderServerDataHelper::YACReaderServerDataHelper() { }
