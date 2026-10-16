#ifndef DESCRIPTIVE_COMIC_METADATA_H
#define DESCRIPTIVE_COMIC_METADATA_H

#include <QStringList>
#include <QVariant>

#include <array>

class QSqlQuery;
class QSqlRecord;

struct DescriptiveComicMetadata {
    QVariant title; // string
    QVariant number; // string
    QVariant isBis; // bool
    QVariant count; // int
    QVariant volume; // string
    QVariant storyArc; // string
    QVariant arcNumber; // string
    QVariant arcCount; // int
    QVariant genere; // string
    QVariant writer; // string
    QVariant penciller; // string
    QVariant inker; // string
    QVariant colorist; // string
    QVariant letterer; // string
    QVariant coverArtist; // string
    QVariant date; // string, dd/mm/yyyy, parts can be empty
    QVariant publisher; // string
    QVariant format; // string
    QVariant color; // bool
    QVariant ageRating; // string
    QVariant synopsis; // string
    QVariant characters; // string
    QVariant notes; // string
    QVariant comicVineID; // string
    QVariant editor; // string
    QVariant imprint; // string
    QVariant teams; // string/list
    QVariant locations; // string/list
    QVariant series; // string
    QVariant alternateSeries; // string
    QVariant alternateNumber; // string
    QVariant alternateCount; // int
    QVariant languageISO; // string
    QVariant seriesGroup; // string
    QVariant mainCharacterOrTeam; // string
    QVariant review; // string
    QVariant tags; // string/list

    struct Column {
        const char *name;
        const char *sqlType;
        QVariant DescriptiveComicMetadata::*member;
    };

    static const std::array<Column, 37> &columns();

    static QStringList assignments();

    void bindTo(QSqlQuery &query) const;
};

#endif
