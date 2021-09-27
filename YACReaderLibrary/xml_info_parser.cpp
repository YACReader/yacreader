#include "xml_info_parser.h"

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
    auto day = info.date.isNull() ? 1 : info.date.toInt();

    info.date = QString("%1/%2/%3").arg(day).arg(month).arg(year);
}

bool tryValues(QXmlStreamReader &reader, ComicInfo &info)
{
    std::map<QString, QVariant &> stringValues = {
        { "Title", info.title },
        { "Volume", info.volume },
        { "StoryArc", info.storyArc },
        { "Genre", info.genere },
        { "Publisher", info.publisher },
        { "Format", info.format },
        { "AgeRating", info.ageRating },
        { "Summary", info.synopsis },
        { "Notes", info.notes },
    };

    std::map<QString, QVariant &> forcedNumbers = {
        { "Number", info.number },
        { "Count", info.count },
        { "AlternateNumber", info.arcNumber },
        { "AlternateCount", info.arcCount },
        { "Day", info.day },
        { "Month", info.month },
        { "Year", info.year }
    };

    std::map<QString, QVariant &> multiValuedStrings = {
        { "Writer", info.writer },
        { "Penciller", info.penciller },
        { "Inker", info.inker },
        { "Colorist", info.colorist },
        { "Letterer", info.letterer },
        { "CoverArtist", info.coverArtist },
        { "Characters", info.characters }
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

    if (reader.name() == "BlackAndWhite") {
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

    if (reader.name() == "Manga") {
        auto string = reader.readElementText();
        if (isValidText(string)) {
            if (string == "Yes" || string == "YesAndRightToLeft") {
                info.manga = true;
            } else if (string == "No") {
                info.manga = false;
            }
        }

        return true;
    }

    if (reader.name() == "Web") {
        auto string = reader.readElementText();
        if (isValidText(string)) {
            auto comicVineId = string.split("-").last().replace("/", "");

            info.comicVineID = comicVineId;
        }

        return true;
    }

    return false;
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
            someDataWasParsed = true;
        } else {
            if (reader.name() != "ComicInfo") {
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
