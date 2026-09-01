#include "organize_files_plan.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

#include <utility>

namespace {

using OrganizeFiles::ComicEntry;

QString translated(const char *text)
{
    return QCoreApplication::translate("OrganizeFiles", text);
}

bool isReservedDeviceName(const QString &segment)
{
    static const QStringList reserved = {
        QStringLiteral("CON"), QStringLiteral("PRN"), QStringLiteral("AUX"), QStringLiteral("NUL"),
        QStringLiteral("COM1"), QStringLiteral("COM2"), QStringLiteral("COM3"), QStringLiteral("COM4"),
        QStringLiteral("COM5"), QStringLiteral("COM6"), QStringLiteral("COM7"), QStringLiteral("COM8"),
        QStringLiteral("COM9"), QStringLiteral("LPT1"), QStringLiteral("LPT2"), QStringLiteral("LPT3"),
        QStringLiteral("LPT4"), QStringLiteral("LPT5"), QStringLiteral("LPT6"), QStringLiteral("LPT7"),
        QStringLiteral("LPT8"), QStringLiteral("LPT9")
    };

    const QString stem = segment.section(QLatin1Char('.'), 0, 0);
    return reserved.contains(stem, Qt::CaseInsensitive);
}

QString rawValue(const QString &name, const ComicEntry &entry)
{
    if (name == QLatin1String("publisher"))
        return entry.publisher;
    if (name == QLatin1String("imprint"))
        return entry.imprint;
    if (name == QLatin1String("series"))
        return entry.series;
    if (name == QLatin1String("volume"))
        return entry.volume;
    if (name == QLatin1String("number"))
        return entry.number;
    if (name == QLatin1String("count"))
        return entry.count;
    if (name == QLatin1String("title"))
        return entry.title;
    if (name == QLatin1String("year"))
        return entry.year;
    if (name == QLatin1String("month"))
        return entry.month;
    if (name == QLatin1String("storyArc"))
        return entry.storyArc;
    if (name == QLatin1String("arcNumber"))
        return entry.arcNumber;
    if (name == QLatin1String("writer"))
        return entry.writer;
    if (name == QLatin1String("filename"))
        return entry.baseName;

    return QString();
}

bool acceptsPadding(const QString &name)
{
    return name == QLatin1String("number") || name == QLatin1String("count") || name == QLatin1String("arcNumber");
}

int paddingWidth(const QString &spec)
{
    if (spec.isEmpty())
        return 0;

    for (const QChar c : spec) {
        if (c != QLatin1Char('0'))
            return 0;
    }

    return spec.size();
}

QString resolveToken(const QString &name,
                     const QString &spec,
                     const ComicEntry &entry,
                     bool insideGroup,
                     bool *empty,
                     QStringList *fallbackFields)
{
    QString value = rawValue(name, entry).trimmed();

    if (value.isEmpty() && !insideGroup) {
        if (name == QLatin1String("series")) {
            value = translated("Unknown Series");
            if (fallbackFields != nullptr)
                *fallbackFields << translated("series");
        } else if (name == QLatin1String("publisher")) {
            value = translated("Unknown Publisher");
            if (fallbackFields != nullptr)
                *fallbackFields << translated("publisher");
        } else if (name == QLatin1String("title")) {
            value = entry.series.trimmed().isEmpty() ? translated("Unknown Series") : entry.series.trimmed();
            if (fallbackFields != nullptr)
                *fallbackFields << translated("title");
        }
    }

    *empty = value.isEmpty();

    if (acceptsPadding(name))
        value = OrganizeFiles::padNumber(value, paddingWidth(spec));

    return value;
}

QString expandTokens(const QString &text,
                     const ComicEntry &entry,
                     bool insideGroup,
                     bool *anyToken,
                     bool *allEmpty,
                     QStringList *fallbackFields)
{
    QString result;
    int i = 0;

    while (i < text.size()) {
        if (text.at(i) != QLatin1Char('{')) {
            result += text.at(i);
            ++i;
            continue;
        }

        const int close = text.indexOf(QLatin1Char('}'), i + 1);
        if (close < 0) {
            result += text.mid(i);
            break;
        }

        const QString content = text.mid(i + 1, close - i - 1);
        const QString name = content.section(QLatin1Char(':'), 0, 0);
        const QString spec = content.section(QLatin1Char(':'), 1);

        bool empty = true;
        result += resolveToken(name, spec, entry, insideGroup, &empty, fallbackFields);

        if (anyToken != nullptr)
            *anyToken = true;
        if (allEmpty != nullptr && !empty)
            *allEmpty = false;

        i = close + 1;
    }

    return result;
}

}

namespace OrganizeFiles {

QStringList knownTokens()
{
    return { QStringLiteral("publisher"), QStringLiteral("imprint"), QStringLiteral("series"),
             QStringLiteral("volume"), QStringLiteral("number"), QStringLiteral("count"),
             QStringLiteral("title"), QStringLiteral("year"), QStringLiteral("month"),
             QStringLiteral("storyArc"), QStringLiteral("arcNumber"), QStringLiteral("writer"),
             QStringLiteral("filename") };
}

QStringList invalidTokens(const QString &pattern)
{
    QStringList invalid;

    static const QRegularExpression tokenExpression(QStringLiteral("\\{([^{}]*)\\}"));
    auto it = tokenExpression.globalMatch(pattern);
    while (it.hasNext()) {
        const auto match = it.next();
        const QString content = match.captured(1);
        const QString name = content.section(QLatin1Char(':'), 0, 0);
        const QString spec = content.section(QLatin1Char(':'), 1);

        const bool nameIsKnown = knownTokens().contains(name);
        const bool specIsValid = spec.isEmpty() ? true : (acceptsPadding(name) && paddingWidth(spec) > 0);

        if (!nameIsKnown || !specIsValid)
            invalid << match.captured(0);
    }

    if (pattern.count(QLatin1Char('{')) != pattern.count(QLatin1Char('}')))
        invalid << QStringLiteral("{");

    if (pattern.count(QLatin1Char('<')) != pattern.count(QLatin1Char('>')))
        invalid << QStringLiteral("<");

    return invalid;
}

bool patternCreatesFolders(const QString &pattern)
{
#ifdef Q_OS_WIN
    return pattern.contains(QLatin1Char('/')) || pattern.contains(QLatin1Char('\\'));
#else
    return pattern.contains(QLatin1Char('/'));
#endif
}

QString pathKey(const QString &path)
{
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    return path.toLower();
#else
    return path;
#endif
}

QString sanitizeSegment(QString segment)
{
    static const QString invalid = QStringLiteral("<>:\"/\\|?*");
    for (QChar &c : segment) {
        if (invalid.contains(c) || c < QChar(0x20))
            c = QLatin1Char('_');
    }

    segment = segment.simplified();

    while (segment.endsWith(QLatin1Char('.')) || segment.endsWith(QLatin1Char(' ')))
        segment.chop(1);

    while (segment.startsWith(QLatin1Char('-')) || segment.startsWith(QLatin1Char('_')) || segment.startsWith(QLatin1Char('.')) || segment.startsWith(QLatin1Char(' ')))
        segment.remove(0, 1);

    while (segment.endsWith(QLatin1Char('-')) || segment.endsWith(QLatin1Char('_')))
        segment.chop(1);

    segment = segment.trimmed();

    if (!segment.isEmpty() && isReservedDeviceName(segment))
        segment.append(QLatin1Char('_'));

    return segment;
}

QString padNumber(const QString &number, int width)
{
    const QString trimmed = number.trimmed();
    if (width <= 0 || trimmed.isEmpty())
        return trimmed;

    int digits = 0;
    while (digits < trimmed.size() && trimmed.at(digits).isDigit())
        ++digits;

    if (digits == 0)
        return trimmed;

    QString leading = trimmed.left(digits);
    while (leading.size() < width)
        leading.prepend(QLatin1Char('0'));

    return leading + trimmed.mid(digits);
}

// Every writer of the date column builds it differently: ComicInfo.xml gives
// "1/7/2018" and writes a 0 where a component is unknown, Comic Vine gives
// "01/07/2018", and the properties dialog gives whatever was typed. The tokens
// are normalised here so that one library cannot produce both "7" and "07".
void applyPublicationDate(ComicEntry &entry, const QString &date)
{
    const auto components = date.split(QLatin1Char('/'));
    if (components.size() != 3)
        return;

    bool valid = false;

    const int month = components.at(1).toInt(&valid);
    if (valid && month >= 1 && month <= 12)
        entry.month = QStringLiteral("%1").arg(month, 2, 10, QLatin1Char('0'));

    const int year = components.at(2).toInt(&valid);
    if (valid && year > 0)
        entry.year = QString::number(year);
}

QString buildRelativePath(const QString &pattern, const ComicEntry &entry, QStringList *fallbackFields)
{
    QString expanded;
    int i = 0;

    while (i < pattern.size()) {
        if (pattern.at(i) == QLatin1Char('<')) {
            const int close = pattern.indexOf(QLatin1Char('>'), i + 1);
            if (close < 0) {
                expanded += expandTokens(pattern.mid(i + 1), entry, false, nullptr, nullptr, fallbackFields);
                break;
            }

            bool anyToken = false;
            bool allEmpty = true;
            const QString group = expandTokens(pattern.mid(i + 1, close - i - 1), entry, true, &anyToken, &allEmpty, nullptr);

            if (!anyToken || !allEmpty)
                expanded += group;

            i = close + 1;
            continue;
        }

        const int nextGroup = pattern.indexOf(QLatin1Char('<'), i);
        const QString chunk = nextGroup < 0 ? pattern.mid(i) : pattern.mid(i, nextGroup - i);
        expanded += expandTokens(chunk, entry, false, nullptr, nullptr, fallbackFields);
        i = nextGroup < 0 ? pattern.size() : nextGroup;
    }

#ifdef Q_OS_WIN
    expanded.replace(QLatin1Char('\\'), QLatin1Char('/'));
#endif

    const auto rawSegments = expanded.split(QLatin1Char('/'), Qt::KeepEmptyParts);

    QStringList segments;
    for (const QString &raw : rawSegments) {
        const QString clean = sanitizeSegment(raw);
        if (!clean.isEmpty())
            segments << clean;
    }

    // An empty last segment would turn the deepest folder into the file, so the
    // original file name takes its place instead.
    const bool fileSegmentIsEmpty = rawSegments.isEmpty() || sanitizeSegment(rawSegments.last()).isEmpty();
    if (segments.isEmpty() || fileSegmentIsEmpty) {
        QString fallback = sanitizeSegment(entry.baseName);
        if (fallback.isEmpty())
            fallback = sanitizeSegment(entry.title);
        if (fallback.isEmpty())
            fallback = translated("Unknown Comic");

        segments << fallback;
    }

    return segments.join(QLatin1Char('/')) + entry.extension;
}

QString defaultPattern(Mode mode)
{
    if (mode == Mode::Rename)
        return QStringLiteral("{series}< #{number:000}>< - {title}>");

    return QStringLiteral("{publisher}/{series}/{series}< #{number:000}>< - {title}>");
}

QList<QPair<QString, QString>> presets(Mode mode)
{
    if (mode == Mode::Rename) {
        return {
            { translated("Series #Number - Title"), QStringLiteral("{series}< #{number:000}>< - {title}>") },
            { translated("Series #Number"), QStringLiteral("{series}< #{number:000}>") },
            { translated("Series #Number (of Count)"), QStringLiteral("{series}< #{number:000}>< (of {count})>") },
            { translated("Number - Title"), QStringLiteral("{number:000}< - {title}>") },
            { translated("Series #Number (Year)"), QStringLiteral("{series}< #{number:000}>< ({year})>") }
        };
    }

    return {
        { translated("Publisher / Series / Series #Number - Title"), QStringLiteral("{publisher}/{series}/{series}< #{number:000}>< - {title}>") },
        { translated("Publisher / Imprint / Series / Series #Number - Title"), QStringLiteral("{publisher}/<{imprint}/>{series}/{series}< #{number:000}>< - {title}>") },
        { translated("Series / Series #Number"), QStringLiteral("{series}/{series}< #{number:000}>") },
        { translated("Publisher / Series / Series #Number (Year)"), QStringLiteral("{publisher}/{series}/{series}< #{number:000}>< ({year})>") },
        { translated("Series / original file name"), QStringLiteral("{series}/{filename}") }
    };
}

PlanBuilder::PlanBuilder(const QList<ComicEntry> &entries, const QString &base, Mode mode)
    : entries(entries), base(QDir::cleanPath(base)), mode(mode)
{
    for (const auto &entry : entries)
        sourcePaths.insert(pathKey(entry.sourceAbsolute));
}

void PlanBuilder::setBase(const QString &base)
{
    this->base = QDir::cleanPath(base);
}

const QHash<QString, QString> &PlanBuilder::namesIn(const QString &absoluteDirectory)
{
    const QString key = pathKey(absoluteDirectory);

    auto it = directoryNames.find(key);
    if (it != directoryNames.end())
        return it.value();

    QHash<QString, QString> names;
    const auto entryList = QDir(absoluteDirectory).entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &name : entryList)
        names.insert(pathKey(name), name);

    return directoryNames.insert(key, names).value();
}

// The casing this directory will actually have on disk — mkpath() never re-cases
// an existing one. The database rows are written from these strings, so they must match.
QString PlanBuilder::canonicalDirectory(const QString &absoluteDirectory)
{
    const QString clean = QDir::cleanPath(absoluteDirectory);
    const QString folded = pathKey(clean);

    auto it = canonicalDirectories.find(folded);
    if (it != canonicalDirectories.end())
        return it.value();

    QString result = clean;
    if (folded != pathKey(base) && folded.startsWith(pathKey(base) + QLatin1Char('/'))) {
        const QString parent = canonicalDirectory(QFileInfo(clean).absolutePath());
        const QString name = QFileInfo(clean).fileName();
        result = parent + QLatin1Char('/') + namesIn(parent).value(pathKey(name), name);
    }

    return canonicalDirectories.insert(folded, result).value();
}

QList<PlannedMove> PlanBuilder::build(const QString &pattern, const Overrides &overrides)
{
    const QDir baseDir(base);

    // A new directory's casing follows its first appearance in the current plan.
    canonicalDirectories.clear();

    // Two passes: entries that stay put claim their paths first, so placement
    // cannot depend on the order of the entries.
    struct Draft {
        PlannedMove move;
        QString destination;
        QStringList fallbackFields;
        bool needsPlacement = false;
    };

    QList<Draft> drafts;
    drafts.reserve(entries.size());

    QSet<QString> claimed;

    for (const auto &original : std::as_const(entries)) {
        ComicEntry entry = original;

        // Resolved here and not when the entry is built, because the base can
        // change while the dialog is open.
        const QString relativeDirectory = baseDir.relativeFilePath(QFileInfo(entry.sourceAbsolute).absolutePath());
        if (relativeDirectory != QLatin1String(".") && !relativeDirectory.startsWith(QLatin1String("..")))
            entry.folderRelative = relativeDirectory;

        PlannedMove move;
        move.comicId = entry.comicId;
        move.sourceAbsolute = entry.sourceAbsolute;
        move.edited = !overrides.value(entry.sourceAbsolute).destinationRelative.isEmpty();

        if (entry.missing) {
            move.status = PlannedMove::Status::Missing;
            move.destinationRelative = baseDir.relativeFilePath(entry.sourceAbsolute);
            drafts.append({ move, QString(), { }, false });
            continue;
        }

        const auto entryOverride = overrides.value(entry.sourceAbsolute);

        QStringList fallbackFields;
        QString patterned = entryOverride.destinationRelative.isEmpty()
                ? buildRelativePath(pattern, entry, &fallbackFields)
                : entryOverride.destinationRelative;

        if (mode == Mode::Rename && entryOverride.destinationRelative.isEmpty()) {
            const QString name = patterned.section(QLatin1Char('/'), -1);
            patterned = entry.folderRelative.isEmpty() ? name : entry.folderRelative + QLatin1Char('/') + name;
        }

        if (entryOverride.excluded) {
            move.status = PlannedMove::Status::Excluded;
            move.destinationRelative = patterned;
            // The file stays where it is, so nothing else may be placed on it.
            claimed.insert(pathKey(entry.sourceAbsolute));
            drafts.append({ move, QString(), { }, false });
            continue;
        }

        // Only the directory part is bent to on-disk casing; a file, unlike a
        // directory, really is renamed to its planned casing.
        const QFileInfo plannedInfo(QDir::cleanPath(base + QLatin1Char('/') + patterned));
        const QString destination = canonicalDirectory(plannedInfo.absolutePath()) + QLatin1Char('/') + plannedInfo.fileName();

        // Compared with case: a capitalisation fix is a real move, and pathKey()
        // would call it unchanged.
        if (destination == entry.sourceAbsolute) {
            move.status = PlannedMove::Status::Unchanged;
            move.destinationRelative = baseDir.relativeFilePath(destination);
            claimed.insert(pathKey(destination));
            drafts.append({ move, QString(), { }, false });
            continue;
        }

        drafts.append({ move, destination, fallbackFields, true });
    }

    QList<PlannedMove> moves;
    moves.reserve(drafts.size());

    for (auto &draft : drafts) {
        if (!draft.needsPlacement) {
            moves.append(draft.move);
            continue;
        }

        PlannedMove &move = draft.move;
        const QString &destination = draft.destination;
        const QStringList &fallbackFields = draft.fallbackFields;

        const QFileInfo destinationInfo(destination);
        const QString directory = destinationInfo.absolutePath();
        const QString stem = destinationInfo.completeBaseName();
        const QString suffix = destinationInfo.suffix().isEmpty() ? QString() : QLatin1Char('.') + destinationInfo.suffix();

        QString candidate = destination;
        int counter = 1;
        while (true) {
            const bool takenInPlan = claimed.contains(pathKey(candidate));
            const bool takenOnDisk = namesIn(directory).contains(pathKey(QFileInfo(candidate).fileName())) && !sourcePaths.contains(pathKey(candidate));

            if (!takenInPlan && !takenOnDisk)
                break;

            candidate = QDir::cleanPath(directory + QLatin1Char('/') + stem + QStringLiteral(" (") + QString::number(counter++) + QLatin1Char(')') + suffix);
        }

        claimed.insert(pathKey(candidate));
        move.destinationRelative = baseDir.relativeFilePath(candidate);

        if (candidate != destination) {
            move.status = PlannedMove::Status::Renamed;
            move.note = QCoreApplication::translate("OrganizeFiles", "Renamed, %1 is already in use").arg(destinationInfo.fileName());
        } else if (!fallbackFields.isEmpty()) {
            move.status = PlannedMove::Status::Incomplete;
            move.note = QCoreApplication::translate("OrganizeFiles", "Missing metadata: %1").arg(fallbackFields.join(QStringLiteral(", ")));
        }

        moves.append(move);
    }

    return moves;
}

}
