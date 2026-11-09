#include "cbl_writer.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <initializer_list>

namespace {

bool isProvider(const QString &name, std::initializer_list<QLatin1StringView> aliases)
{
    return std::any_of(aliases.begin(), aliases.end(), [&](QLatin1StringView alias) { return name.compare(alias, Qt::CaseInsensitive) == 0; });
}

// the names of json-cbl-standard 1.0; empty for a provider it has no name for
QString jsonProviderName(const QString &name)
{
    if (isProvider(name, { QLatin1StringView("cv"), QLatin1StringView("comicvine") }))
        return QStringLiteral("comicvine");
    if (isProvider(name, { QLatin1StringView("gcd"), QLatin1StringView("grandcomicsdatabase") }))
        return QStringLiteral("grandComicsDatabase");
    if (isProvider(name, { QLatin1StringView("metron") }))
        return QStringLiteral("metron");
    return { };
}

// ComicRack's name for Comic Vine
QString xmlProviderName(const QString &name)
{
    return isProvider(name, { QLatin1StringView("cv"), QLatin1StringView("comicvine") }) ? QStringLiteral("cv") : name;
}

QString attribute(const char *name, const QString &value)
{
    return value.isEmpty() ? QString() : QStringLiteral(" %1=\"%2\"").arg(QLatin1StringView(name), value.toHtmlEscaped());
}

QString xmlBook(const CblBook &book)
{
    const auto year = book.year.isEmpty() ? book.coverDate.section('-', 0, 0) : book.year;
    QString children;
    if (!book.id.isEmpty())
        children += QStringLiteral("      <Id>%1</Id>\n").arg(book.id.toHtmlEscaped());
    for (const auto &ids : book.databases)
        children += QStringLiteral("      <Database%1%2%3 />\n").arg(attribute("Name", xmlProviderName(ids.name)), attribute("Series", ids.seriesId), attribute("Issue", ids.issueId));
    if (!book.fileName.isEmpty())
        children += QStringLiteral("      <FileName>%1</FileName>\n").arg(book.fileName.toHtmlEscaped());

    const auto start = QStringLiteral("    <Book%1%2%3%4%5")
                               .arg(attribute("Series", book.series), attribute("Number", book.number), attribute("Volume", book.volume), attribute("Year", year), attribute("Format", book.format));
    return children.isEmpty() ? start + QStringLiteral(" />\n") : start + QStringLiteral(">\n") + children + QStringLiteral("    </Book>\n");
}

QByteArray writeXml(const QString &name, const QList<CblExportBook> &books)
{
    QString text = QStringLiteral("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                                  "<ReadingList xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
    text += QStringLiteral("  <Name>%1</Name>\n").arg(name.toHtmlEscaped());
    text += QStringLiteral("  <NumIssues>%1</NumIssues>\n").arg(books.size());
    text += QStringLiteral("  <Books>\n");
    for (const auto &book : books) {
        if (book.sourceFormat == QLatin1StringView("xml") && !book.source.isEmpty())
            text += QStringLiteral("    ") + book.source.trimmed() + QStringLiteral("\n");
        else
            text += xmlBook(book.book);
    }
    text += QStringLiteral("  </Books>\n</ReadingList>\n");
    return text.toUtf8();
}

QJsonObject jsonBook(const CblBook &book)
{
    QJsonObject issue;
    if (!book.series.isEmpty())
        issue.insert(QStringLiteral("seriesName"), book.series);
    bool isYear = false;
    if (const auto startYear = book.volume.toInt(&isYear); isYear && startYear >= 1900 && startYear <= 2100)
        issue.insert(QStringLiteral("seriesStartYear"), startYear);
    if (!book.number.isEmpty())
        issue.insert(QStringLiteral("issueNumber"), book.number);
    if (!book.coverDate.isEmpty())
        issue.insert(QStringLiteral("issueCoverDate"), book.coverDate);
    if (!book.issueType.isEmpty())
        issue.insert(QStringLiteral("issueType"), book.issueType);

    QJsonArray ids;
    for (const auto &databaseIds : book.databases) {
        if (const auto provider = jsonProviderName(databaseIds.name); !provider.isEmpty())
            ids.append(QJsonObject { { QStringLiteral("name"), provider }, { QStringLiteral("series"), databaseIds.seriesId }, { QStringLiteral("issue"), databaseIds.issueId } });
    }
    if (!ids.isEmpty())
        issue.insert(QStringLiteral("id"), ids);
    return issue;
}

QByteArray writeJson(const QString &name, const QList<CblExportBook> &books, const QString &uuid)
{
    QJsonArray issues;
    for (const auto &book : books) {
        const auto source = book.sourceFormat == QLatin1StringView("json") ? QJsonDocument::fromJson(book.source.toUtf8()) : QJsonDocument();
        issues.append(source.isObject() ? source.object() : jsonBook(book.book));
    }

    const QJsonObject root {
        { QStringLiteral("fileDetails"), QJsonObject { { QStringLiteral("version"), 1.0 }, { QStringLiteral("UUID"), uuid } } },
        { QStringLiteral("listDetails"), QJsonObject { { QStringLiteral("name"), name } } },
        { QStringLiteral("issueList"), issues },
    };
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

}

QByteArray writeCbl(const QString &name, const QList<CblExportBook> &books, CblReadingList::Format format, const QString &uuid)
{
    return format == CblReadingList::Json ? writeJson(name, books, uuid) : writeXml(name, books);
}
