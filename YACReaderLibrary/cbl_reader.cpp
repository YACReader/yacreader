#include "cbl_reader.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringDecoder>
#include <QXmlStreamReader>

#include <cmath>

namespace {

std::optional<int> toInt(const QString &text)
{
    bool ok = false;
    const auto value = text.trimmed().toInt(&ok);
    return ok ? std::optional<int>(value) : std::nullopt;
}

// XML

QString attribute(const QXmlStreamAttributes &attributes, QLatin1StringView name)
{
    return attributes.value(name).toString().trimmed();
}

CblBook readXmlBook(QXmlStreamReader &xml)
{
    const auto attributes = xml.attributes();
    CblBook book;
    book.series = attribute(attributes, QLatin1StringView("Series"));
    book.number = attribute(attributes, QLatin1StringView("Number"));
    book.volume = attribute(attributes, QLatin1StringView("Volume"));
    book.year = attribute(attributes, QLatin1StringView("Year"));
    book.format = attribute(attributes, QLatin1StringView("Format"));

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1StringView("Database")) {
            const auto databaseAttributes = xml.attributes();
            book.databases.append({ attribute(databaseAttributes, QLatin1StringView("Name")),
                                    attribute(databaseAttributes, QLatin1StringView("Series")),
                                    attribute(databaseAttributes, QLatin1StringView("Issue")) });
            xml.skipCurrentElement();
        } else if (xml.name() == QLatin1StringView("Id")) {
            book.id = xml.readElementText().trimmed();
        } else if (xml.name() == QLatin1StringView("FileName")) {
            book.fileName = xml.readElementText().trimmed();
        } else {
            xml.skipCurrentElement();
        }
    }
    return book;
}

// the text of an XML document, in the encoding of its byte order mark or declaration, else UTF-8
QString xmlText(const QByteArray &data)
{
    if (data.startsWith("\xFF\xFE") || data.startsWith("\xFE\xFF"))
        return QStringDecoder(QStringConverter::Utf16)(data);

    static const QRegularExpression declaration(R"(^\s*<\?xml[^>]*encoding\s*=\s*["']([^"']+)["'])");
    const auto match = declaration.match(QString::fromLatin1(data.left(200)));
    if (match.hasMatch()) {
        QStringDecoder decoder(match.captured(1).toLatin1().constData());
        if (decoder.isValid())
            return decoder(data);
    }
    return QString::fromUtf8(data);
}

CblReadResult readXml(const QString &text)
{
    QXmlStreamReader xml(text);
    CblReadingList list;
    list.format = CblReadingList::Xml;

    if (xml.readNextStartElement() && xml.name() != QLatin1StringView("ReadingList"))
        return CblReadError { CblReadError::NotAReadingList, xml.name().toString(), xml.lineNumber(), xml.columnNumber() };

    while (xml.readNextStartElement()) {
        const auto name = xml.name();
        if (name == QLatin1StringView("Name")) {
            list.name = xml.readElementText().trimmed();
        } else if (name == QLatin1StringView("Summary")) {
            list.description = xml.readElementText().trimmed();
        } else if (name == QLatin1StringView("NumIssues")) {
            list.sourceCount = toInt(xml.readElementText());
        } else if (name == QLatin1StringView("StartYear")) {
            list.startYear = toInt(xml.readElementText());
        } else if (name == QLatin1StringView("StartMonth")) {
            list.startMonth = toInt(xml.readElementText());
        } else if (name == QLatin1StringView("EndYear")) {
            list.endYear = toInt(xml.readElementText());
        } else if (name == QLatin1StringView("EndMonth")) {
            list.endMonth = toInt(xml.readElementText());
        } else if (name == QLatin1StringView("Books")) {
            // the text from the end of the previous element to the end of a book is that book
            auto start = xml.characterOffset();
            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1StringView("Book")) {
                    auto book = readXmlBook(xml);
                    book.source = text.mid(start, xml.characterOffset() - start).trimmed();
                    list.books.append(book);
                } else {
                    xml.skipCurrentElement();
                }
                start = xml.characterOffset();
            }
        } else {
            xml.skipCurrentElement();
        }
    }

    // reading on to the end finds errors after the reading list too
    while (!xml.atEnd() && !xml.hasError())
        xml.readNext();

    if (xml.hasError())
        return CblReadError { CblReadError::Malformed, xml.errorString(), xml.lineNumber(), xml.columnNumber() };

    return list;
}

// JSON

// strings as written; whole numbers without a fraction (seriesStartYear is a number)
QString text(const QJsonValue &value)
{
    if (value.isString())
        return value.toString().trimmed();
    if (value.isDouble()) {
        const auto number = value.toDouble();
        return number == std::floor(number) ? QString::number(static_cast<qint64>(number)) : QString::number(number);
    }
    return { };
}

std::optional<int> integer(const QJsonValue &value)
{
    if (value.isDouble()) {
        const auto number = value.toDouble();
        return number == std::floor(number) ? std::optional<int>(static_cast<int>(number)) : std::nullopt;
    }
    if (value.isString())
        return toInt(value.toString());
    return std::nullopt;
}

QStringList texts(const QJsonValue &value)
{
    QStringList result;
    for (const auto &item : value.toArray()) {
        if (const auto itemText = text(item); !itemText.isEmpty())
            result.append(itemText);
    }
    return result;
}

CblBook readJsonBook(const QJsonObject &issue)
{
    CblBook book;
    book.series = text(issue.value(QLatin1StringView("seriesName")));
    book.number = text(issue.value(QLatin1StringView("issueNumber")));
    book.volume = text(issue.value(QLatin1StringView("seriesStartYear")));
    book.coverDate = text(issue.value(QLatin1StringView("issueCoverDate")));
    book.year = book.coverDate.section('-', 0, 0);
    book.issueType = text(issue.value(QLatin1StringView("issueType")));
    for (const auto &id : issue.value(QLatin1StringView("id")).toArray()) {
        const auto ids = id.toObject();
        book.databases.append({ text(ids.value(QLatin1StringView("name"))),
                                text(ids.value(QLatin1StringView("series"))),
                                text(ids.value(QLatin1StringView("issue"))) });
    }
    return book;
}

CblReadResult readJson(const QByteArray &data)
{
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        const auto before = data.left(parseError.offset);
        const auto line = before.count('\n') + 1;
        const auto column = parseError.offset - (before.lastIndexOf('\n') + 1) + 1;
        return CblReadError { CblReadError::Malformed, parseError.errorString(), line, column };
    }

    const auto root = document.object();
    if (!document.isObject() || !root.value(QLatin1StringView("issueList")).isArray())
        return CblReadError { CblReadError::NotAReadingList, QStringLiteral("no issueList") };

    CblReadingList list;
    list.format = CblReadingList::Json;
    list.uuid = text(root.value(QLatin1StringView("fileDetails")).toObject().value(QLatin1StringView("UUID")));
    list.notes = text(root.value(QLatin1StringView("notes")));

    const auto details = root.value(QLatin1StringView("listDetails")).toObject();
    list.name = text(details.value(QLatin1StringView("name")));
    list.description = text(details.value(QLatin1StringView("description")));
    list.startYear = integer(details.value(QLatin1StringView("startYear")));
    list.endYear = integer(details.value(QLatin1StringView("endYear")));
    list.publisher = text(details.value(QLatin1StringView("publisher")));
    list.imprint = text(details.value(QLatin1StringView("imprint")));
    list.type = text(details.value(QLatin1StringView("type")));
    list.tags = texts(details.value(QLatin1StringView("tags")));
    list.coverImageUrls = texts(details.value(QLatin1StringView("coverImageURLs")));
    for (const auto &item : details.value(QLatin1StringView("relationships")).toArray()) {
        const auto relationship = item.toObject();
        list.relationships.append({ text(relationship.value(QLatin1StringView("name"))),
                                    text(relationship.value(QLatin1StringView("UUID"))),
                                    text(relationship.value(QLatin1StringView("relationship"))) });
    }
    for (const auto &item : details.value(QLatin1StringView("source")).toArray()) {
        const auto source = item.toObject();
        list.sources.append({ text(source.value(QLatin1StringView("name"))),
                              text(source.value(QLatin1StringView("url"))),
                              text(source.value(QLatin1StringView("id"))) });
    }

    const auto issues = root.value(QLatin1StringView("issueList")).toArray();
    for (qsizetype i = 0; i < issues.size(); i++) {
        if (!issues.at(i).isObject())
            return CblReadError { CblReadError::Malformed, QStringLiteral("issueList item %1 is not an object").arg(i) };
        auto book = readJsonBook(issues.at(i).toObject());
        book.source = QString::fromUtf8(QJsonDocument(issues.at(i).toObject()).toJson(QJsonDocument::Compact));
        list.books.append(book);
    }

    return list;
}

}

CblReadResult readCbl(QIODevice &device)
{
    auto data = device.readAll();
    if (data.startsWith("\xEF\xBB\xBF"))
        data.remove(0, 3);

    const auto first = std::find_if(data.cbegin(), data.cend(), [](char c) { return !QChar::isSpace(uchar(c)); });
    if (first != data.cend() && *first == '{')
        return readJson(data);
    // the reader gets text, so the offsets of a book are positions in it
    return readXml(xmlText(data));
}

CblReadResult readCbl(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return CblReadError { CblReadError::CannotOpen, file.errorString() };

    return readCbl(file);
}
