#include "cbl_importer.h"

#include "QsLog.h"
#include "db_helper.h"
#include "overloaded.h"

#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace {

QVariant text(const QString &value)
{
    const auto trimmed = value.trimmed();
    return trimmed.isEmpty() ? QVariant() : QVariant(trimmed);
}

QString comicVineIssueId(const CblBook &book)
{
    static const QRegularExpression digits(R"(^\d+$)");
    for (const auto &ids : book.databases) {
        const bool comicVine = ids.name.compare(QLatin1StringView("cv"), Qt::CaseInsensitive) == 0 || ids.name.compare(QLatin1StringView("comicvine"), Qt::CaseInsensitive) == 0;
        if (comicVine && digits.match(ids.issueId.trimmed()).hasMatch())
            return ids.issueId.trimmed();
    }
    return { };
}

// dd/mm/yyyy with the parts that are known; a year alone never gets a day or month
QVariant date(const CblBook &book)
{
    const auto parts = book.coverDate.trimmed().split('-');
    if (parts.size() == 3 && parts.at(0).toInt() > 0)
        return QString("%1/%2/%3").arg(parts.at(2), parts.at(1), parts.at(0));
    if (parts.size() == 2 && parts.at(0).toInt() > 0)
        return QString("/%1/%2").arg(parts.at(1), parts.at(0));

    const auto year = book.year.trimmed().toInt();
    if (year > 0)
        return QString("//%1").arg(year);
    return { };
}

// the books of one import that are the same issue; empty when a book cannot be compared
QString sameIssueKey(const CblBook &book)
{
    if (const auto id = comicVineIssueId(book); !id.isEmpty())
        return "cv:" + id;
    if (book.series.trimmed().isEmpty() && book.number.trimmed().isEmpty())
        return { };
    return QStringList { book.series.trimmed().toCaseFolded(), book.number.trimmed().toCaseFolded(), book.volume.trimmed().toCaseFolded(), book.year.trimmed() }.join(QChar(0x1F));
}

}

Placeholder placeholderFromCblBook(const CblBook &book)
{
    Placeholder placeholder;
    placeholder.series = text(book.series);
    placeholder.number = text(book.number);
    placeholder.volume = text(book.volume);
    placeholder.format = text(book.format);
    placeholder.date = date(book);
    placeholder.comicVineID = text(comicVineIssueId(book));
    return placeholder;
}

QString cblListName(const CblReadingList &list, const QString &filePath)
{
    const auto name = list.name.trimmed();
    return name.isEmpty() ? QFileInfo(filePath).completeBaseName() : name;
}

std::optional<ReadingListId> importCblReadingList(const QString &name, const CblReadingList &list, const QList<std::optional<CblCandidate>> &choices, QSqlDatabase &db)
{
    const auto &books = list.books;
    if (books.size() != choices.size())
        return std::nullopt;
    const auto sourceFormat = list.format == CblReadingList::Json ? QStringLiteral("json") : QStringLiteral("xml");

    db.transaction();
    const auto fail = [&db](const QSqlQuery &query) -> std::optional<ReadingListId> {
        QLOG_ERROR() << "Unable to import a CBL reading list:" << query.lastError().text();
        db.rollback();
        return std::nullopt;
    };

    QSqlQuery readingList(db);
    readingList.prepare("INSERT INTO reading_list (name) VALUES (:name)");
    readingList.bindValue(":name", name);
    if (!readingList.exec())
        return fail(readingList);
    const auto readingListId = ReadingListId(readingList.lastInsertId().toULongLong());

    QSqlQuery entry(db);
    entry.prepare("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id, placeholder_id, source, source_format) "
                  "VALUES (:reading_list_id, :ordering, :comic_info_id, :placeholder_id, :source, :source_format)");

    QHash<QString, PlaceholderId> newPlaceholders;
    for (qsizetype i = 0; i < books.size(); i++) {
        QVariant comicInfoId;
        QVariant placeholderId;
        if (const auto &choice = choices.at(i)) {
            std::visit(overloaded {
                               [&](const CblLibraryComic &comic) { comicInfoId = comic.id.value(); },
                               [&](const CblLibraryPlaceholder &placeholder) { placeholderId = placeholder.id.value(); },
                       },
                       *choice);
        } else {
            const auto key = sameIssueKey(books.at(i));
            std::optional<PlaceholderId> created;
            if (!key.isEmpty() && newPlaceholders.contains(key)) {
                created = newPlaceholders.value(key);
            } else {
                created = DBHelper::insertPlaceholder(placeholderFromCblBook(books.at(i)), db);
                if (!created) {
                    db.rollback();
                    return std::nullopt;
                }
                if (!key.isEmpty())
                    newPlaceholders.insert(key, *created);
            }
            placeholderId = created->value();
        }

        entry.bindValue(":reading_list_id", readingListId.value());
        entry.bindValue(":ordering", i);
        entry.bindValue(":comic_info_id", comicInfoId);
        entry.bindValue(":placeholder_id", placeholderId);
        // only a placeholder needs the book to be matched later
        const bool keepSource = !placeholderId.isNull() && !books.at(i).source.isEmpty();
        entry.bindValue(":source", keepSource ? QVariant(books.at(i).source) : QVariant());
        entry.bindValue(":source_format", keepSource ? QVariant(sourceFormat) : QVariant());
        if (!entry.exec())
            return fail(entry);
    }

    if (!db.commit())
        return fail(readingList);
    return readingListId;
}
