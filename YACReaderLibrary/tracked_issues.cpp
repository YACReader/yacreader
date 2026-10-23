#include "tracked_issues.h"

#include "QsLog.h"
#include "db_helper.h"
#include "folder_placeholder_match.h"
#include "placeholder.h"

#include <QRegularExpression>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <algorithm>

namespace {

struct Part {
    QString issue;
    qlonglong first = 0;
    qlonglong last = -1;
};

struct Parts {
    QList<Part> parts;
    bool valid = true;
    bool tooMany = false;
};

// the parts of an Issues text; the issues are counted from the range sizes, so a range is never expanded here
Parts readParts(const QString &text)
{
    static const QRegularExpression range(R"(^(\d+)\s*-\s*(\d+)$)");

    Parts result;
    if (text.trimmed().isEmpty())
        return result;

    qlonglong count = 0;
    for (const auto &part : text.split(',')) {
        const auto issue = part.trimmed();
        if (issue.isEmpty()) {
            result.valid = false;
            continue;
        }
        const auto match = range.match(issue);
        if (!match.hasMatch()) {
            result.parts.append({ issue });
            count++;
        } else {
            bool firstIsNumber = false;
            bool lastIsNumber = false;
            const auto first = match.captured(1).toLongLong(&firstIsNumber);
            const auto last = match.captured(2).toLongLong(&lastIsNumber);
            if (!firstIsNumber || !lastIsNumber) {
                result.tooMany = true;
                break;
            }
            if (last < first) {
                result.valid = false;
                continue;
            }
            result.parts.append({ QString(), first, last });
            count += last - first + 1;
        }
        if (count > maxTrackedIssues) {
            result.tooMany = true;
            break;
        }
    }
    return result;
}

}

bool tooManyTrackedIssues(const QString &text)
{
    return readParts(text).tooMany;
}

TrackedIssues parseTrackedIssues(const QString &text)
{
    const auto parts = readParts(text);
    TrackedIssues result;
    result.valid = parts.valid && !parts.tooMany;
    result.tooMany = parts.tooMany;
    if (parts.tooMany)
        return result;

    QSet<QString> seen;
    const auto add = [&](const QString &number) {
        const auto key = issueNumberKey(number);
        if (seen.contains(key)) {
            result.repeated++;
            return;
        }
        seen.insert(key);
        result.numbers.append(number);
    };
    for (const auto &part : parts.parts) {
        if (!part.issue.isEmpty())
            add(part.issue);
        for (auto number = part.first; number <= part.last; number++)
            add(QString::number(number));
    }
    return result;
}

QString compactIssues(const QStringList &numbers, int parts)
{
    QStringList texts;
    for (int i = 0; i < numbers.size(); i++) {
        bool isWhole = false;
        const auto first = numbers.at(i).toLongLong(&isWhole);
        int last = i;
        while (isWhole && last + 1 < numbers.size() && numbers.at(last + 1).toLongLong() == first + (last + 1 - i) && !numbers.at(last + 1).startsWith('0'))
            last++;
        texts.append(last == i ? numbers.at(i) : QStringLiteral("%1\u2013%2").arg(numbers.at(i), numbers.at(last)));
        i = last;
    }
    if (texts.size() > parts)
        return texts.mid(0, parts).join(QStringLiteral(", ")) + QStringLiteral(", \u2026");
    return texts.join(QStringLiteral(", "));
}

QString missingIssueRanges(const QStringList &numbers)
{
    QList<qlonglong> present;
    for (const auto &number : numbers) {
        bool isWhole = false;
        const auto value = number.trimmed().toLongLong(&isWhole);
        if (isWhole && value > 0)
            present.append(value);
    }
    std::sort(present.begin(), present.end());

    QStringList ranges;
    qlonglong missing = 0;
    qlonglong previous = 0;
    for (const auto number : std::as_const(present)) {
        if (number > previous + 1) {
            const auto first = previous + 1;
            const auto last = number - 1;
            missing += last - first + 1;
            if (missing > maxTrackedIssues)
                return QString();
            ranges.append(last == first ? QString::number(first) : QStringLiteral("%1-%2").arg(first).arg(last));
        }
        previous = number;
    }
    return ranges.join(QStringLiteral(", "));
}

std::optional<QStringList> folderIssueNumbers(FolderId folderId, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.setForwardOnly(true);
    query.prepare("SELECT ci.number FROM comic c INNER JOIN comic_info ci ON ci.id = c.comicInfoId WHERE c.parentId = :folder_id "
                  "UNION ALL SELECT p.number FROM folder_placeholder fp INNER JOIN placeholder p ON p.id = fp.placeholder_id WHERE fp.folder_id = :folder_id");
    query.bindValue(":folder_id", folderId.value());
    if (!query.exec()) {
        QLOG_ERROR() << "Unable to read the issue numbers of a folder:" << query.lastError().text();
        return std::nullopt;
    }
    QStringList numbers;
    while (query.next()) {
        const auto number = query.value(0).toString().trimmed();
        if (!number.isEmpty())
            numbers.append(number);
    }
    return numbers;
}

DescriptiveComicMetadata folderRunMetadata(FolderId folderId, QSqlDatabase &db)
{
    DescriptiveComicMetadata metadata;
    for (const auto &[column, field] : { std::pair { "series", &DescriptiveComicMetadata::series },
                                         std::pair { "volume", &DescriptiveComicMetadata::volume },
                                         std::pair { "publisher", &DescriptiveComicMetadata::publisher } }) {
        QSqlQuery query(db);
        query.prepare(QString("SELECT ci.%1 FROM comic c INNER JOIN comic_info ci ON ci.id = c.comicInfoId "
                              "WHERE c.parentId = :folder_id AND TRIM(COALESCE(ci.%1, '')) <> '' "
                              "GROUP BY ci.%1 ORDER BY COUNT(*) DESC, ci.%1 LIMIT 1")
                              .arg(column));
        query.bindValue(":folder_id", folderId.value());
        if (query.exec() && query.next())
            metadata.*field = query.value(0);
    }
    return metadata;
}

std::optional<int> trackIssuesInFolder(FolderId folderId, const QStringList &numbers, const DescriptiveMetadataPatch &patch, const std::optional<QString> &coverKey, QSqlDatabase &db)
{
    db.transaction();
    const auto existing = folderIssueNumbers(folderId, db);
    if (!existing) {
        db.rollback();
        return std::nullopt;
    }

    QSet<QString> existingKeys;
    for (const auto &number : std::as_const(*existing))
        existingKeys.insert(issueNumberKey(number));

    int tracked = 0;
    for (const auto &number : numbers) {
        if (existingKeys.contains(issueNumberKey(number)))
            continue;

        auto issuePatch = patch;
        issuePatch.set(&DescriptiveComicMetadata::number, number);
        const auto id = DBHelper::insertPlaceholder(Placeholder(), db);
        if (!id || !DBHelper::updatePlaceholderMetadata(*id, issuePatch, db) || (coverKey && !DBHelper::updatePlaceholderCover(*id, coverKey, db)) ||
            !DBHelper::placePlaceholderInFolder(*id, folderId, db)) {
            db.rollback();
            return std::nullopt;
        }
        tracked++;
    }

    if (!db.commit())
        return std::nullopt;
    return tracked;
}
