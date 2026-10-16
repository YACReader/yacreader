#include "descriptive_comic_metadata.h"

#include <QSqlQuery>
#include <QSqlRecord>

const std::array<DescriptiveComicMetadata::Column, 37> &DescriptiveComicMetadata::columns()
{
    using M = DescriptiveComicMetadata;
    static const std::array<Column, 37> columns = { {
            { "title", "TEXT", &M::title },
            { "number", "TEXT", &M::number },
            { "isBis", "BOOLEAN", &M::isBis },
            { "count", "INTEGER", &M::count },
            { "volume", "TEXT", &M::volume },
            { "storyArc", "TEXT", &M::storyArc },
            { "arcNumber", "TEXT", &M::arcNumber },
            { "arcCount", "INTEGER", &M::arcCount },
            { "genere", "TEXT", &M::genere },
            { "writer", "TEXT", &M::writer },
            { "penciller", "TEXT", &M::penciller },
            { "inker", "TEXT", &M::inker },
            { "colorist", "TEXT", &M::colorist },
            { "letterer", "TEXT", &M::letterer },
            { "coverArtist", "TEXT", &M::coverArtist },
            { "date", "TEXT", &M::date },
            { "publisher", "TEXT", &M::publisher },
            { "format", "TEXT", &M::format },
            { "color", "BOOLEAN", &M::color },
            { "ageRating", "TEXT", &M::ageRating },
            { "synopsis", "TEXT", &M::synopsis },
            { "characters", "TEXT", &M::characters },
            { "notes", "TEXT", &M::notes },
            { "comicVineID", "TEXT", &M::comicVineID },
            { "editor", "TEXT", &M::editor },
            { "imprint", "TEXT", &M::imprint },
            { "teams", "TEXT", &M::teams },
            { "locations", "TEXT", &M::locations },
            { "series", "TEXT", &M::series },
            { "alternateSeries", "TEXT", &M::alternateSeries },
            { "alternateNumber", "TEXT", &M::alternateNumber },
            { "alternateCount", "INTEGER", &M::alternateCount },
            { "languageISO", "TEXT", &M::languageISO },
            { "seriesGroup", "TEXT", &M::seriesGroup },
            { "mainCharacterOrTeam", "TEXT", &M::mainCharacterOrTeam },
            { "review", "TEXT", &M::review },
            { "tags", "TEXT", &M::tags },
    } };
    return columns;
}

QStringList DescriptiveComicMetadata::assignments()
{
    QStringList result;
    for (const auto &column : columns()) {
        const auto name = QString::fromLatin1(column.name);
        result.append(name + " = :" + name);
    }
    return result;
}

void DescriptiveComicMetadata::bindTo(QSqlQuery &query) const
{
    for (const auto &column : columns()) {
        query.bindValue(":" + QString::fromLatin1(column.name), this->*column.member);
    }
}
