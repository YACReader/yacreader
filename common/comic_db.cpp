#include "comic_db.h"

#include "table_projection.h"
#include "yacreader_global.h"

#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

//-----------------------------------------------------------------------------
// COMIC------------------------------------------------------------------------
//-----------------------------------------------------------------------------
QString ComicDB::toTXT() const
{
    QString txt;

    // Legacy info
    txt.append(QString("comicid:%1\r\n").arg(file.id.value()));
    txt.append(QString("hash:%1\r\n").arg(info.hash));
    txt.append(QString("path:%1\r\n").arg(file.path));
    txt.append(QString("numpages:%1\r\n").arg(info.numPages.toString()));

    // new 7.0
    txt.append(QString("rating:%1\r\n").arg(info.rating));
    txt.append(QString("currentPage:%1\r\n").arg(info.currentPage));
    txt.append(QString("contrast:%1\r\n").arg(info.contrast));

    // send read status, at some point downloads have to migrate to use comicfullinfocontroller
    txt.append(QString("read:%1\r\n").arg(info.read ? 1 : 0));

    // Informaci�n general
    if (!info.coverPage.isNull())
        txt.append(QString("coverPage:%1\r\n").arg(info.coverPage.toString()));

    if (!info.title.isNull())
        txt.append(QString("title:%1\r\n").arg(info.title.toString()));

    if (!info.number.isNull())
        txt.append(QString("number:%1\r\n").arg(info.number.toString()));

    if (!info.isBis.isNull())
        txt.append(QString("isBis:%1\r\n").arg(info.isBis.toBool() ? "1" : "0"));

    if (!info.count.isNull())
        txt.append(QString("count:%1\r\n").arg(info.count.toString()));

    if (!info.volume.isNull())
        txt.append(QString("volume:%1\r\n").arg(info.volume.toString()));

    if (!info.storyArc.isNull())
        txt.append(QString("storyArc:%1\r\n").arg(info.storyArc.toString()));

    if (!info.arcNumber.isNull())
        txt.append(QString("arcNumber:%1\r\n").arg(info.arcNumber.toString()));

    if (!info.arcCount.isNull())
        txt.append(QString("arcCount:%1\r\n").arg(info.arcCount.toString()));

    if (!info.genere.isNull())
        txt.append(QString("genere:%1\r\n").arg(info.genere.toString()));

    // Autores
    if (!info.writer.isNull())
        txt.append(QString("writer:%1\r\n").arg(info.writer.toString()));

    if (!info.penciller.isNull())
        txt.append(QString("penciller:%1\r\n").arg(info.penciller.toString()));

    if (!info.inker.isNull())
        txt.append(QString("inker:%1\r\n").arg(info.inker.toString()));

    if (!info.colorist.isNull())
        txt.append(QString("colorist:%1\r\n").arg(info.colorist.toString()));

    if (!info.letterer.isNull())
        txt.append(QString("letterer:%1\r\n").arg(info.letterer.toString()));

    if (!info.coverArtist.isNull())
        txt.append(QString("coverArtist:%1\r\n").arg(info.coverArtist.toString()));
    // Publicaci�n
    if (!info.date.isNull())
        txt.append(QString("date:%1\r\n").arg(info.date.toString()));

    if (!info.publisher.isNull())
        txt.append(QString("publisher:%1\r\n").arg(info.publisher.toString()));

    if (!info.format.isNull())
        txt.append(QString("format:%1\r\n").arg(info.format.toString()));

    if (!info.color.isNull())
        txt.append(QString("color:%1\r\n").arg(info.color.toString()));

    if (!info.ageRating.isNull())
        txt.append(QString("ageRating:%1\r\n").arg(info.ageRating.toString()));

    if (!info.type.isNull())
        txt.append(QString("manga:%1\r\n").arg(info.type.value<YACReader::FileType>() == YACReader::FileType::Manga ? "1" : "0"));

    // Argumento
    if (!info.synopsis.isNull())
        txt.append(QString("synopsis:%1\r\n").arg(info.synopsis.toString()));

    if (!info.characters.isNull())
        txt.append(QString("characters:%1\r\n").arg(info.characters.toString()));

    if (!info.notes.isNull())
        txt.append(QString("notes:%1\r\n").arg(info.notes.toString()));

    if (!info.lastTimeOpened.isNull())
        txt.append(QString("lastTimeOpened:%1\r\n").arg(info.lastTimeOpened.toULongLong()));

    if (!info.added.isNull())
        txt.append(QString("added:%1\r\n").arg(info.added.toULongLong()));

    if (!info.type.isNull())
        txt.append(QString("type:%1\r\n").arg(info.type.toInt()));

    if (!info.editor.isNull())
        txt.append(QString("editor:%1\r\n").arg(info.editor.toString()));

    if (!info.imprint.isNull())
        txt.append(QString("imprint:%1\r\n").arg(info.imprint.toString()));

    if (!info.teams.isNull())
        txt.append(QString("teams:%1\r\n").arg(info.teams.toString()));

    if (!info.locations.isNull())
        txt.append(QString("locations:%1\r\n").arg(info.locations.toString()));

    if (!info.series.isNull())
        txt.append(QString("series:%1\r\n").arg(info.series.toString()));

    if (!info.alternateSeries.isNull())
        txt.append(QString("alternateSeries:%1\r\n").arg(info.alternateSeries.toString()));

    if (!info.alternateNumber.isNull())
        txt.append(QString("alternateNumber:%1\r\n").arg(info.alternateNumber.toString()));

    if (!info.alternateCount.isNull())
        txt.append(QString("alternateCount:%1\r\n").arg(info.alternateCount.toString()));

    if (!info.languageISO.isNull())
        txt.append(QString("languageISO:%1\r\n").arg(info.languageISO.toString()));

    if (!info.seriesGroup.isNull())
        txt.append(QString("seriesGroup:%1\r\n").arg(info.seriesGroup.toString()));

    if (!info.mainCharacterOrTeam.isNull())
        txt.append(QString("mainCharacterOrTeam:%1\r\n").arg(info.mainCharacterOrTeam.toString()));

    if (!info.review.isNull())
        txt.append(QString("review:%1\r\n").arg(info.review.toString()));

    if (!info.tags.isNull())
        txt.append(QString("tags:%1\r\n").arg(info.tags.toString()));

    // 9.16

    if (!info.imageFiltersJson.isNull())
        txt.append(QString("imageFiltersJson:%1\r\n").arg(info.imageFiltersJson.toString()));

    if (!info.lastTimeImageFiltersSet.isNull())
        txt.append(QString("lastTimeImageFiltersSet:%1\r\n").arg(info.lastTimeImageFiltersSet.toULongLong()));

    if (!info.lastTimeCoverSet.isNull())
        txt.append(QString("lastTimeCoverSet:%1\r\n").arg(info.lastTimeCoverSet.toULongLong()));

    if (!info.usesExternalCover.isNull())
        txt.append(QString("usesExternalCover:%1\r\n").arg(info.usesExternalCover.toBool() ? "1" : "0"));

    if (!info.lastTimeMetadataSet.isNull())
        txt.append(QString("lastTimeMetadataSet:%1\r\n").arg(info.lastTimeMetadataSet.toULongLong()));

    return txt;
}

QString ComicDB::getFileName() const
{
    return QFileInfo(file.path).fileName();
}

QString ComicDB::getTitleOrFileName() const
{
    return titleOrFileName(info, QFileInfo(file.path).fileName());
}

QString ComicDB::getParentFolderName() const
{
    QStringList paths = file.path.split('/');
    if (paths.length() < 2)
        return "";
    else
        return paths[paths.length() - 2];
}

qulonglong ComicDB::getFileSize() const
{
    // the size is encoded in the hash after the SHA-1
    return info.hash.right(info.hash.length() - 40).toLongLong();
}

QString ComicDB::getTitleIncludingNumber() const
{
    return titleIncludingNumber(info, QFileInfo(file.path).fileName());
}

QString ComicDB::getInfoTitle() const
{
    return infoTitle(info, QFileInfo(file.path).fileName());
}

QString titleOrFileName(const ComicInfo &info, const QString &fileName)
{
    if (!info.title.isNull() && !info.title.toString().isEmpty())
        return info.title.toString();
    else
        return fileName;
}

QString titleIncludingNumber(const ComicInfo &info, const QString &fileName)
{
    if (!info.number.isNull()) {
        return "#" + info.number.toString() + " - " + titleOrFileName(info, fileName);
    }

    return titleOrFileName(info, fileName);
}

QString infoTitle(const ComicInfo &info, const QString &fileName)
{
    if (!info.number.isNull() && !info.title.isNull() && !info.series.isNull())
        return "#" + info.number.toString() + " - " + info.title.toString() + " (" + info.series.toString() + ")";

    if (!info.title.isNull() && !info.series.isNull())
        return info.title.toString() + " (" + info.series.toString() + ")";

    if (!info.number.isNull() && !info.title.isNull())
        return "#" + info.number.toString() + " - " + info.title.toString();

    if (!info.number.isNull() && !info.series.isNull())
        return "#" + info.number.toString() + " - " + info.series.toString();

    if (!info.number.isNull())
        return "#" + info.number.toString() + " - " + titleOrFileName(info, fileName);

    if (!info.title.isNull() && !info.series.isNull())
        return info.title.toString() + " (" + info.series.toString() + ")";

    if (!info.title.isNull())
        return info.title.toString();

    if (!info.series.isNull())
        return info.series.toString();

    return fileName;
}

//-----------------------------------------------------------------------------
// COMIC_INFO-------------------------------------------------------------------
//-----------------------------------------------------------------------------
ComicInfo::ComicInfo()
    : read(false),
      edited(false),
      rating(0),
      hasBeenOpened(false),
      currentPage(1),
      bookmark1(-1),
      bookmark2(-1),
      bookmark3(-1),
      brightness(-1),
      contrast(-1),
      gamma(-1)
{
}

void ComicInfo::deleteMetadata()
{
    title = QVariant();

    coverPage = QVariant();

    number = QVariant();
    isBis = QVariant();
    count = QVariant();

    volume = QVariant();
    storyArc = QVariant();
    arcNumber = QVariant();
    arcCount = QVariant();

    genere = QVariant();

    writer = QVariant();
    penciller = QVariant();
    inker = QVariant();
    colorist = QVariant();
    letterer = QVariant();
    coverArtist = QVariant();

    date = QVariant();

    publisher = QVariant();
    format = QVariant();
    color = QVariant();
    ageRating = QVariant();

    synopsis = QVariant();
    characters = QVariant();
    notes = QVariant();

    // type = QVariant(); reset or not???
    editor = QVariant();
    imprint = QVariant();
    teams = QVariant();
    locations = QVariant();
    series = QVariant();
    alternateSeries = QVariant();
    alternateNumber = QVariant();
    alternateCount = QVariant();
    languageISO = QVariant();
    seriesGroup = QVariant();
    mainCharacterOrTeam = QVariant();
    review = QVariant();

    // tags = QVariant(); reset or not???

    comicVineID = QVariant();
}
QPixmap ComicInfo::getCover(const QString &basePath) const
{
    return QPixmap(YACReader::LibraryPaths::coverPath(basePath, hash));
}

QStringList ComicInfo::getWriters() const
{
    if (writer.toString().length() > 0) {
        return writer.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getPencillers() const
{
    if (penciller.toString().length() > 0) {
        return penciller.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getInkers() const
{
    if (inker.toString().length() > 0) {
        return inker.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getColorists() const
{
    if (colorist.toString().length() > 0) {
        return colorist.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getLetterers() const
{
    if (letterer.toString().length() > 0) {
        return letterer.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getCoverArtists() const
{
    if (coverArtist.toString().length() > 0) {
        return coverArtist.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getEditors() const
{
    if (editor.toString().length() > 0) {
        return editor.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getImprint() const
{
    if (imprint.toString().length() > 0) {
        return imprint.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getCharacters() const
{
    if (characters.toString().length() > 0) {
        return characters.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getTeams() const
{
    if (teams.toString().length() > 0) {
        return teams.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getLocations() const
{
    if (locations.toString().length() > 0) {
        return locations.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getTags() const
{
    if (tags.toString().length() > 0) {
        return tags.toString().split("\n");
    }

    return QStringList();
}

QString ComicInfo::getTypeString() const
{
    if (type.isNull()) {
        return QStringLiteral("Null");
    }

    if (!type.canConvert<YACReader::FileType>()) {
        assert(false);
        return QStringLiteral("Unknown");
    }

    switch (type.value<YACReader::FileType>()) {
    case YACReader::FileType::Comic:
        return QStringLiteral("Comic");
    case YACReader::FileType::Manga:
        return QStringLiteral("Manga");
    case YACReader::FileType::WesternManga:
        return QStringLiteral("Western Manga");
    case YACReader::FileType::WebComic:
        return QStringLiteral("Web Comic");
    case YACReader::FileType::Yonkoma:
        return QStringLiteral("4-Koma");
    }

    assert(false);
    return QStringLiteral("Unknown");
}

QString ComicInfo::getStoryArcInfoString() const
{
    if (arcNumber.toString().length() > 0 && arcCount.toString().length() > 0 && storyArc.toString().length() > 0) {
        return "(" + arcNumber.toString() + "/" + arcCount.toString() + ") " + storyArc.toString();
    }

    if (arcNumber.toString().length() > 0 && storyArc.toString().length() > 0) {
        return "(" + arcNumber.toString() + ") " + storyArc.toString();
    }

    return storyArc.toString().length() > 0 ? storyArc.toString() : "";
}

QString ComicInfo::getAlternateSeriesString() const
{
    if (alternateNumber.toString().length() > 0 && alternateCount.toString().length() > 0 && alternateSeries.toString().length() > 0) {
        return "(" + alternateNumber.toString() + "/" + alternateCount.toString() + ") " + alternateSeries.toString();
    }

    if (alternateNumber.toString().length() > 0 && alternateSeries.toString().length() > 0) {
        return "(" + alternateNumber.toString() + ") " + alternateSeries.toString();
    }

    return alternateSeries.toString().length() > 0 ? alternateSeries.toString() : "";
}

QString ComicDB::selectList(const QString &fileAlias, const QString &infoAlias)
{
    return TableProjection::selectList(fileAlias, ComicFile::columns()) + ", " + TableProjection::selectList(infoAlias, ComicInfo::columns());
}

ComicDB::ColumnIndexes ComicDB::columnIndexes(const QSqlRecord &record, const QString &fileAlias, const QString &infoAlias)
{
    return { TableProjection::columnIndexes(record, fileAlias, ComicFile::columns()), TableProjection::columnIndexes(record, infoAlias, ComicInfo::columns()) };
}

ComicDB ComicDB::fromRecord(const QSqlRecord &record, const QString &fileAlias, const QString &infoAlias)
{
    ComicDB comic;
    comic.file = ComicFile::fromRecord(record, fileAlias);
    comic.info = ComicInfo::fromRecord(record, infoAlias);
    return comic;
}

ComicDB ComicDB::fromQuery(const QSqlQuery &query, const ColumnIndexes &columnIndexes)
{
    ComicDB comic;
    comic.file = ComicFile::fromQuery(query, columnIndexes.file);
    comic.info = ComicInfo::fromQuery(query, columnIndexes.info);
    return comic;
}

const QStringList &ComicInfo::columns()
{
    static const QStringList columns = [] {
        QStringList result = { "hash", "edited", "read", "hasBeenOpened", "rating", "currentPage",
                               "bookmark1", "bookmark2", "bookmark3", "brightness", "contrast", "gamma",
                               "coverPage", "numPages", "lastTimeOpened", "coverSizeRatio", "originalCoverSize",
                               "added", "type", "imageFiltersJson", "lastTimeImageFiltersSet", "lastTimeCoverSet",
                               "usesExternalCover", "lastTimeMetadataSet" };
        for (const auto &column : DescriptiveComicMetadata::columns())
            result.append(QString::fromLatin1(column.name));
        return result;
    }();
    return columns;
}

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
bool ComicInfo::hasSameValues(const ComicInfo &other) const
{
    for (const auto &column : DescriptiveComicMetadata::columns()) {
        if (this->*column.member != other.*column.member)
            return false;
    }

    return id == other.id && hash == other.hash && read == other.read && edited == other.edited &&
            hasBeenOpened == other.hasBeenOpened && rating == other.rating && currentPage == other.currentPage &&
            bookmark1 == other.bookmark1 && bookmark2 == other.bookmark2 && bookmark3 == other.bookmark3 &&
            brightness == other.brightness && contrast == other.contrast && gamma == other.gamma &&
            coverPage == other.coverPage && numPages == other.numPages && lastTimeOpened == other.lastTimeOpened &&
            coverSizeRatio == other.coverSizeRatio && originalCoverSize == other.originalCoverSize &&
            added == other.added && type == other.type && imageFiltersJson == other.imageFiltersJson &&
            lastTimeImageFiltersSet == other.lastTimeImageFiltersSet && lastTimeCoverSet == other.lastTimeCoverSet &&
            usesExternalCover == other.usesExternalCover && lastTimeMetadataSet == other.lastTimeMetadataSet;
}

namespace {

// `value(k)` is column k of {id} + ComicInfo::columns()
template<typename Value>
ComicInfo readComicInfo(const Value &value)
{
    ComicInfo info;

    info.id = ComicInfoId(value(0).toULongLong());
    info.hash = value(1).toString();
    info.edited = value(2).toBool();
    info.read = value(3).toBool();
    info.hasBeenOpened = value(4).toBool();
    info.rating = value(5).toInt();
    info.currentPage = value(6).toInt();
    info.bookmark1 = value(7).toInt();
    info.bookmark2 = value(8).toInt();
    info.bookmark3 = value(9).toInt();
    info.brightness = value(10).toInt();
    info.contrast = value(11).toInt();
    info.gamma = value(12).toInt();
    info.coverPage = value(13);
    info.numPages = value(14);
    info.lastTimeOpened = value(15);
    info.coverSizeRatio = value(16);
    info.originalCoverSize = value(17);
    info.added = value(18);
    info.type = value(19);
    info.imageFiltersJson = value(20);
    info.lastTimeImageFiltersSet = value(21);
    info.lastTimeCoverSet = value(22);
    info.usesExternalCover = value(23).toBool();
    info.lastTimeMetadataSet = value(24);

    constexpr int firstDescriptiveColumn = 25;
    int column = firstDescriptiveColumn;
    for (const auto &descriptive : DescriptiveComicMetadata::columns()) {
        info.*descriptive.member = value(column++);
    }

    return info;
}

}

ComicInfo ComicInfo::fromRecord(const QSqlRecord &record, const QString &alias)
{
    const auto indexes = TableProjection::columnIndexes(record, alias, columns());
    return readComicInfo([&](int column) { return record.value(indexes.at(column)); });
}

ComicInfo ComicInfo::fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes)
{
    return readComicInfo([&](int column) { return query.value(columnIndexes.at(column)); });
}
QT_WARNING_POP

QDataStream &operator<<(QDataStream &stream, const ComicDB &comic)
{
    stream << comic.file.id.value();
    stream << comic.file.fileName;
    stream << comic.file.folderId.value();
    stream << comic.file.path;
    stream << (comic.info.id != ComicInfoId()); // hasCover
    stream << comic.info;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, ComicDB &comic)
{
    qulonglong id;
    qulonglong folderId;
    bool hasCover;
    stream >> id;
    stream >> comic.file.fileName;
    stream >> folderId;
    stream >> comic.file.path;
    stream >> hasCover;
    stream >> comic.info;
    comic.file.id = ComicId(id);
    comic.file.folderId = FolderId(folderId);
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const ComicInfo &comicInfo)
{
    stream << comicInfo.id.value();
    stream << comicInfo.read;
    stream << comicInfo.edited;
    stream << comicInfo.hash;
    stream << (comicInfo.id != ComicInfoId()); // existOnDb

    stream << comicInfo.hasBeenOpened;
    stream << comicInfo.rating;
    stream << comicInfo.currentPage;
    stream << comicInfo.bookmark1;
    stream << comicInfo.bookmark2;
    stream << comicInfo.bookmark3;
    stream << comicInfo.brightness;
    stream << comicInfo.contrast;
    stream << comicInfo.gamma;

    stream << comicInfo.title;

    stream << comicInfo.coverPage;
    stream << comicInfo.numPages;

    stream << comicInfo.number;
    stream << comicInfo.isBis;
    stream << comicInfo.count;

    stream << comicInfo.volume;
    stream << comicInfo.storyArc;
    stream << comicInfo.arcNumber;
    stream << comicInfo.arcCount;

    stream << comicInfo.genere;

    stream << comicInfo.writer;
    stream << comicInfo.penciller;
    stream << comicInfo.inker;
    stream << comicInfo.colorist;
    stream << comicInfo.letterer;
    stream << comicInfo.coverArtist;

    stream << comicInfo.date;
    stream << comicInfo.publisher;
    stream << comicInfo.format;
    stream << comicInfo.color;
    stream << comicInfo.ageRating;

    stream << comicInfo.synopsis;
    stream << comicInfo.characters;
    stream << comicInfo.notes;

    stream << comicInfo.comicVineID;

    stream << comicInfo.lastTimeOpened;

    stream << comicInfo.coverSizeRatio;
    stream << comicInfo.originalCoverSize;

    stream << comicInfo.added;
    stream << comicInfo.type;
    stream << comicInfo.added;
    stream << comicInfo.type;
    stream << comicInfo.editor;
    stream << comicInfo.imprint;
    stream << comicInfo.teams;
    stream << comicInfo.locations;
    stream << comicInfo.series;
    stream << comicInfo.alternateSeries;
    stream << comicInfo.alternateNumber;
    stream << comicInfo.alternateCount;
    stream << comicInfo.languageISO;
    stream << comicInfo.seriesGroup;
    stream << comicInfo.mainCharacterOrTeam;
    stream << comicInfo.review;
    stream << comicInfo.tags;

    stream << comicInfo.imageFiltersJson;
    stream << comicInfo.lastTimeImageFiltersSet;
    stream << comicInfo.lastTimeCoverSet;
    stream << comicInfo.usesExternalCover;
    stream << comicInfo.lastTimeMetadataSet;

    return stream;
}

QDataStream &operator>>(QDataStream &stream, ComicInfo &comicInfo)
{
    qulonglong id;
    stream >> id;
    comicInfo.id = ComicInfoId(id);
    stream >> comicInfo.read;
    stream >> comicInfo.edited;
    stream >> comicInfo.hash;
    bool existOnDb;
    stream >> existOnDb;

    stream >> comicInfo.hasBeenOpened;
    stream >> comicInfo.rating;
    stream >> comicInfo.currentPage;
    stream >> comicInfo.bookmark1;
    stream >> comicInfo.bookmark2;
    stream >> comicInfo.bookmark3;
    stream >> comicInfo.brightness;
    stream >> comicInfo.contrast;
    stream >> comicInfo.gamma;

    stream >> comicInfo.title;

    stream >> comicInfo.coverPage;
    stream >> comicInfo.numPages;

    stream >> comicInfo.number;
    stream >> comicInfo.isBis;
    stream >> comicInfo.count;

    stream >> comicInfo.volume;
    stream >> comicInfo.storyArc;
    stream >> comicInfo.arcNumber;
    stream >> comicInfo.arcCount;

    stream >> comicInfo.genere;

    stream >> comicInfo.writer;
    stream >> comicInfo.penciller;
    stream >> comicInfo.inker;
    stream >> comicInfo.colorist;
    stream >> comicInfo.letterer;
    stream >> comicInfo.coverArtist;

    stream >> comicInfo.date;
    stream >> comicInfo.publisher;
    stream >> comicInfo.format;
    stream >> comicInfo.color;
    stream >> comicInfo.ageRating;

    stream >> comicInfo.synopsis;
    stream >> comicInfo.characters;
    stream >> comicInfo.notes;

    stream >> comicInfo.comicVineID;

    stream >> comicInfo.lastTimeOpened;

    stream >> comicInfo.coverSizeRatio;
    stream >> comicInfo.originalCoverSize;

    stream >> comicInfo.added;
    stream >> comicInfo.type;
    stream >> comicInfo.added;
    stream >> comicInfo.type;
    stream >> comicInfo.editor;
    stream >> comicInfo.imprint;
    stream >> comicInfo.teams;
    stream >> comicInfo.locations;
    stream >> comicInfo.series;
    stream >> comicInfo.alternateSeries;
    stream >> comicInfo.alternateNumber;
    stream >> comicInfo.alternateCount;
    stream >> comicInfo.languageISO;
    stream >> comicInfo.seriesGroup;
    stream >> comicInfo.mainCharacterOrTeam;
    stream >> comicInfo.review;
    stream >> comicInfo.tags;

    stream >> comicInfo.imageFiltersJson;
    stream >> comicInfo.lastTimeImageFiltersSet;
    stream >> comicInfo.lastTimeCoverSet;
    stream >> comicInfo.usesExternalCover;
    stream >> comicInfo.lastTimeMetadataSet;

    return stream;
}
