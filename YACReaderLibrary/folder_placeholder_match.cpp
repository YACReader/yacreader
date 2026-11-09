#include "folder_placeholder_match.h"

#include "QsLog.h"
#include "comic_db.h"
#include "db_helper.h"
#include "library_comic_match.h"
#include "placeholder.h"

#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <algorithm>
#include <functional>

namespace {

constexpr int trustedPrefixMinimum = 3;

QString withoutExtension(const QString &fileName)
{
    const auto dot = fileName.lastIndexOf('.');
    return dot > 0 ? fileName.left(dot) : fileName;
}

// the name without extension, tags in brackets, underscores, case, or repeated spaces
QString cleanName(const QString &fileName)
{
    static const QRegularExpression brackets(R"(\([^()]*\)|\[[^\[\]]*\]|\{[^{}]*\})");
    auto name = withoutExtension(fileName);
    name.replace('_', ' ');
    name.remove(brackets);
    return name.toCaseFolded().simplified();
}

const QRegularExpression &numberInName()
{
    static const QRegularExpression expression(R"(\d+(?:\.\d+)?)");
    return expression;
}

// the text before a number of a clean name, with its other numbers as "#"
QString prefixBefore(const QString &name, qsizetype position)
{
    auto prefix = name.left(position);
    prefix.replace(numberInName(), QStringLiteral("#"));
    return prefix;
}

// the words that make a file a special, which has no issue number (Kavita's FormatTagSpecialKeywords, as whole words)
bool isSpecial(const QString &fileName)
{
    static const QRegularExpression special(R"(\b(special|annual|one[ -]shot|tpb|trade paper back|omnibus|compendium|absolute|graphic novel|gn|fcbd|giant[ -]size|director's cut|box[ -]set|anthology|prologue|epilogue|reference)\b|\bsp\d+\b)",
                                            QRegularExpression::CaseInsensitiveOption);
    return special.match(fileName).hasMatch();
}

}

QString issueNumberKey(const QString &number)
{
    static const QRegularExpression leadingZeros(R"(^(-?)0+(?=\d))");
    auto key = number.trimmed().toCaseFolded();
    if (key.startsWith('0') || key.startsWith(QLatin1StringView("-0")))
        key.replace(leadingZeros, QStringLiteral("\\1"));
    return key;
}

bool sameIssueNumber(const QString &a, const QString &b)
{
    return issueNumberKey(a) == issueNumberKey(b);
}

QString comicVineIdOf(const DescriptiveComicMetadata &metadata)
{
    const auto id = metadata.comicVineID.toString().trimmed();
    if (!id.isEmpty())
        return id;
    static const QRegularExpression inNotes(R"(\[CVDB(\d+)\])", QRegularExpression::CaseInsensitiveOption);
    const auto match = inNotes.match(metadata.notes.toString());
    return match.hasMatch() ? match.captured(1) : QString();
}

// in Kavita's order (ComicChapterRegex), the comic rules without other languages; the first that matches gives the number
std::optional<QString> issueNumberInFileName(const QString &fileName)
{
    auto name = withoutExtension(fileName);
    name.replace('_', ' ');
    if (isSpecial(name))
        return std::nullopt;

    static const QList<QRegularExpression> rules = [] {
        const QStringList patterns = {
            // Batman & Wildcat (1 of 3)
            R"(\((\d+) of \d+)",
            // Batman Beyond 04 (of 6) (1999)
            R"(^.+?(\d+)[\s-]?\(of)",
            // Batman Beyond 2.0 001 (2013)
            R"(^.+?\S\.\d (\d+))",
            // Teen Titans v1 001 (1966-02) (digital)
            R"(^.+? v\d+ (?:c ?)?(\d+(?:\.\d+)?))",
            // Batman & Robin the Teen Wonder #0, Batgirl Vol.2000 #57 (December, 2004)
            R"(^.+?\s#(\d+(?:\.\d+)?))",
            // Batman 2016 - Chapter 01, Batman 2016 - Issue #01, Blade Runner 2019 - Ch. 01
            R"(^.+?(?:(?:chapter|ch|c)\.?|issue)\s#?(\d+(?:\.\d+)?))",
            // Invincible 070.5 - Invincible Returns 1 (2010) (digital)
            R"(^.+?(?<!vol)(?<!vol\.)(?<!volume)\s(\d+(?:\.\d+)?)(?![\d.])\s*-)",
            // Batman & Grendel (1996) 01 - Devil's Bones, Saga 001 (2012) (Digital)
            R"(^.+?(?<!vol)(?<!vol\.)(?<!volume) (\d+(?:\.\d+)?)(?![\d.])(?!\s*-?\s*(?:vol(?:ume)?|tome)(?![a-z])))",
            // spawn-123, spawn-chapter-123
            R"(^.+?-(?:chapter-)?(\d+))",
        };
        QList<QRegularExpression> expressions;
        for (const auto &pattern : patterns)
            expressions.append(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
        return expressions;
    }();

    for (const auto &rule : rules) {
        const auto match = rule.match(name);
        if (match.hasMatch())
            return match.captured(1);
    }
    return std::nullopt;
}

FileNamePattern::FileNamePattern(const QList<QPair<QString, QString>> &numberedFiles)
{
    for (const auto &[fileName, number] : numberedFiles)
        add(fileName, number);
}

void FileNamePattern::add(const QString &fileName, const QString &number)
{
    const auto name = cleanName(fileName);
    auto numbers = numberInName().globalMatch(name);
    while (numbers.hasNext()) {
        const auto found = numbers.next();
        auto &observations = prefixes[prefixBefore(name, found.capturedStart())];
        if (sameIssueNumber(found.captured(), number))
            observations.agree++;
        else
            observations.disagree++;
    }
}

bool FileNamePattern::isTrusted(const QString &prefix) const
{
    const auto observations = prefixes.value(prefix);
    return observations.agree >= trustedPrefixMinimum && observations.disagree == 0;
}

bool FileNamePattern::isLearned() const
{
    for (auto it = prefixes.cbegin(); it != prefixes.cend(); ++it) {
        if (isTrusted(it.key()))
            return true;
    }
    return false;
}

std::optional<QString> FileNamePattern::issueNumber(const QString &fileName) const
{
    const auto name = cleanName(fileName);
    std::optional<QString> result;
    auto numbers = numberInName().globalMatch(name);
    while (numbers.hasNext()) {
        const auto found = numbers.next();
        if (!isTrusted(prefixBefore(name, found.capturedStart())))
            continue;
        if (result && !sameIssueNumber(*result, found.captured()))
            return std::nullopt;
        result = found.captured();
    }
    return result;
}

namespace {

// the only placeholder that `fits`, if there is exactly one
std::optional<Placeholder> single(const QList<Placeholder> &placeholders, const std::function<bool(const Placeholder &)> &fits)
{
    std::optional<Placeholder> found;
    for (const auto &placeholder : placeholders) {
        if (!fits(placeholder))
            continue;
        if (found)
            return std::nullopt;
        found = placeholder;
    }
    return found;
}

// file name and number of the other comics of the folder that have a number
QList<QPair<QString, QString>> numberedFilesOf(FolderId folderId, ComicId excluded, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.setForwardOnly(true);
    query.prepare("SELECT c.fileName, ci.number FROM comic c INNER JOIN comic_info ci ON ci.id = c.comicInfoId "
                  "WHERE c.parentId = :folder_id AND c.id <> :comic_id AND TRIM(COALESCE(ci.number, '')) <> ''");
    query.bindValue(":folder_id", folderId.value());
    query.bindValue(":comic_id", excluded.value());
    QList<QPair<QString, QString>> files;
    if (!query.exec()) {
        QLOG_ERROR() << "Unable to read the numbered comics of a folder:" << query.lastError().text();
        return files;
    }
    while (query.next())
        files.append({ query.value(0).toString(), query.value(1).toString() });
    return files;
}

}

std::optional<Placeholder> MissingComicMatcher::missingComicOf(const ComicDB &comic, FolderState &folder, QSqlDatabase &db)
{
    const auto &placeholders = folder.placeholders;
    const auto &info = comic.info;
    if (const auto comicVineId = comicVineIdOf(info); !comicVineId.isEmpty()) {
        if (auto found = single(placeholders, [&](const Placeholder &p) { return comicVineIdOf(p) == comicVineId; }))
            return found;
    }

    const auto number = info.number.toString().trimmed();
    if (!number.isEmpty()) {
        const auto series = info.series.toString().simplified();
        return single(placeholders, [&](const Placeholder &p) {
            const auto placeholderSeries = p.series.toString().simplified();
            const bool sameSeries = series.isEmpty() || placeholderSeries.isEmpty() || series.compare(placeholderSeries, Qt::CaseInsensitive) == 0;
            return sameSeries && sameIssueNumber(p.number.toString(), number);
        });
    }

    // the comics added later in the scan are learned as they come (match)
    if (!folder.pattern)
        folder.pattern = FileNamePattern(numberedFilesOf(comic.file.folderId, comic.file.id, db));
    const auto fileNumber = folder.pattern->isLearned() ? folder.pattern->issueNumber(comic.file.fileName) : issueNumberInFileName(comic.file.fileName);
    if (!fileNumber)
        return std::nullopt;
    return single(placeholders, [&](const Placeholder &p) { return sameIssueNumber(p.number.toString(), *fileNumber); });
}

std::optional<PlaceholderId> MissingComicMatcher::match(const ComicDB &comic, const QString &libraryDataPath, QSqlDatabase &db, ComicMatchFiles &files)
{
    auto folder = folders.find(comic.file.folderId);
    if (folder == folders.end()) {
        const auto placeholders = DBHelper::loadFolderPlaceholders(comic.file.folderId, db);
        if (!placeholders)
            return std::nullopt;
        folder = folders.insert(comic.file.folderId, FolderState { *placeholders, std::nullopt });
    }
    if (folder->placeholders.isEmpty())
        return std::nullopt;

    // the number the comic has after the match, as the pattern would read it from the database
    const auto learn = [&folder, &comic](const QString &number) {
        if (folder->pattern && !number.trimmed().isEmpty())
            folder->pattern->add(comic.file.fileName, number);
    };

    const auto ownNumber = comic.info.number.toString();
    const auto missing = missingComicOf(comic, *folder, db);
    if (!missing) {
        learn(ownNumber);
        return std::nullopt;
    }

    // a failed match leaves nothing in the caller's transaction
    QSqlQuery savepoint(db);
    if (!savepoint.exec("SAVEPOINT missing_comic_match"))
        return std::nullopt;
    ComicMatchFiles matchFiles;
    const bool matched = matchPlaceholderToComic(missing->id, comic.info.id, libraryDataPath, db, matchFiles);
    if (!matched)
        savepoint.exec("ROLLBACK TO missing_comic_match");
    savepoint.exec("RELEASE missing_comic_match");
    if (!matched) {
        finishComicMatchFiles(libraryDataPath, matchFiles, false);
        learn(ownNumber);
        return std::nullopt;
    }
    folder->placeholders.removeIf([&missing](const Placeholder &placeholder) { return placeholder.id == missing->id; });
    learn(ownNumber.trimmed().isEmpty() ? missing->number.toString() : ownNumber);
    files.writtenCoverPaths.append(matchFiles.writtenCoverPaths);
    files.placeholderCoverKeys.append(matchFiles.placeholderCoverKeys);
    files.comicCoverHashes.append(matchFiles.comicCoverHashes);
    return missing->id;
}
