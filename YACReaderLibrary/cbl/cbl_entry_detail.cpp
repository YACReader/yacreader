#include "cbl_entry_detail.h"

#include "qnaturalsorting.h"

#include <QDir>
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSet>
#include <QVBoxLayout>

#include <algorithm>
#include <array>
#include <functional>

namespace {

enum Field {
    SeriesField,
    NumberField,
    VolumeField,
    YearField,
    FormatField,
    ComicVineField,
    FieldCount,
};

using Values = std::array<QString, FieldCount>;
using Differences = std::array<bool, FieldCount>;

QString fieldName(int field)
{
    switch (field) {
    case SeriesField:
        return CblEntryDetail::tr("Series");
    case NumberField:
        return CblEntryDetail::tr("Number");
    case VolumeField:
        return CblEntryDetail::tr("Volume");
    case YearField:
        return CblEntryDetail::tr("Year");
    case FormatField:
        return CblEntryDetail::tr("Format");
    case ComicVineField:
        return CblEntryDetail::tr("ComicVine ID");
    }
    return { };
}

Values bookValues(const CblBook &book)
{
    const auto year = cblBookYear(book);
    return { book.series, book.number, book.volume, year > 0 ? QString::number(year) : QString(), book.format.isEmpty() ? book.issueType : book.format, cblComicVineIssueId(book) };
}

Values metadataValues(const CblLibraryMetadata &metadata)
{
    const auto year = cblYearOfDate(metadata.date);
    return { metadata.series, metadata.number, metadata.volume, year > 0 ? QString::number(year) : QString(), metadata.format, metadata.comicVineId };
}

bool differentText(const QString &a, const QString &b)
{
    return !a.trimmed().isEmpty() && !b.trimmed().isEmpty() && a.simplified().compare(b.simplified(), Qt::CaseInsensitive) != 0;
}

// a value that is not known on one side is not different
Differences differences(const CblBook &book, const CblLibraryMetadata &metadata)
{
    const auto bookYear = cblBookYear(book);
    const auto year = cblYearOfDate(metadata.date);
    return { differentText(book.series, metadata.series),
             !book.number.trimmed().isEmpty() && !metadata.number.trimmed().isEmpty() && !cblSameNumber(book.number, metadata.number),
             differentText(book.volume, cblEffectiveVolume(metadata)),
             bookYear > 0 && year > 0 && bookYear != year,
             differentText(book.format.isEmpty() ? book.issueType : book.format, metadata.format),
             differentText(cblComicVineIssueId(book), metadata.comicVineId) };
}

QString metadataTitle(const CblLibraryMetadata &metadata)
{
    return metadata.number.isEmpty() ? metadata.series : metadata.series + QStringLiteral(" #") + metadata.number;
}

QString coverPath(const QString &libraryDataPath, const CblCandidate &candidate)
{
    if (const auto *comic = std::get_if<CblLibraryComic>(&candidate))
        return comic->hash.isEmpty() ? QString() : QDir(libraryDataPath).filePath(QStringLiteral("covers/") + comic->hash + QStringLiteral(".jpg"));
    const auto &placeholder = std::get<CblLibraryPlaceholder>(candidate);
    return placeholder.coverKey.isEmpty() ? QString() : QDir(libraryDataPath).filePath(QStringLiteral("covers/placeholders/") + placeholder.coverKey + QStringLiteral(".jpg"));
}

bool hasFile(const CblCandidate &candidate)
{
    const auto *comic = std::get_if<CblLibraryComic>(&candidate);
    return comic != nullptr && comic->hasFile;
}

const CblLibraryMetadata &metadataOf(const CblCandidate &candidate)
{
    return std::visit([](const auto &item) -> const CblLibraryMetadata & { return item; }, candidate);
}

// the distinct non-empty values, in natural order
QStringList distinctValues(QStringList values)
{
    values.removeAll(QString());
    values.removeDuplicates();
    std::sort(values.begin(), values.end(), naturalSortLessThanCI);
    return values;
}

// one column next to the book: a comic, a possible comic, or the closest series
struct Subject {
    QString title;
    QString subtitle;
    std::optional<CblCandidate> candidate;
    bool showsCover = false;
    QString coverPath;
    Values values;
    Differences different { };
};

QLabel *mutedLabel(const QString &text, QWidget *parent)
{
    auto label = new QLabel(text, parent);
    label->setForegroundRole(QPalette::PlaceholderText);
    return label;
}

QLabel *boldLabel(const QString &text, QWidget *parent)
{
    auto label = new QLabel(text, parent);
    auto font = label->font();
    font.setBold(true);
    label->setFont(font);
    return label;
}

}

QString cblMatchLevelText(CblMatchLevel level)
{
    switch (level) {
    case CblMatchLevel::ComicVineId:
        return CblEntryDetail::tr("ComicVine ID");
    case CblMatchLevel::SameSeries:
        return CblEntryDetail::tr("series and number");
    case CblMatchLevel::SeriesWithoutVolume:
        return CblEntryDetail::tr("series without the volume, and number");
    case CblMatchLevel::LettersAndDigits:
        return CblEntryDetail::tr("letters and digits of the series, and number");
    case CblMatchLevel::SeriesWithYear:
        return CblEntryDetail::tr("series with its year, and number");
    }
    return { };
}

QColor cblWarningColor(const QPalette &palette)
{
    const bool dark = palette.color(QPalette::Base).lightness() < 128;
    return dark ? QColor(0xE0, 0xA0, 0x30) : QColor(0xB2, 0x6B, 0x00);
}

QString cblBookDetails(const CblBook &book)
{
    QStringList parts;
    if (!book.volume.isEmpty())
        parts.append(CblEntryDetail::tr("volume %1").arg(book.volume));
    if (const auto year = cblBookYear(book); year > 0)
        parts.append(QString::number(year));
    return parts.join(QStringLiteral(", "));
}

CblEntryDetail::CblEntryDetail(const CblImportEntry &entry, int row, int entryCount, const QList<CblLibraryComic> &closestSeries, const QString &libraryDataPath, const QColor &warningColor, QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    QList<Subject> subjects;
    QString footer;

    const auto subjectOf = [&](const CblCandidate &candidate, const QString &title) {
        Subject subject;
        subject.title = title;
        if (const auto *comic = std::get_if<CblLibraryComic>(&candidate); comic != nullptr && comic->hasFile)
            subject.subtitle = CblImportModel::fileName(*comic);
        else
            subject.subtitle = metadataTitle(metadataOf(candidate));
        subject.candidate = candidate;
        subject.showsCover = true;
        subject.coverPath = coverPath(libraryDataPath, candidate);
        subject.values = metadataValues(metadataOf(candidate));
        subject.different = differences(entry.book, metadataOf(candidate));
        return subject;
    };

    if (entry.choice) {
        subjects.append(subjectOf(*entry.choice, hasFile(*entry.choice) ? tr("Comic in your library") : tr("Missing in your library")));
        subjects.last().candidate.reset();
        if (entry.changed)
            footer = tr("Changed by you.");
        else if (entry.current == entry.choice)
            footer = tr("The entry has this comic.");
        else if (hasFile(*entry.choice))
            footer = tr("Matched by %1.").arg(cblMatchLevelText(entry.match.level.value_or(CblMatchLevel::SameSeries)));
        else
            footer = tr("Your library already has this comic as missing. Matched by %1.").arg(cblMatchLevelText(entry.match.level.value_or(CblMatchLevel::SameSeries)));
    } else if (!entry.match.candidates.isEmpty()) {
        for (int i = 0; i < entry.match.candidates.size(); ++i)
            subjects.append(subjectOf(entry.match.candidates.at(i), tr("Possible comic %1").arg(i + 1)));
        if (entry.changed)
            footer = tr("Changed by you: it is imported as missing.");
        else if (entry.match.level == CblMatchLevel::ComicVineId)
            footer = tr("The comics have the same ComicVine ID.");
        else if (entry.match.candidates.size() == 1)
            footer = tr("The comic has the same series and number, but its volume or year is different.");
        else
            footer = tr("The comics have the same series and number.");
        footer += QLatin1Char(' ') + tr("Values that are different from the file are in amber.");
    } else {
        if (!closestSeries.isEmpty()) {
            const auto &first = closestSeries.first();
            Subject subject;
            subject.title = tr("Closest series in your library");
            subject.showsCover = true;
            subject.coverPath = coverPath(libraryDataPath, first);
            const auto folder = first.path.section('/', 0, -2).mid(1);
            subject.subtitle = folder.isEmpty() ? tr("Library") : folder;

            QStringList numbers;
            QStringList volumes;
            QStringList years;
            QStringList formats;
            for (const auto &comic : closestSeries) {
                numbers.append(comic.number);
                volumes.append(cblEffectiveVolume(comic));
                const auto year = cblYearOfDate(comic.date);
                years.append(year > 0 ? QString::number(year) : QString());
                formats.append(comic.format);
            }
            numbers = distinctValues(numbers);
            volumes = distinctValues(volumes);
            years = distinctValues(years);
            formats = distinctValues(formats);

            constexpr int shownNumbers = 12;
            auto numbersText = numbers.mid(0, shownNumbers).join(QStringLiteral(", "));
            if (numbers.size() > shownNumbers)
                numbersText += QStringLiteral(", …");
            const auto contains = [](const QStringList &values, const QString &value, const std::function<bool(const QString &, const QString &)> &same) {
                return value.trimmed().isEmpty() || values.isEmpty() || std::any_of(values.cbegin(), values.cend(), [&](const QString &other) { return same(value, other); });
            };
            const auto sameText = [](const QString &a, const QString &b) { return !differentText(a, b); };
            const auto bookYear = cblBookYear(entry.book);

            subject.values = { first.series, numbers.isEmpty() ? QString() : tr("Has %1").arg(numbersText), volumes.join(QStringLiteral(", ")), years.join(QStringLiteral(", ")), formats.join(QStringLiteral(", ")), QString() };
            subject.different = { differentText(entry.book.series, first.series),
                                  !contains(numbers, entry.book.number, cblSameNumber),
                                  !contains(volumes, entry.book.volume, sameText),
                                  !contains(years, bookYear > 0 ? QString::number(bookYear) : QString(), sameText),
                                  !contains(formats, entry.book.format, sameText),
                                  false };
            subjects.append(subject);
        }

        QStringList tried;
        if (!cblComicVineIssueId(entry.book).isEmpty())
            tried.append(cblMatchLevelText(CblMatchLevel::ComicVineId));
        if (!entry.book.series.trimmed().isEmpty()) {
            for (const auto level : { CblMatchLevel::SameSeries, CblMatchLevel::SeriesWithoutVolume, CblMatchLevel::LettersAndDigits, CblMatchLevel::SeriesWithYear })
                tried.append(cblMatchLevelText(level));
        }
        footer = entry.changed ? tr("Changed by you: it is imported as missing.") : tr("No comic found. Tried: %1.").arg(tried.join(QStringLiteral("; ")));
    }

    const auto warningStyle = QStringLiteral("color: %1;").arg(warningColor.name());

    grid = new QGridLayout;
    grid->setHorizontalSpacing(24);
    grid->setVerticalSpacing(4);

    auto bookHeader = new QVBoxLayout;
    bookHeader->setSpacing(0);
    bookHeader->addStretch();
    bookHeader->addWidget(boldLabel(tr("In the file"), this));
    bookHeader->addWidget(mutedLabel(tr("Entry %1 of %2").arg(row + 1).arg(entryCount), this));
    grid->addLayout(bookHeader, 0, 1, Qt::AlignBottom);

    for (int i = 0; i < subjects.size(); ++i) {
        const auto &subject = subjects.at(i);
        auto header = new QHBoxLayout;
        header->setSpacing(8);
        if (subject.showsCover) {
            auto cover = new QLabel(this);
            cover->setFixedSize(36, 54);
            cover->setAlignment(Qt::AlignCenter);
            QPixmap pixmap;
            if (!subject.coverPath.isEmpty() && pixmap.load(subject.coverPath))
                cover->setPixmap(pixmap.scaled(cover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            else
                cover->setStyleSheet(QStringLiteral("background-color: palette(mid);"));
            header->addWidget(cover);
        }

        auto texts = new QVBoxLayout;
        texts->setSpacing(0);
        texts->addWidget(boldLabel(subject.title, this));
        texts->addWidget(mutedLabel(subject.subtitle, this));
        if (subject.candidate) {
            const auto candidate = *subject.candidate;
            auto button = new QPushButton(hasFile(candidate) ? tr("Use this comic") : tr("Use this missing comic"), this);
            button->setAutoDefault(false);
            connect(button, &QPushButton::clicked, this, [this, candidate] { emit candidateChosen(candidate); });
            texts->addWidget(button, 0, Qt::AlignLeft);
        }
        header->addLayout(texts);
        header->addStretch();
        grid->addLayout(header, 0, i + 2, Qt::AlignBottom);
    }

    const auto book = bookValues(entry.book);
    for (int field = 0; field < FieldCount; ++field) {
        const auto anyDifferent = std::any_of(subjects.cbegin(), subjects.cend(), [field](const Subject &subject) { return subject.different[field]; });
        auto name = new QLabel(anyDifferent ? fieldName(field) + QStringLiteral(" ≠") : fieldName(field), this);
        if (anyDifferent)
            name->setStyleSheet(warningStyle);
        else
            name->setForegroundRole(QPalette::PlaceholderText);
        labelsWidth = std::max(labelsWidth, name->sizeHint().width());
        grid->addWidget(name, field + 1, 0);
        grid->addWidget(new QLabel(book[field].isEmpty() ? QStringLiteral("—") : book[field], this), field + 1, 1);

        for (int i = 0; i < subjects.size(); ++i) {
            const auto &value = subjects.at(i).values[field];
            const auto text = value.isEmpty() ? QStringLiteral("—") : value;
            auto label = subjects.at(i).different[field] ? new QLabel(text, this) : mutedLabel(text, this);
            if (subjects.at(i).different[field])
                label->setStyleSheet(warningStyle);
            grid->addWidget(label, field + 1, i + 2);
        }
    }
    grid->setColumnStretch(subjects.size() + 2, 1);

    auto footerLabel = mutedLabel(footer, this);
    footerLabel->setWordWrap(true);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 10);
    layout->addLayout(grid);
    layout->addSpacing(6);
    layout->addWidget(footerLabel);
}

void CblEntryDetail::alignLibraryColumns(int x)
{
    const auto left = layout()->contentsMargins().left();
    grid->setColumnMinimumWidth(0, labelsWidth);
    grid->setColumnMinimumWidth(1, std::max(0, x - left - labelsWidth - 2 * grid->horizontalSpacing()));
}
