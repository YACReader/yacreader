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
QString effectiveVolume(const CblLibraryComic &comic)
{
    if (!comic.volume.trimmed().isEmpty())
        return comic.volume.trimmed();
    const auto year = yearInName().match(comic.series);
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

// keeps the comics that pass, unless none does
void narrow(QList<CblLibraryComic> &comics, const std::function<bool(const CblLibraryComic &)> &passes)
{
    QList<CblLibraryComic> passing;
    for (const auto &comic : std::as_const(comics)) {
        if (passes(comic))
            passing.append(comic);
    }
    if (!passing.isEmpty())
        comics = passing;
}

void breakTies(const CblBook &book, QList<CblLibraryComic> &comics)
{
    if (const auto volume = book.volume.trimmed().toCaseFolded(); !volume.isEmpty())
        narrow(comics, [&](const CblLibraryComic &comic) { return effectiveVolume(comic).toCaseFolded() == volume; });
    if (const auto year = bookYear(book); year > 0)
        narrow(comics, [&](const CblLibraryComic &comic) { const auto comicYear = yearOfDate(comic.date); return comicYear > 0 && std::abs(comicYear - year) <= 1; });
    if (const auto format = book.format.trimmed(); !format.isEmpty())
        narrow(comics, [&](const CblLibraryComic &comic) { return comic.format.trimmed().compare(format, Qt::CaseInsensitive) == 0; });
    narrow(comics, [](const CblLibraryComic &comic) { return comic.hasFile; });
}

// the volume is the same or the year is within one, and no known volume or year disagrees
bool agrees(const CblBook &book, const CblLibraryComic &comic)
{
    const auto bookVolume = book.volume.trimmed();
    const auto comicVolume = effectiveVolume(comic);
    const auto volumeKnown = !bookVolume.isEmpty() && !comicVolume.isEmpty();
    const auto volumeSame = volumeKnown && bookVolume.compare(comicVolume, Qt::CaseInsensitive) == 0;

    const auto year = bookYear(book);
    const auto comicYear = yearOfDate(comic.date);
    const auto yearKnown = year > 0 && comicYear > 0;
    const auto yearClose = yearKnown && std::abs(year - comicYear) <= 1;

    return (volumeSame || yearClose) && (!volumeKnown || volumeSame) && (!yearKnown || yearClose);
}

}

CblMatcher::CblMatcher(const QList<CblLibraryComic> &comics)
    : comics(comics)
{
    for (qsizetype i = 0; i < this->comics.size(); i++) {
        const auto &comic = this->comics.at(i);
        if (const auto id = comic.comicVineId.trimmed(); !id.isEmpty())
            byComicVineId[id].append(i);
        if (comic.series.trimmed().isEmpty())
            continue;
        for (int level = 0; level < 4; level++) {
            for (const auto &seriesKey : seriesKeys(nameLevels[level], comic.series, comic.volume)) {
                auto &indexes = byName[level][key(seriesKey, comic.number)];
                if (!indexes.contains(i))
                    indexes.append(i);
            }
        }
    }
}

CblBookMatch CblMatcher::match(const CblBook &book) const
{
    CblBookMatch result;

    QList<qsizetype> found;
    for (const auto &ids : book.databases) {
        if (!isComicVine(ids) || ids.issueId.isEmpty())
            continue;
        for (const auto index : byComicVineId.value(ids.issueId)) {
            if (!found.contains(index))
                found.append(index);
        }
    }
    if (!found.isEmpty()) {
        for (const auto index : found)
            result.candidates.append(comics.at(index));
        narrow(result.candidates, [](const CblLibraryComic &comic) { return comic.hasFile; });
        result.level = CblMatchLevel::ComicVineId;
        if (result.candidates.size() == 1)
            result.accepted = result.candidates.first();
        return result;
    }

    if (book.series.trimmed().isEmpty())
        return result;

    for (int level = 0; level < 4; level++) {
        for (const auto &seriesKey : seriesKeys(nameLevels[level], book.series, book.volume)) {
            for (const auto index : byName[level].value(key(seriesKey, book.number))) {
                if (!found.contains(index))
                    found.append(index);
            }
        }
        if (found.isEmpty())
            continue;

        for (const auto index : found)
            result.candidates.append(comics.at(index));
        breakTies(book, result.candidates);
        result.level = nameLevels[level];
        if (result.candidates.size() == 1 && agrees(book, result.candidates.first()))
            result.accepted = result.candidates.first();
        return result;
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
        comics.append({ ComicInfoId(query.value(0).toULongLong()),
                        query.value(1).toBool(),
                        query.value(2).toString(),
                        query.value(3).toString(),
                        query.value(4).toString(),
                        query.value(5).toString(),
                        query.value(6).toString(),
                        query.value(7).toString() });
    }
    return comics;
}
