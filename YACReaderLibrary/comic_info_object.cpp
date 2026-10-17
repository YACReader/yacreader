#include "comic_info_object.h"

#include "overloaded.h"

#include <QFileInfo>

ComicInfoObject::ComicInfoObject(QObject *parent)
    : QObject(parent), favorite(false)
{
}

ComicInfoObject::ComicInfoObject(const ComicRecord &record, bool isFavorite, QObject *parent)
    : QObject(parent), favorite(isFavorite)
{
    std::visit(overloaded {
                       [this](const ComicRecordCase::Comic &comic) {
                           info = comic.comic.info;
                           fileName = QFileInfo(comic.comic.file.path).fileName();
                       },
                       [this](const ComicRecordCase::DeletedComic &comic) {
                           info = comic.info;
                           fileName = comic.lastKnownFile ? comic.lastKnownFile->name : QString();
                           isAvailable = false;
                       },
               },
               record);
}

qulonglong ComicInfoObject::id() const
{
    return info.id.value();
}

bool ComicInfoObject::read() const
{
    return info.read;
}

bool ComicInfoObject::edited() const
{
    return info.edited;
}

QString ComicInfoObject::hash() const
{
    return info.hash;
}

int ComicInfoObject::rating() const
{
    return info.rating;
}

bool ComicInfoObject::hasBeenOpened() const
{
    return info.hasBeenOpened;
}

int ComicInfoObject::currentPage() const
{
    return info.currentPage;
}

int ComicInfoObject::bookmark1() const
{
    return info.bookmark1;
}

int ComicInfoObject::bookmark2() const
{
    return info.bookmark2;
}

int ComicInfoObject::bookmark3() const
{
    return info.bookmark3;
}

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
int ComicInfoObject::brightness() const
{
    return info.brightness;
}

int ComicInfoObject::contrast() const
{
    return info.contrast;
}

int ComicInfoObject::gamma() const
{
    return info.gamma;
}
QT_WARNING_POP

QVariant ComicInfoObject::title() const
{
    return info.title;
}

QVariant ComicInfoObject::coverPage() const
{
    return info.coverPage;
}

QVariant ComicInfoObject::numPages() const
{
    return info.numPages;
}

QVariant ComicInfoObject::number() const
{
    return info.number;
}

QVariant ComicInfoObject::isBis() const
{
    return info.isBis;
}

QVariant ComicInfoObject::count() const
{
    return info.count;
}

QVariant ComicInfoObject::volume() const
{
    return info.volume;
}

QVariant ComicInfoObject::storyArc() const
{
    return info.storyArc;
}

QVariant ComicInfoObject::arcNumber() const
{
    return info.arcNumber;
}

QVariant ComicInfoObject::arcCount() const
{
    return info.arcCount;
}

QVariant ComicInfoObject::genere() const
{
    return info.genere;
}

QVariant ComicInfoObject::writer() const
{
    return info.writer;
}

QVariant ComicInfoObject::penciller() const
{
    return info.penciller;
}

QVariant ComicInfoObject::inker() const
{
    return info.inker;
}

QVariant ComicInfoObject::colorist() const
{
    return info.colorist;
}

QVariant ComicInfoObject::letterer() const
{
    return info.letterer;
}

QVariant ComicInfoObject::coverArtist() const
{
    return info.coverArtist;
}

QVariant ComicInfoObject::date() const
{
    return info.date;
}

QVariant ComicInfoObject::publisher() const
{
    return info.publisher;
}

QVariant ComicInfoObject::format() const
{
    return info.format;
}

QVariant ComicInfoObject::color() const
{
    return info.color;
}

QVariant ComicInfoObject::ageRating() const
{
    return info.ageRating;
}

QVariant ComicInfoObject::synopsis() const
{
    return info.synopsis;
}

QVariant ComicInfoObject::characters() const
{
    return info.characters;
}

QVariant ComicInfoObject::notes() const
{
    return info.notes;
}

QVariant ComicInfoObject::comicVineID() const
{
    return info.comicVineID;
}

QVariant ComicInfoObject::lastTimeOpened() const
{
    return info.lastTimeOpened;
}

QVariant ComicInfoObject::coverSizeRatio() const
{
    return info.coverSizeRatio;
}

QVariant ComicInfoObject::originalCoverSize() const
{
    return info.originalCoverSize;
}

QVariant ComicInfoObject::added() const
{
    return info.added;
}

QVariant ComicInfoObject::type() const
{
    return info.type;
}

QVariant ComicInfoObject::editor() const
{
    return info.editor;
}

QVariant ComicInfoObject::imprint() const
{
    return info.imprint;
}

QVariant ComicInfoObject::teams() const
{
    return info.teams;
}

QVariant ComicInfoObject::locations() const
{
    return info.locations;
}

QVariant ComicInfoObject::series() const
{
    return info.series;
}

QVariant ComicInfoObject::alternateSeries() const
{
    return info.alternateSeries;
}

QVariant ComicInfoObject::alternateNumber() const
{
    return info.alternateNumber;
}

QVariant ComicInfoObject::alternateCount() const
{
    return info.alternateCount;
}

QVariant ComicInfoObject::languageISO() const
{
    return info.languageISO;
}

QVariant ComicInfoObject::seriesGroup() const
{
    return info.seriesGroup;
}

QVariant ComicInfoObject::mainCharacterOrTeam() const
{
    return info.mainCharacterOrTeam;
}

QVariant ComicInfoObject::review() const
{
    return info.review;
}

QVariant ComicInfoObject::tags() const
{
    return info.tags;
}

QVariant ComicInfoObject::imageFiltersJson() const
{
    return info.imageFiltersJson;
}

QVariant ComicInfoObject::lastTimeImageFiltersSet() const
{
    return info.lastTimeImageFiltersSet;
}

QVariant ComicInfoObject::lastTimeCoverSet() const
{
    return info.lastTimeCoverSet;
}

QVariant ComicInfoObject::usesExternalCover() const
{
    return info.usesExternalCover;
}

QVariant ComicInfoObject::lastTimeMetadataSet() const
{
    return info.lastTimeMetadataSet;
}

bool ComicInfoObject::isFavorite() const
{
    return favorite;
}

void ComicInfoObject::setRead(bool read)
{
    if (read != info.read) {
        info.read = read;
        emit readChanged();
    }
}

void ComicInfoObject::setRating(int rating)
{
    if (rating != info.rating) {
        info.rating = rating;
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
    return info.getWriters();
}

QStringList ComicInfoObject::getPencillers() const
{
    return info.getPencillers();
}

QStringList ComicInfoObject::getInkers() const
{
    return info.getInkers();
}

QStringList ComicInfoObject::getColorists() const
{
    return info.getColorists();
}

QStringList ComicInfoObject::getLetterers() const
{
    return info.getLetterers();
}

QStringList ComicInfoObject::getCoverArtists() const
{
    return info.getCoverArtists();
}

QStringList ComicInfoObject::getEditors() const
{
    return info.getEditors();
}

QStringList ComicInfoObject::getImprint() const
{
    return info.getImprint();
}

QStringList ComicInfoObject::getCharacters() const
{
    return info.getCharacters();
}

QStringList ComicInfoObject::getTeams() const
{
    return info.getTeams();
}

QStringList ComicInfoObject::getLocations() const
{
    return info.getLocations();
}

QStringList ComicInfoObject::getTags() const
{
    return info.getTags();
}

QString ComicInfoObject::getTypeString() const
{
    return info.getTypeString();
}

QString ComicInfoObject::getStoryArcInfoString() const
{
    return info.getStoryArcInfoString();
}

QString ComicInfoObject::getAlternateSeriesString() const
{
    return info.getAlternateSeriesString();
}

QString ComicInfoObject::getTitleOrFileName() const
{
    return titleOrFileName(info, fileName);
}

qulonglong ComicInfoObject::getFileSize() const
{
    // the size is encoded in the hash after the SHA-1
    return info.hash.right(info.hash.length() - 40).toLongLong();
}

QString ComicInfoObject::getTitleIncludingNumber() const
{
    return titleIncludingNumber(info, fileName);
}

QString ComicInfoObject::getInfoTitle() const
{
    return infoTitle(info, fileName);
}

bool ComicInfoObject::available() const
{
    return isAvailable;
}
