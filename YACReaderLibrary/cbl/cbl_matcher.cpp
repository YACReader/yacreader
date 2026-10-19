#include "cbl_matcher.h"

#include "QsLog.h"

#include <QRegularExpression>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <cstdlib>
#include <functional>

namespace {

// "v1", "Vol. 2", "Volume 3" inside a series name
const QRegularExpression &volumeInName()
{
    static const QRegularExpression expression(R"(\bv(ol(ume)?)?\.?\s?\d+\b\s*)", QRegularExpression::CaseInsensitiveOption);
    return expression;
}

// "Batman (2016)": Comic Vine names a volume by its start year
const QRegularExpression &yearInName()
{
    static const QRegularExpression expression(R"(^(.*\S)\s*\((\d{4})\)\s*$)");
    return expression;
}

QString sameSeries(const QString &series)
{
    return series.simplified().toCaseFolded();
}

QString seriesWithoutVolume(const QString &series)
{
    auto result = series;
    result.remove(volumeInName());
    return sameSeries(result);
}

// letters and digits of any script, without the words "the" and "and"; only Latin letters lose their accents,
// because in other scripts a mark can make another letter (ボ and ホ, й and и)
QString lettersAndDigits(const QString &series)
{
    const auto decomposed = seriesWithoutVolume(series).normalized(QString::NormalizationForm_KD);
    QString result;
    QString word;
    const auto endWord = [&] {
        if (word != QLatin1StringView("the") && word != QLatin1StringView("and"))
            result += word;
        word.clear();
    };
    auto latinBase = false;
    for (const auto c : decomposed) {
        if (c.isMark()) {
            if (!latinBase)
                word += c;
            continue;
        }
        latinBase = c.script() == QChar::Script_Latin;
        if (c.isLetterOrNumber())
            word += c;
        else
            endWord();
    }
    endWord();
    return result;
}

// numbers are text; only the case and leading zeros do not count
QString number(const QString &number)
{
    static const QRegularExpression leadingZeros(R"(^(-?)0+(?=\d))");
    auto result = number.trimmed().toCaseFolded();
    result.replace(leadingZeros, QStringLiteral("\\1"));
    return result;
}

int yearOfDate(const QString &date)
{
    const auto year = date.section('/', -1).trimmed().toInt();
    return year > 0 ? year : 0;
}

int bookYear(const CblBook &book)
{
    auto year = book.year.toInt();
    if (year <= 0)
        year = book.coverDate.section('-', 0, 0).toInt();
    return year > 0 ? year : 0;
}

// the volume, or the year in a "Series (Year)" name
QString effectiveVolume(const CblLibraryMetadata &metadata)
{
    if (!metadata.volume.trimmed().isEmpty())
        return metadata.volume.trimmed();
    const auto year = yearInName().match(metadata.series);
    return year.hasMatch() ? year.captured(2) : QString();
}

QString key(const QString &series, const QString &issueNumber)
{
    return series + QChar(0x1F) + number(issueNumber);
}

// the keys of a series at a name level; "Series (Year)" gives one key per known year
QStringList seriesKeys(CblMatchLevel level, const QString &series, const QString &volume)
{
    switch (level) {
    case CblMatchLevel::SameSeries:
        return { sameSeries(series) };
    case CblMatchLevel::SeriesWithoutVolume:
        return { seriesWithoutVolume(series) };
    case CblMatchLevel::LettersAndDigits:
        return { lettersAndDigits(series) };
    case CblMatchLevel::SeriesWithYear: {
        QStringList keys;
        const auto year = yearInName().match(series);
        if (year.hasMatch())
            keys.append(lettersAndDigits(year.captured(1)) + QChar(0x1F) + year.captured(2));
        if (!volume.trimmed().isEmpty())
            keys.append(lettersAndDigits(series) + QChar(0x1F) + volume.trimmed().toCaseFolded());
        return keys;
    }
    case CblMatchLevel::ComicVineId:
        break;
    }
    return { };
}

constexpr CblMatchLevel nameLevels[] = { CblMatchLevel::SameSeries, CblMatchLevel::SeriesWithoutVolume, CblMatchLevel::LettersAndDigits, CblMatchLevel::SeriesWithYear };

bool isComicVine(const CblDatabaseIds &ids)
{
    return ids.name.compare(QLatin1StringView("cv"), Qt::CaseInsensitive) == 0 || ids.name.compare(QLatin1StringView("comicvine"), Qt::CaseInsensitive) == 0;
}

template<typename Item>
CblMatcher::Index buildIndex(const QList<Item> &items)
{
    CblMatcher::Index index;
    for (qsizetype i = 0; i < items.size(); i++) {
        const auto &item = items.at(i);
        if (const auto id = item.comicVineId.trimmed(); !id.isEmpty())
            index.byComicVineId[id].append(i);
        if (item.series.trimmed().isEmpty())
            continue;
        for (int level = 0; level < 4; level++) {
            for (const auto &seriesKey : seriesKeys(nameLevels[level], item.series, item.volume)) {
                auto &positions = index.byName[level][key(seriesKey, item.number)];
                if (!positions.contains(i))
                    positions.append(i);
            }
        }
    }
    return index;
}

// keeps the items that pass, unless none does
template<typename Item>
void narrow(QList<Item> &items, const std::function<bool(const Item &)> &passes)
{
    QList<Item> passing;
    for (const auto &item : std::as_const(items)) {
        if (passes(item))
            passing.append(item);
    }
    if (!passing.isEmpty())
        items = passing;
}

// a comic with a file wins over a deleted one
void preferFiles(QList<CblLibraryComic> &comics)
{
    narrow<CblLibraryComic>(comics, [](const CblLibraryComic &comic) { return comic.hasFile; });
}

void preferFiles(QList<CblLibraryPlaceholder> &)
{
}

template<typename Item>
void breakTies(const CblBook &book, QList<Item> &items)
{
    if (const auto volume = book.volume.trimmed().toCaseFolded(); !volume.isEmpty())
        narrow<Item>(items, [&](const Item &item) { return effectiveVolume(item).toCaseFolded() == volume; });
    if (const auto year = bookYear(book); year > 0)
        narrow<Item>(items, [&](const Item &item) { const auto itemYear = yearOfDate(item.date); return itemYear > 0 && std::abs(itemYear - year) <= 1; });
    if (const auto format = book.format.trimmed(); !format.isEmpty())
        narrow<Item>(items, [&](const Item &item) { return item.format.trimmed().compare(format, Qt::CaseInsensitive) == 0; });
    preferFiles(items);
}

// the volume is the same or the year is within one, and no known volume or year disagrees
bool agrees(const CblBook &book, const CblLibraryMetadata &metadata)
{
    const auto bookVolume = book.volume.trimmed();
    const auto itemVolume = effectiveVolume(metadata);
    const auto volumeKnown = !bookVolume.isEmpty() && !itemVolume.isEmpty();
    const auto volumeSame = volumeKnown && bookVolume.compare(itemVolume, Qt::CaseInsensitive) == 0;

    const auto year = bookYear(book);
    const auto itemYear = yearOfDate(metadata.date);
    const auto yearKnown = year > 0 && itemYear > 0;
    const auto yearClose = yearKnown && std::abs(year - itemYear) <= 1;

    return (volumeSame || yearClose) && (!volumeKnown || volumeSame) && (!yearKnown || yearClose);
}

// the match of `book` among `items`, or nothing when no item fits it
template<typename Item>
std::optional<CblBookMatch> matchIn(const CblBook &book, const QList<Item> &items, const CblMatcher::Index &index)
{
    const auto result = [&](const QList<qsizetype> &found, CblMatchLevel level, bool needsAgreement) {
        QList<Item> fitting;
        for (const auto position : found)
            fitting.append(items.at(position));
        if (needsAgreement)
            breakTies(book, fitting);
        else
            preferFiles(fitting);

        CblBookMatch match;
        match.level = level;
        for (const auto &item : std::as_const(fitting))
            match.candidates.append(item);
        if (fitting.size() == 1 && (!needsAgreement || agrees(book, fitting.first())))
            match.accepted = fitting.first();
        return match;
    };

    QList<qsizetype> found;
    for (const auto &ids : book.databases) {
        if (!isComicVine(ids) || ids.issueId.isEmpty())
            continue;
        for (const auto position : index.byComicVineId.value(ids.issueId)) {
            if (!found.contains(position))
                found.append(position);
        }
    }
    if (!found.isEmpty())
        return result(found, CblMatchLevel::ComicVineId, false);

    if (book.series.trimmed().isEmpty())
        return std::nullopt;

    for (int level = 0; level < 4; level++) {
        for (const auto &seriesKey : seriesKeys(nameLevels[level], book.series, book.volume)) {
            for (const auto position : index.byName[level].value(key(seriesKey, book.number))) {
                if (!found.contains(position))
                    found.append(position);
            }
        }
        if (!found.isEmpty())
            return result(found, nameLevels[level], true);
    }
    return std::nullopt;
}

}

CblMatcher::CblMatcher(const QList<CblLibraryComic> &comics, const QList<CblLibraryPlaceholder> &placeholders)
    : comics(comics), placeholders(placeholders), comicIndex(buildIndex(comics)), placeholderIndex(buildIndex(placeholders))
{
}

// an accepted comic wins; a comic that is only a candidate does not hide a placeholder of the same issue
CblBookMatch CblMatcher::match(const CblBook &book) const
{
    const auto comicMatch = matchIn(book, comics, comicIndex);
    if (comicMatch && comicMatch->accepted)
        return *comicMatch;

    const auto placeholderMatch = matchIn(book, placeholders, placeholderIndex);
    if (!comicMatch)
        return placeholderMatch.value_or(CblBookMatch { });
    if (!placeholderMatch)
        return *comicMatch;

    auto result = *comicMatch;
    result.candidates += placeholderMatch->candidates;
    if (placeholderMatch->accepted) {
        result.accepted = placeholderMatch->accepted;
        result.level = placeholderMatch->level;
    }
    return result;
}

std::optional<QList<CblLibraryComic>> CblMatcher::loadLibraryComics(QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.setForwardOnly(true);
    if (!query.exec("SELECT ci.id, ci.id IN (SELECT comicInfoId FROM comic), ci.series, ci.number, ci.volume, ci.date, ci.format, ci.comicVineID "
                    "FROM comic_info ci "
                    "WHERE COALESCE(ci.series, '') <> '' OR COALESCE(ci.comicVineID, '') <> ''")) {
        QLOG_ERROR() << "Unable to load comics for CBL matching:" << query.lastError().text();
        return std::nullopt;
    }

    QList<CblLibraryComic> comics;
    while (query.next()) {
        CblLibraryComic comic;
        comic.id = ComicInfoId(query.value(0).toULongLong());
        comic.hasFile = query.value(1).toBool();
        comic.series = query.value(2).toString();
        comic.number = query.value(3).toString();
        comic.volume = query.value(4).toString();
        comic.date = query.value(5).toString();
        comic.format = query.value(6).toString();
        comic.comicVineId = query.value(7).toString();
        comics.append(comic);
    }
    return comics;
}

std::optional<QList<CblLibraryPlaceholder>> CblMatcher::loadLibraryPlaceholders(QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.setForwardOnly(true);
    if (!query.exec("SELECT p.id, p.series, p.number, p.volume, p.date, p.format, p.comicVineID "
                    "FROM placeholder p "
                    "WHERE COALESCE(p.series, '') <> '' OR COALESCE(p.comicVineID, '') <> ''")) {
        QLOG_ERROR() << "Unable to load placeholders for CBL matching:" << query.lastError().text();
        return std::nullopt;
    }

    QList<CblLibraryPlaceholder> placeholders;
    while (query.next()) {
        CblLibraryPlaceholder placeholder;
        placeholder.id = PlaceholderId(query.value(0).toULongLong());
        placeholder.series = query.value(1).toString();
        placeholder.number = query.value(2).toString();
        placeholder.volume = query.value(3).toString();
        placeholder.date = query.value(4).toString();
        placeholder.format = query.value(5).toString();
        placeholder.comicVineId = query.value(6).toString();
        placeholders.append(placeholder);
    }
    return placeholders;
}
