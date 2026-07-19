#include "xml_info_parser.h"

#include "yacreader_global.h"

#include <QtCore>

bool isValidText(const QString &string)
{
    return !string.isEmpty() && !string.isNull();
}

QString transLateMultiValuedString(QString &string, const QString &originSeparator = ", ", const QString &targetSeparator = "\n")
{
    return string.replace(originSeparator, targetSeparator);
}

bool parseField(QXmlStreamReader &reader, const QString &xmlName, QVariant &dest, bool multivalued = false)
{
    auto name = reader.name();
    if (name == xmlName) {
        auto string = reader.readElementText();
        if (isValidText(string)) {
            if (multivalued) {
                dest = transLateMultiValuedString(string);
            } else {
                dest = string;
            }
        }

        return true;
    }

    return false;
}

bool parseFieldNumber(QXmlStreamReader &reader, const QString &xmlName, QVariant &dest)
{
    if (reader.name() == xmlName) {
        auto string = reader.readElementText();
        if (isValidText(string)) {
            bool success;
            auto number = string.toInt(&success);
            if (success) {
                dest = number;
            }
        }

        return true;
    }

    return false;
}

void consolidateDate(ComicInfo &info)
{
    if (!info.year.isValid() && !info.month.isValid() && !info.day.isValid()) {
        return;
    }

    auto year = info.year.isNull() ? 0 : info.year.toInt();
    auto month = info.month.isNull() ? 1 : info.month.toInt();
    auto day = info.day.isNull() ? 1 : info.day.toInt();

    info.date = QString("%1/%2/%3").arg(day).arg(month).arg(year);
}

bool tryValues(QXmlStreamReader &reader, ComicInfo &info)
{
    std::map<QString, QVariant &> stringValues = {
        { "Number", info.number },
        { "Title", info.title },
        { "Volume", info.volume },
        { "StoryArc", info.storyArc },
        { "Genre", info.genere },
        { "Publisher", info.publisher },
        { "Format", info.format },
        { "AgeRating", info.ageRating },
        { "Summary", info.synopsis },
        { "Notes", info.notes },
        { "Editor", info.editor },
        { "Imprint", info.imprint },
        { "Series", info.series },
        { "AlternateSeries", info.alternateSeries },
        { "AlternateNumber", info.alternateNumber },
        { "LanguageISO", info.languageISO },
        { "SeriesGroup", info.seriesGroup },
        { "MainCharacterOrTeam", info.mainCharacterOrTeam },
        { "Review", info.review },
    };

    std::map<QString, QVariant &> forcedNumbers = {
        { "Count", info.count },
        { "AlternateCount", info.alternateCount },
        { "Day", info.day },
        { "Month", info.month },
        { "Year", info.year },
    };

    std::map<QString, QVariant &> multiValuedStrings = {
        { "Writer", info.writer },
        { "Penciller", info.penciller },
        { "Inker", info.inker },
        { "Colorist", info.colorist },
        { "Letterer", info.letterer },
        { "CoverArtist", info.coverArtist },
        { "Characters", info.characters },
        { "Teams", info.teams },
        { "Locations", info.locations }
    };

    for (auto &pair : stringValues) {
        if (parseField(reader, pair.first, pair.second)) {
            return true;
        }
    }

    for (auto &pair : forcedNumbers) {
        if (parseFieldNumber(reader, pair.first, pair.second)) {
            return true;
        }
    }

    for (auto &pair : multiValuedStrings) {
        if (parseField(reader, pair.first, pair.second, true)) {
            return true;
        }
    }

    if (reader.name() == QString("BlackAndWhite")) {
        auto string = reader.readElementText();
        if (isValidText(string)) {
            if (string == "Yes") {
                info.color = false;
            } else if (string == "No") {
                info.color = true;
            }
        }

        return true;
    }

    if (reader.name() == QString("Manga")) {
        auto string = reader.readElementText();
        if (isValidText(string)) {
            if (string == "Yes" || string == "YesAndRightToLeft") { // there was a breaking change in ComicInfo 2.0, Yes means now WesterManga reading style, but old info stills means manga
                info.type = QVariant::fromValue(YACReader::FileType::Manga);
            } else if (string == "No") {
                info.type = QVariant::fromValue(YACReader::FileType::Comic);
            }
        }

        return true;
    }

    // TODO, check if the url is actually a comic vine link
    if (reader.name() == QString("Web")) {
        auto string = reader.readElementText();
        if (isValidText(string)) {
            auto comicVineId = string.split("-").last().replace("/", "");

            info.comicVineID = comicVineId;
        }

        return true;
    }

    return false;
}

const QList<YACReader::XmlMetadataField> &YACReader::xmlMetadataFields()
{
    static const QList<XmlMetadataField> fields = {
        { "title", &ComicInfo::title },
        { "number", &ComicInfo::number },
        { "count", &ComicInfo::count },
        { "volume", &ComicInfo::volume },
        { "storyArc", &ComicInfo::storyArc },
        { "genere", &ComicInfo::genere },
        { "writer", &ComicInfo::writer },
        { "penciller", &ComicInfo::penciller },
        { "inker", &ComicInfo::inker },
        { "colorist", &ComicInfo::colorist },
        { "letterer", &ComicInfo::letterer },
        { "coverArtist", &ComicInfo::coverArtist },
        { "date", &ComicInfo::date },
        { "publisher", &ComicInfo::publisher },
        { "format", &ComicInfo::format },
        { "color", &ComicInfo::color },
        { "ageRating", &ComicInfo::ageRating },
        { "synopsis", &ComicInfo::synopsis },
        { "characters", &ComicInfo::characters },
        { "notes", &ComicInfo::notes },
        { "comicVineID", &ComicInfo::comicVineID },
        { "type", &ComicInfo::type },
        { "editor", &ComicInfo::editor },
        { "imprint", &ComicInfo::imprint },
        { "teams", &ComicInfo::teams },
        { "locations", &ComicInfo::locations },
        { "series", &ComicInfo::series },
        { "alternateSeries", &ComicInfo::alternateSeries },
        { "alternateNumber", &ComicInfo::alternateNumber },
        { "alternateCount", &ComicInfo::alternateCount },
        { "languageISO", &ComicInfo::languageISO },
        { "seriesGroup", &ComicInfo::seriesGroup },
        { "mainCharacterOrTeam", &ComicInfo::mainCharacterOrTeam },
        { "review", &ComicInfo::review },
    };

    return fields;
}

bool YACReader::parseXMLIntoInfo(const QByteArray &xmlRawData, ComicInfo &info)
{
    if (xmlRawData.isEmpty()) {
        return false;
    }

    QXmlStreamReader reader(xmlRawData);

    bool someDataWasParsed = false;

    while (reader.readNextStartElement()) {
        if (tryValues(reader, info)) {
            someDataWasParsed = true | someDataWasParsed;
        } else {
            if (reader.name() != QString("ComicInfo")) {
                reader.skipCurrentElement();
            }
        }
    }

    consolidateDate(info);

    if (reader.error()) {
        return false;
    }

    return someDataWasParsed;
}
