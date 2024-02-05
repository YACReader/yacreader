#include "comic_db.h"

#include "yacreader_global.h"

#include <QVariant>
#include <QFileInfo>

//-----------------------------------------------------------------------------
// COMIC------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ComicDB::ComicDB()
{
}

ComicDB::ComicDB(const ComicDB &comicDB)
{
    operator=(comicDB);
}

bool ComicDB::isDir() const
{
    return false;
}

QString ComicDB::toTXT()
{
    QString txt;

    // Legacy info
    txt.append(QString("comicid:%1\r\n").arg(id));
    txt.append(QString("hash:%1\r\n").arg(info.hash));
    txt.append(QString("path:%1\r\n").arg(path));
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

    return txt;
}

ComicDB &ComicDB::operator=(const ComicDB &other)
{
    LibraryItem::operator=(other);

    this->_hasCover = other._hasCover;

    this->info = other.info;

    return *this;
}

QString ComicDB::getFileName() const
{
    return QFileInfo(path).fileName();
}

QString ComicDB::getTitleOrFileName() const
{
    if (!info.title.isNull() && !info.title.toString().isEmpty())
        return info.title.toString();
    else
        return QFileInfo(path).fileName();
}

QString ComicDB::getParentFolderName() const
{
    QStringList paths = path.split('/');
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
    if (!info.number.isNull()) {
        return "#" + info.number.toString() + " - " + getTitleOrFileName();
    }

    return getTitleOrFileName();
}

QString ComicDB::getInfoTitle() const
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
        return "#" + info.number.toString() + " - " + getTitleOrFileName();

    if (!info.title.isNull() && !info.series.isNull())
        return info.title.toString() + " (" + info.series.toString() + ")";

    if (!info.title.isNull())
        return info.title.toString();

    if (!info.series.isNull())
        return info.series.toString();

    return QFileInfo(path).fileName();
}

//-----------------------------------------------------------------------------
// COMIC_INFO-------------------------------------------------------------------
//-----------------------------------------------------------------------------
ComicInfo::ComicInfo()
    : read(false),
      edited(false),
      existOnDb(false),
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

ComicInfo::ComicInfo(const ComicInfo &comicInfo)
    : QObject()
{
    operator=(comicInfo);
}

ComicInfo::~ComicInfo()
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

    day = QVariant();
    month = QVariant();
    year = QVariant();

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
// the default operator= should work
ComicInfo &ComicInfo::operator=(const ComicInfo &comicInfo)
{
    hash = comicInfo.hash;
    id = comicInfo.id;
    existOnDb = comicInfo.existOnDb;
    read = comicInfo.read;
    edited = comicInfo.edited;

    hasBeenOpened = comicInfo.hasBeenOpened;
    rating = comicInfo.rating;
    currentPage = comicInfo.currentPage;
    bookmark1 = comicInfo.bookmark1;
    bookmark2 = comicInfo.bookmark2;
    bookmark3 = comicInfo.bookmark3;
    brightness = comicInfo.brightness;
    contrast = comicInfo.contrast;
    gamma = comicInfo.gamma;

    title = comicInfo.title;
    coverPage = comicInfo.coverPage;
    numPages = comicInfo.numPages;
    number = comicInfo.number;
    isBis = comicInfo.isBis;
    count = comicInfo.count;
    volume = comicInfo.volume;
    storyArc = comicInfo.storyArc;
    arcNumber = comicInfo.arcNumber;
    arcCount = comicInfo.arcCount;
    genere = comicInfo.genere;
    writer = comicInfo.writer;
    penciller = comicInfo.penciller;
    inker = comicInfo.inker;
    colorist = comicInfo.colorist;
    letterer = comicInfo.letterer;
    coverArtist = comicInfo.coverArtist;
    date = comicInfo.date;
    publisher = comicInfo.publisher;
    format = comicInfo.format;
    color = comicInfo.color;
    ageRating = comicInfo.ageRating;
    synopsis = comicInfo.synopsis;
    characters = comicInfo.characters;
    notes = comicInfo.notes;
    comicVineID = comicInfo.comicVineID;

    lastTimeOpened = comicInfo.lastTimeOpened;

    coverSizeRatio = comicInfo.coverSizeRatio;
    originalCoverSize = comicInfo.originalCoverSize;

    added = comicInfo.added;
    type = comicInfo.type;
    editor = comicInfo.editor;
    imprint = comicInfo.imprint;
    teams = comicInfo.teams;
    locations = comicInfo.locations;
    series = comicInfo.series;
    alternateSeries = comicInfo.alternateSeries;
    alternateNumber = comicInfo.alternateNumber;
    alternateCount = comicInfo.alternateCount;
    languageISO = comicInfo.languageISO;
    seriesGroup = comicInfo.seriesGroup;
    mainCharacterOrTeam = comicInfo.mainCharacterOrTeam;
    review = comicInfo.review;
    tags = comicInfo.tags;

    return *this;
}

QPixmap ComicInfo::getCover(const QString &basePath)
{
    if (cover.isNull()) {
        cover.load(basePath + "/.yacreaderlibrary/covers/" + hash + ".jpg");
    }
    QPixmap c;
    c.convertFromImage(cover);
    return c;
}

QStringList ComicInfo::getWriters()
{
    if (writer.toString().length() > 0) {
        return writer.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getPencillers()
{
    if (penciller.toString().length() > 0) {
        return penciller.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getInkers()
{
    if (inker.toString().length() > 0) {
        return inker.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getColorists()
{
    if (colorist.toString().length() > 0) {
        return colorist.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getLetterers()
{
    if (letterer.toString().length() > 0) {
        return letterer.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getCoverArtists()
{
    if (coverArtist.toString().length() > 0) {
        return coverArtist.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getEditors()
{
    if (editor.toString().length() > 0) {
        return editor.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getImprint()
{
    if (imprint.toString().length() > 0) {
        return imprint.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getCharacters()
{
    if (characters.toString().length() > 0) {
        return characters.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getTeams()
{
    if (teams.toString().length() > 0) {
        return teams.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getLocations()
{
    if (locations.toString().length() > 0) {
        return locations.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getTags()
{
    if (tags.toString().length() > 0) {
        return tags.toString().split("\n");
    }

    return QStringList();
}

QString ComicInfo::getTypeString()
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

QString ComicInfo::getStoryArcInfoString()
{
    if (arcNumber.toString().length() > 0 && arcCount.toString().length() > 0 && storyArc.toString().length() > 0) {
        return "(" + arcNumber.toString() + "/" + arcCount.toString() + ") " + storyArc.toString();
    }

    if (arcNumber.toString().length() > 0 && storyArc.toString().length() > 0) {
        return "(" + arcNumber.toString() + ") " + storyArc.toString();
    }

    return storyArc.toString().length() > 0 ? storyArc.toString() : "";
}

QString ComicInfo::getAlternateSeriesString()
{
    if (alternateNumber.toString().length() > 0 && alternateCount.toString().length() > 0 && alternateSeries.toString().length() > 0) {
        return "(" + alternateNumber.toString() + "/" + alternateCount.toString() + ") " + alternateSeries.toString();
    }

    if (alternateNumber.toString().length() > 0 && alternateSeries.toString().length() > 0) {
        return "(" + alternateNumber.toString() + ") " + alternateSeries.toString();
    }

    return alternateSeries.toString().length() > 0 ? alternateSeries.toString() : "";
}

void ComicInfo::setRead(bool r)
{
    if (r != read) {
        read = r;
        emit readChanged();
    }
}

void ComicInfo::setRating(int r)
{
    if (r != rating) {
        rating = r;
        emit ratingChanged();
    }
}

void ComicInfo::setFavorite(bool f)
{
    if (f != isFavorite) {
        isFavorite = f;
        emit favoriteChanged();
    }
}

QDataStream &operator<<(QDataStream &stream, const ComicDB &comic)
{
    stream << comic.id;
    stream << comic.name;
    stream << comic.parentId;
    stream << comic.path;
    stream << comic._hasCover;
    stream << comic.info;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, ComicDB &comic)
{
    stream >> comic.id;
    stream >> comic.name;
    stream >> comic.parentId;
    stream >> comic.path;
    stream >> comic._hasCover;
    stream >> comic.info;
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const ComicInfo &comicInfo)
{
    stream << comicInfo.id;
    stream << comicInfo.read;
    stream << comicInfo.edited;
    stream << comicInfo.hash;
    stream << comicInfo.existOnDb;

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

    return stream;
}

QDataStream &operator>>(QDataStream &stream, ComicInfo &comicInfo)
{
    stream >> comicInfo.id;
    stream >> comicInfo.read;
    stream >> comicInfo.edited;
    stream >> comicInfo.hash;
    stream >> comicInfo.existOnDb;

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

    return stream;
}
