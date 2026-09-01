#include "cbl_reader.h"

#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>

CblReadResult CblReader::read(const QString &filePath)
{
    CblReadResult result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = file.errorString();
        return result;
    }

    QXmlStreamReader xml(&file);
    bool foundReadingList = false;
    int ordering = 0;

    while (!xml.atEnd()) {
        xml.readNext();

        if (!xml.isStartElement())
            continue;

        if (!foundReadingList) {
            if (xml.name() != QLatin1String("ReadingList")) {
                result.errorMessage = QStringLiteral("The selected file is not a CBL reading list.");
                return result;
            }
            foundReadingList = true;
            continue;
        }

        if (xml.name() == QLatin1String("Name")) {
            result.readingList.name = xml.readElementText().trimmed();
        } else if (xml.name() == QLatin1String("Book")) {
            const auto attributes = xml.attributes();

            CblBook book;
            book.series = attributes.value(QLatin1String("Series")).toString().trimmed();
            book.number = attributes.value(QLatin1String("Number")).toString().trimmed();
            book.volume = attributes.value(QLatin1String("Volume")).toString().trimmed();
            book.year = attributes.value(QLatin1String("Year")).toString().trimmed();
            book.ordering = ordering++;

            while (!(xml.isEndElement() && xml.name() == QLatin1String("Book")) && !xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == QLatin1String("Id"))
                    book.id = xml.readElementText().trimmed();
            }

            result.readingList.books.append(book);
        }
    }

    if (xml.hasError()) {
        result.errorMessage = xml.errorString();
        result.errorLine = xml.lineNumber();
        result.errorColumn = xml.columnNumber();
        return result;
    }

    if (!foundReadingList) {
        result.errorMessage = QStringLiteral("The selected file does not contain a ReadingList element.");
        return result;
    }

    if (result.readingList.name.isEmpty())
        result.readingList.name = QFileInfo(filePath).completeBaseName();

    result.success = true;
    return result;
}
