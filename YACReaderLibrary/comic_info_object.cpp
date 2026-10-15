#include "comic_info_object.h"

ComicInfoObject::ComicInfoObject(QObject *parent)
    : QObject(parent), favorite(false)
{
}

ComicInfoObject::ComicInfoObject(const ComicDB &comic, bool isFavorite, QObject *parent)
    : QObject(parent), comic(comic), favorite(isFavorite)
{
}

qulonglong ComicInfoObject::id() const
{
    return comic.info.id.value();
}

bool ComicInfoObject::read() const
{
    return comic.info.read;
}

bool ComicInfoObject::edited() const
{
    return comic.info.edited;
}

QString ComicInfoObject::hash() const
{
    return comic.info.hash;
}

int ComicInfoObject::rating() const
{
    return comic.info.rating;
}

bool ComicInfoObject::hasBeenOpened() const
{
    return comic.info.hasBeenOpened;
}

int ComicInfoObject::currentPage() const
{
    return comic.info.currentPage;
}

int ComicInfoObject::bookmark1() const
{
    return comic.info.bookmark1;
}

int ComicInfoObject::bookmark2() const
{
    return comic.info.bookmark2;
}

int ComicInfoObject::bookmark3() const
{
    return comic.info.bookmark3;
}

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
int ComicInfoObject::brightness() const
{
    return comic.info.brightness;
}

int ComicInfoObject::contrast() const
{
    return comic.info.contrast;
}

int ComicInfoObject::gamma() const
{
    return comic.info.gamma;
}
QT_WARNING_POP

QVariant ComicInfoObject::title() const
{
    return comic.info.title;
}

QVariant ComicInfoObject::coverPage() const
{
    return comic.info.coverPage;
}

QVariant ComicInfoObject::numPages() const
{
    return comic.info.numPages;
}

QVariant ComicInfoObject::number() const
{
    return comic.info.number;
}

QVariant ComicInfoObject::isBis() const
{
    return comic.info.isBis;
}

QVariant ComicInfoObject::count() const
{
    return comic.info.count;
}

QVariant ComicInfoObject::volume() const
{
    return comic.info.volume;
}

QVariant ComicInfoObject::storyArc() const
{
    return comic.info.storyArc;
}

QVariant ComicInfoObject::arcNumber() const
{
    return comic.info.arcNumber;
}

QVariant ComicInfoObject::arcCount() const
{
    return comic.info.arcCount;
}

QVariant ComicInfoObject::genere() const
{
    return comic.info.genere;
}

QVariant ComicInfoObject::writer() const
{
    return comic.info.writer;
}

QVariant ComicInfoObject::penciller() const
{
    return comic.info.penciller;
}

QVariant ComicInfoObject::inker() const
{
    return comic.info.inker;
}

QVariant ComicInfoObject::colorist() const
{
    return comic.info.colorist;
}

QVariant ComicInfoObject::letterer() const
{
    return comic.info.letterer;
}

QVariant ComicInfoObject::coverArtist() const
{
    return comic.info.coverArtist;
}

QVariant ComicInfoObject::date() const
{
    return comic.info.date;
}

QVariant ComicInfoObject::day() const
{
    return comic.info.day;
}

QVariant ComicInfoObject::month() const
{
    return comic.info.month;
}

QVariant ComicInfoObject::year() const
{
    return comic.info.year;
}

QVariant ComicInfoObject::publisher() const
{
    return comic.info.publisher;
}

QVariant ComicInfoObject::format() const
{
    return comic.info.format;
}

QVariant ComicInfoObject::color() const
{
    return comic.info.color;
}

QVariant ComicInfoObject::ageRating() const
{
    return comic.info.ageRating;
}

QVariant ComicInfoObject::synopsis() const
{
    return comic.info.synopsis;
}

QVariant ComicInfoObject::characters() const
{
    return comic.info.characters;
}

QVariant ComicInfoObject::notes() const
{
    return comic.info.notes;
}

QVariant ComicInfoObject::comicVineID() const
{
    return comic.info.comicVineID;
}

QVariant ComicInfoObject::lastTimeOpened() const
{
    return comic.info.lastTimeOpened;
}

QVariant ComicInfoObject::coverSizeRatio() const
{
    return comic.info.coverSizeRatio;
}

QVariant ComicInfoObject::originalCoverSize() const
{
    return comic.info.originalCoverSize;
}

QVariant ComicInfoObject::added() const
{
    return comic.info.added;
}

QVariant ComicInfoObject::type() const
{
    return comic.info.type;
}

QVariant ComicInfoObject::editor() const
{
    return comic.info.editor;
}

QVariant ComicInfoObject::imprint() const
{
    return comic.info.imprint;
}

QVariant ComicInfoObject::teams() const
{
    return comic.info.teams;
}

QVariant ComicInfoObject::locations() const
{
    return comic.info.locations;
}

QVariant ComicInfoObject::series() const
{
    return comic.info.series;
}

QVariant ComicInfoObject::alternateSeries() const
{
    return comic.info.alternateSeries;
}

QVariant ComicInfoObject::alternateNumber() const
{
    return comic.info.alternateNumber;
}

QVariant ComicInfoObject::alternateCount() const
{
    return comic.info.alternateCount;
}

QVariant ComicInfoObject::languageISO() const
{
    return comic.info.languageISO;
}

QVariant ComicInfoObject::seriesGroup() const
{
    return comic.info.seriesGroup;
}

QVariant ComicInfoObject::mainCharacterOrTeam() const
{
    return comic.info.mainCharacterOrTeam;
}

QVariant ComicInfoObject::review() const
{
    return comic.info.review;
}

QVariant ComicInfoObject::tags() const
{
    return comic.info.tags;
}

QVariant ComicInfoObject::imageFiltersJson() const
{
    return comic.info.imageFiltersJson;
}

QVariant ComicInfoObject::lastTimeImageFiltersSet() const
{
    return comic.info.lastTimeImageFiltersSet;
}

QVariant ComicInfoObject::lastTimeCoverSet() const
{
    return comic.info.lastTimeCoverSet;
}

QVariant ComicInfoObject::usesExternalCover() const
{
    return comic.info.usesExternalCover;
}

QVariant ComicInfoObject::lastTimeMetadataSet() const
{
    return comic.info.lastTimeMetadataSet;
}

bool ComicInfoObject::isFavorite() const
{
    return favorite;
}

void ComicInfoObject::setRead(bool read)
{
    if (read != comic.info.read) {
        comic.info.read = read;
        emit readChanged();
    }
}

void ComicInfoObject::setRating(int rating)
{
    if (rating != comic.info.rating) {
        comic.info.rating = rating;
        emit ratingChanged();
    }
}

void ComicInfoObject::setFavorite(bool isFavorite)
{
    if (isFavorite != favorite) {
        favorite = isFavorite;
        emit favoriteChanged();
    }
}

QStringList ComicInfoObject::getWriters() const
{
    return comic.info.getWriters();
}

QStringList ComicInfoObject::getPencillers() const
{
    return comic.info.getPencillers();
}

QStringList ComicInfoObject::getInkers() const
{
    return comic.info.getInkers();
}

QStringList ComicInfoObject::getColorists() const
{
    return comic.info.getColorists();
}

QStringList ComicInfoObject::getLetterers() const
{
    return comic.info.getLetterers();
}

QStringList ComicInfoObject::getCoverArtists() const
{
    return comic.info.getCoverArtists();
}

QStringList ComicInfoObject::getEditors() const
{
    return comic.info.getEditors();
}

QStringList ComicInfoObject::getImprint() const
{
    return comic.info.getImprint();
}

QStringList ComicInfoObject::getCharacters() const
{
    return comic.info.getCharacters();
}

QStringList ComicInfoObject::getTeams() const
{
    return comic.info.getTeams();
}

QStringList ComicInfoObject::getLocations() const
{
    return comic.info.getLocations();
}

QStringList ComicInfoObject::getTags() const
{
    return comic.info.getTags();
}

QString ComicInfoObject::getTypeString() const
{
    return comic.info.getTypeString();
}

QString ComicInfoObject::getStoryArcInfoString() const
{
    return comic.info.getStoryArcInfoString();
}

QString ComicInfoObject::getAlternateSeriesString() const
{
    return comic.info.getAlternateSeriesString();
}

QString ComicInfoObject::getTitleOrFileName() const
{
    return comic.getTitleOrFileName();
}

qulonglong ComicInfoObject::getFileSize() const
{
    return comic.getFileSize();
}

QString ComicInfoObject::getTitleIncludingNumber() const
{
    return comic.getTitleIncludingNumber();
}

QString ComicInfoObject::getInfoTitle() const
{
    return comic.getInfoTitle();
}
