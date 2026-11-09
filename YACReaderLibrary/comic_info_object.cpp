#include "comic_info_object.h"

#include "overloaded.h"

#include <QFileInfo>

ComicInfoObject::ComicInfoObject(QObject *parent)
    : QObject(parent), favorite(false)
{
}

ComicInfoObject::ComicInfoObject(const ComicRecord &record, bool isFavorite, QObject *parent)
    : QObject(parent), record(record), fileName(recordFileName(record)), isAvailable(recordComic(record) != nullptr), favorite(isFavorite)
{
}

qulonglong ComicInfoObject::id() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->id.value();
}

bool ComicInfoObject::read() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->read;
}

bool ComicInfoObject::edited() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->edited;
}

QString ComicInfoObject::hash() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->hash;
}

int ComicInfoObject::rating() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->rating;
}

bool ComicInfoObject::hasBeenOpened() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->hasBeenOpened;
}

int ComicInfoObject::currentPage() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->currentPage;
}

int ComicInfoObject::bookmark1() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->bookmark1;
}

int ComicInfoObject::bookmark2() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->bookmark2;
}

int ComicInfoObject::bookmark3() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->bookmark3;
}

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
int ComicInfoObject::brightness() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->brightness;
}

int ComicInfoObject::contrast() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->contrast;
}

int ComicInfoObject::gamma() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->gamma;
}
QT_WARNING_POP

QVariant ComicInfoObject::title() const
{
    return recordMetadata(record).title;
}

QVariant ComicInfoObject::coverPage() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->coverPage;
}

QVariant ComicInfoObject::numPages() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->numPages;
}

QVariant ComicInfoObject::number() const
{
    return recordMetadata(record).number;
}

QVariant ComicInfoObject::isBis() const
{
    return recordMetadata(record).isBis;
}

QVariant ComicInfoObject::count() const
{
    return recordMetadata(record).count;
}

QVariant ComicInfoObject::volume() const
{
    return recordMetadata(record).volume;
}

QVariant ComicInfoObject::storyArc() const
{
    return recordMetadata(record).storyArc;
}

QVariant ComicInfoObject::arcNumber() const
{
    return recordMetadata(record).arcNumber;
}

QVariant ComicInfoObject::arcCount() const
{
    return recordMetadata(record).arcCount;
}

QVariant ComicInfoObject::genere() const
{
    return recordMetadata(record).genere;
}

QVariant ComicInfoObject::writer() const
{
    return recordMetadata(record).writer;
}

QVariant ComicInfoObject::penciller() const
{
    return recordMetadata(record).penciller;
}

QVariant ComicInfoObject::inker() const
{
    return recordMetadata(record).inker;
}

QVariant ComicInfoObject::colorist() const
{
    return recordMetadata(record).colorist;
}

QVariant ComicInfoObject::letterer() const
{
    return recordMetadata(record).letterer;
}

QVariant ComicInfoObject::coverArtist() const
{
    return recordMetadata(record).coverArtist;
}

QVariant ComicInfoObject::date() const
{
    return recordMetadata(record).date;
}

QVariant ComicInfoObject::publisher() const
{
    return recordMetadata(record).publisher;
}

QVariant ComicInfoObject::format() const
{
    return recordMetadata(record).format;
}

QVariant ComicInfoObject::color() const
{
    return recordMetadata(record).color;
}

QVariant ComicInfoObject::ageRating() const
{
    return recordMetadata(record).ageRating;
}

QVariant ComicInfoObject::synopsis() const
{
    return recordMetadata(record).synopsis;
}

QVariant ComicInfoObject::characters() const
{
    return recordMetadata(record).characters;
}

QVariant ComicInfoObject::notes() const
{
    return recordMetadata(record).notes;
}

QVariant ComicInfoObject::comicVineID() const
{
    return recordMetadata(record).comicVineID;
}

QVariant ComicInfoObject::lastTimeOpened() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->lastTimeOpened;
}

QVariant ComicInfoObject::coverSizeRatio() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->coverSizeRatio;
}

QVariant ComicInfoObject::originalCoverSize() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->originalCoverSize;
}

QVariant ComicInfoObject::added() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->added;
}

QVariant ComicInfoObject::type() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->type;
}

QVariant ComicInfoObject::editor() const
{
    return recordMetadata(record).editor;
}

QVariant ComicInfoObject::imprint() const
{
    return recordMetadata(record).imprint;
}

QVariant ComicInfoObject::teams() const
{
    return recordMetadata(record).teams;
}

QVariant ComicInfoObject::locations() const
{
    return recordMetadata(record).locations;
}

QVariant ComicInfoObject::series() const
{
    return recordMetadata(record).series;
}

QVariant ComicInfoObject::alternateSeries() const
{
    return recordMetadata(record).alternateSeries;
}

QVariant ComicInfoObject::alternateNumber() const
{
    return recordMetadata(record).alternateNumber;
}

QVariant ComicInfoObject::alternateCount() const
{
    return recordMetadata(record).alternateCount;
}

QVariant ComicInfoObject::languageISO() const
{
    return recordMetadata(record).languageISO;
}

QVariant ComicInfoObject::seriesGroup() const
{
    return recordMetadata(record).seriesGroup;
}

QVariant ComicInfoObject::mainCharacterOrTeam() const
{
    return recordMetadata(record).mainCharacterOrTeam;
}

QVariant ComicInfoObject::review() const
{
    return recordMetadata(record).review;
}

QVariant ComicInfoObject::tags() const
{
    return recordMetadata(record).tags;
}

QVariant ComicInfoObject::imageFiltersJson() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->imageFiltersJson;
}

QVariant ComicInfoObject::lastTimeImageFiltersSet() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->lastTimeImageFiltersSet;
}

QVariant ComicInfoObject::lastTimeCoverSet() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->lastTimeCoverSet;
}

QVariant ComicInfoObject::usesExternalCover() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->usesExternalCover;
}

QVariant ComicInfoObject::lastTimeMetadataSet() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->lastTimeMetadataSet;
}

bool ComicInfoObject::isFavorite() const
{
    return favorite;
}

void ComicInfoObject::setRead(bool read)
{
    auto *info = recordInfo(record);
    if (info != nullptr && read != info->read) {
        info->read = read;
        emit readChanged();
    }
}

void ComicInfoObject::setRating(int rating)
{
    auto *info = recordInfo(record);
    if (info != nullptr && rating != info->rating) {
        info->rating = rating;
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
    return recordMetadata(record).getWriters();
}

QStringList ComicInfoObject::getPencillers() const
{
    return recordMetadata(record).getPencillers();
}

QStringList ComicInfoObject::getInkers() const
{
    return recordMetadata(record).getInkers();
}

QStringList ComicInfoObject::getColorists() const
{
    return recordMetadata(record).getColorists();
}

QStringList ComicInfoObject::getLetterers() const
{
    return recordMetadata(record).getLetterers();
}

QStringList ComicInfoObject::getCoverArtists() const
{
    return recordMetadata(record).getCoverArtists();
}

QStringList ComicInfoObject::getEditors() const
{
    return recordMetadata(record).getEditors();
}

QStringList ComicInfoObject::getImprint() const
{
    return recordMetadata(record).getImprint();
}

QStringList ComicInfoObject::getCharacters() const
{
    return recordMetadata(record).getCharacters();
}

QStringList ComicInfoObject::getTeams() const
{
    return recordMetadata(record).getTeams();
}

QStringList ComicInfoObject::getLocations() const
{
    return recordMetadata(record).getLocations();
}

QStringList ComicInfoObject::getTags() const
{
    return recordMetadata(record).getTags();
}

QString ComicInfoObject::getTypeString() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    return info->getTypeString();
}

QString ComicInfoObject::getStoryArcInfoString() const
{
    return recordMetadata(record).getStoryArcInfoString();
}

QString ComicInfoObject::getAlternateSeriesString() const
{
    return recordMetadata(record).getAlternateSeriesString();
}

QString ComicInfoObject::getTitleOrFileName() const
{
    return orDash(titleOrFileName(recordMetadata(record), fileName));
}

qulonglong ComicInfoObject::getFileSize() const
{
    const auto *info = recordInfo(record);
    if (info == nullptr)
        return { };
    // the size is encoded in the hash after the SHA-1
    return info->hash.right(info->hash.length() - 40).toLongLong();
}

QString ComicInfoObject::getTitleIncludingNumber() const
{
    return orDash(titleIncludingNumber(recordMetadata(record), fileName));
}

QString ComicInfoObject::getInfoTitle() const
{
    return orDash(infoTitle(recordMetadata(record), fileName));
}

// Display fallback only; never stored in the comic's metadata.
QString ComicInfoObject::orDash(const QString &title)
{
    return title.isEmpty() ? QStringLiteral("-") : title;
}

bool ComicInfoObject::available() const
{
    return isAvailable;
}

bool ComicInfoObject::canSetReadState() const
{
    return recordInfo(record) != nullptr;
}
