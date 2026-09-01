#include "db_helper.h"
#include "organize_files_journal.h"
#include "organize_files_plan.h"
#include "organize_files_worker.h"

#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

#include <algorithm>

using namespace OrganizeFiles;

class OrganizeFilesTest : public QObject
{
    Q_OBJECT

private slots:
    void substitutesEveryToken();
    void readsTheYearAndTheMonthFromTheDateColumn();
    void keepsPunctuationOutOfEmptyOptionalGroups();
    void padsOnlyTheLeadingDigits();
    void reportsInvalidTokens();
    void sanitizesSegments();
    void resolvesCollisionsAgainstDiskAndPlan();
    void freesTheNameOfAFileThatMovesAway();
    void keepsOverridesAndExclusions();
    void renameModeKeepsEveryFileInItsFolder();
    void rejectsSeparatorsInAFilenamePattern();
    void claimsThePathOfAnExcludedComic();
    void renamesWhenOnlyTheCaseChanges();
    void adoptsTheOnDiskCasingOfExistingFolders();
    void mergesPlannedFolderCasingsIntoOne();
    void ordersMovesSoNothingIsOverwritten();
    void removesOnlyEmptyDirectoriesInsideTheBase();
    void sweepsCreatedDirectoriesWhenAMoveFails();
    void undoRemovesOnlyTheDirectoriesTheRunCreated();
    void movesComicRowWithoutLosingCuration();
    void keepsAFolderThatStillHoldsAComic();
    void createsFolderRowsInheritingTheType();
    void removesOnlyEmptyCreatedFolderRows();
    void journalRoundTrip();
    void journalCarriesTheFolderRowsItRemoved();
    void undoPutsTheDatabaseBackExactlyAsItWas();
};

namespace {

ComicEntry spiderMan()
{
    ComicEntry entry;
    entry.comicId = 1;
    entry.sourceAbsolute = QStringLiteral("/library/Unsorted/asm42.cbz");
    entry.baseName = QStringLiteral("asm42");
    entry.extension = QStringLiteral(".cbz");
    entry.publisher = QStringLiteral("Marvel");
    entry.imprint = QStringLiteral("Epic");
    entry.series = QStringLiteral("The Amazing Spider-Man");
    entry.volume = QStringLiteral("1");
    entry.number = QStringLiteral("42");
    entry.count = QStringLiteral("100");
    entry.title = QStringLiteral("The Sinister Six");
    entry.year = QStringLiteral("2018");
    entry.month = QStringLiteral("07");
    entry.storyArc = QStringLiteral("Sinister War");
    entry.arcNumber = QStringLiteral("2");
    entry.writer = QStringLiteral("Dan Slott");
    return entry;
}

ComicEntry bareScan()
{
    ComicEntry entry;
    entry.comicId = 2;
    entry.sourceAbsolute = QStringLiteral("/library/Unsorted/scan001.cbz");
    entry.baseName = QStringLiteral("scan001");
    entry.extension = QStringLiteral(".cbz");
    return entry;
}

QSqlDatabase createDatabase(const QString &connectionName)
{
    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
    db.setDatabaseName(QStringLiteral(":memory:"));
    db.open();

    QSqlQuery query(db);
    query.exec("PRAGMA foreign_keys = ON");
    query.exec("CREATE TABLE folder (id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, name TEXT NOT NULL, path TEXT NOT NULL, "
               "finished BOOLEAN DEFAULT 0, completed BOOLEAN DEFAULT 1, numChildren INTEGER, firstChildHash TEXT, customImage TEXT, "
               "manga BOOLEAN DEFAULT 0, type INTEGER DEFAULT 0, added INTEGER, updated INTEGER, "
               "FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE)");
    query.exec("CREATE TABLE comic_info (id INTEGER PRIMARY KEY, hash TEXT, added INTEGER)");
    query.exec("CREATE TABLE comic (id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, comicInfoId INTEGER NOT NULL, fileName TEXT NOT NULL, path TEXT, "
               "FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE, FOREIGN KEY(comicInfoId) REFERENCES comic_info(id))");
    query.exec("CREATE TABLE label (id INTEGER PRIMARY KEY, name TEXT, ordering INTEGER, color INTEGER)");
    query.exec("CREATE TABLE comic_label (id INTEGER PRIMARY KEY, label_id INTEGER, comic_id INTEGER, ordering INTEGER, "
               "FOREIGN KEY(comic_id) REFERENCES comic(id) ON DELETE CASCADE)");
    query.exec("CREATE TABLE comic_reading_list (id INTEGER PRIMARY KEY, reading_list_id INTEGER, comic_id INTEGER, ordering INTEGER, "
               "FOREIGN KEY(comic_id) REFERENCES comic(id) ON DELETE CASCADE)");

    query.exec("INSERT INTO folder VALUES (1, 1, 'root', '/', 0, 1, NULL, NULL, NULL, 0, 1, 100, 100)");
    // customImage and type are the folder state that an undo has to bring back. They
    // only survive if the row comes back with the same id.
    query.exec("INSERT INTO folder VALUES (2, 1, 'Unsorted', '/Unsorted', 0, 1, NULL, NULL, 'cover.jpg', 0, 3, 200, 200)");
    query.exec("INSERT INTO comic_info VALUES (1, 'hash1', 500)");
    query.exec("INSERT INTO comic VALUES (10, 2, 1, 'asm42.cbz', '/Unsorted/asm42.cbz')");
    query.exec("INSERT INTO label VALUES (1, 'To read', 0, 0)");
    query.exec("INSERT INTO comic_label VALUES (1, 1, 10, 0)");
    query.exec("INSERT INTO comic_reading_list VALUES (1, 1, 10, 0)");

    return db;
}

void writeFile(const QString &path)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write("x");
    file.close();
}

}

void OrganizeFilesTest::substitutesEveryToken()
{
    const auto entry = spiderMan();

    QCOMPARE(buildRelativePath(QStringLiteral("{publisher}/{series}/{number} {title}"), entry),
             QStringLiteral("Marvel/The Amazing Spider-Man/42 The Sinister Six.cbz"));
    QCOMPARE(buildRelativePath(QStringLiteral("{imprint}/{volume}/{count}/{year}/{month}"), entry),
             QStringLiteral("Epic/1/100/2018/07.cbz"));
    QCOMPARE(buildRelativePath(QStringLiteral("{storyArc} {arcNumber}/{writer}/{filename}"), entry),
             QStringLiteral("Sinister War 2/Dan Slott/asm42.cbz"));
}

void OrganizeFilesTest::readsTheYearAndTheMonthFromTheDateColumn()
{
    ComicEntry entry;

    applyPublicationDate(entry, QStringLiteral("1/7/2018"));
    QCOMPARE(entry.year, QStringLiteral("2018"));
    QCOMPARE(entry.month, QStringLiteral("07"));

    // Comic Vine pads its components, ComicInfo.xml does not.
    ComicEntry padded;
    applyPublicationDate(padded, QStringLiteral("01/07/2018"));
    QCOMPARE(padded.year, entry.year);
    QCOMPARE(padded.month, entry.month);

    // A missing year is stored as a 0, which is not a year.
    ComicEntry monthOnly;
    applyPublicationDate(monthOnly, QStringLiteral("1/7/0"));
    QVERIFY(monthOnly.year.isEmpty());
    QCOMPARE(monthOnly.month, QStringLiteral("07"));

    // A missing month is stored as a 1, so it cannot be told apart from
    // January. Comics with only a year get {month} 01.
    ComicEntry yearOnly;
    applyPublicationDate(yearOnly, QStringLiteral("1/1/2018"));
    QCOMPARE(yearOnly.year, QStringLiteral("2018"));
    QCOMPARE(yearOnly.month, QStringLiteral("01"));

    for (const auto &date : { QString(), QStringLiteral("2018"), QStringLiteral("//"), QStringLiteral("1/13/2018") }) {
        ComicEntry rejected;
        applyPublicationDate(rejected, date);
        QVERIFY2(rejected.month.isEmpty(), qPrintable(date));
    }

    // The tokens are the ones an empty optional group drops.
    ComicEntry undated;
    undated.baseName = QStringLiteral("scan001");
    undated.extension = QStringLiteral(".cbz");
    undated.series = QStringLiteral("Series");
    applyPublicationDate(undated, QStringLiteral("1/1/0"));
    QCOMPARE(buildRelativePath(QStringLiteral("{series}< ({year})>"), undated),
             QStringLiteral("Series.cbz"));
}

void OrganizeFilesTest::keepsPunctuationOutOfEmptyOptionalGroups()
{
    const auto entry = bareScan();

    QStringList fallbacks;
    QCOMPARE(buildRelativePath(QStringLiteral("{series}< ({year})>/<#{number}>< - {title}>"), entry, &fallbacks),
             QStringLiteral("Unknown Series/scan001.cbz"));

    QCOMPARE(buildRelativePath(QStringLiteral("{publisher}/{series}"), entry),
             QStringLiteral("Unknown Publisher/Unknown Series.cbz"));

    QVERIFY(!fallbacks.isEmpty());

    const auto complete = spiderMan();
    QCOMPARE(buildRelativePath(QStringLiteral("{series}< ({year})>/<#{number}>< - {title}>"), complete),
             QStringLiteral("The Amazing Spider-Man (2018)/#42 - The Sinister Six.cbz"));

    QCOMPARE(buildRelativePath(QStringLiteral("{publisher}/<{imprint}/>{series}"), complete),
             QStringLiteral("Marvel/Epic/The Amazing Spider-Man.cbz"));
    QCOMPARE(buildRelativePath(QStringLiteral("{publisher}/<{imprint}/>{series}"), entry),
             QStringLiteral("Unknown Publisher/Unknown Series.cbz"));
}

void OrganizeFilesTest::padsOnlyTheLeadingDigits()
{
    QCOMPARE(padNumber(QStringLiteral("42"), 3), QStringLiteral("042"));
    QCOMPARE(padNumber(QStringLiteral("42AU"), 4), QStringLiteral("0042AU"));
    QCOMPARE(padNumber(QStringLiteral("AU42"), 4), QStringLiteral("AU42"));
    QCOMPARE(padNumber(QStringLiteral("1234"), 3), QStringLiteral("1234"));

    QCOMPARE(buildRelativePath(QStringLiteral("{number:000}"), spiderMan()), QStringLiteral("042.cbz"));
}

void OrganizeFilesTest::reportsInvalidTokens()
{
    QVERIFY(invalidTokens(QStringLiteral("{series}/{number:000}")).isEmpty());
    QCOMPARE(invalidTokens(QStringLiteral("{series}/{penciller}")), QStringList { QStringLiteral("{penciller}") });
    QCOMPARE(invalidTokens(QStringLiteral("{title:000}")), QStringList { QStringLiteral("{title:000}") });
    QVERIFY(!invalidTokens(QStringLiteral("{series")).isEmpty());
    QVERIFY(!invalidTokens(QStringLiteral("{series}<({year})")).isEmpty());
}

void OrganizeFilesTest::sanitizesSegments()
{
    QCOMPARE(sanitizeSegment(QStringLiteral("a/b:c*d")), QStringLiteral("a_b_c_d"));
    QCOMPARE(sanitizeSegment(QStringLiteral("trailing dots...")), QStringLiteral("trailing dots"));
    QCOMPARE(sanitizeSegment(QStringLiteral("  spaced   out  ")), QStringLiteral("spaced out"));
    QCOMPARE(sanitizeSegment(QStringLiteral("- leading dash")), QStringLiteral("leading dash"));
    QCOMPARE(sanitizeSegment(QStringLiteral("NUL")), QStringLiteral("NUL_"));
    QCOMPARE(sanitizeSegment(QStringLiteral("com1.cbz")), QStringLiteral("com1.cbz_"));
    QCOMPARE(sanitizeSegment(QStringLiteral("Console")), QStringLiteral("Console"));
}

void OrganizeFilesTest::resolvesCollisionsAgainstDiskAndPlan()
{
    QTemporaryDir temporary;
    const QString base = temporary.path();

    writeFile(base + QStringLiteral("/Marvel/Series/001.cbz"));

    ComicEntry first;
    first.comicId = 1;
    first.sourceAbsolute = base + QStringLiteral("/in/a.cbz");
    first.extension = QStringLiteral(".cbz");
    first.baseName = QStringLiteral("a");
    first.publisher = QStringLiteral("Marvel");
    first.series = QStringLiteral("Series");
    first.number = QStringLiteral("1");

    ComicEntry second = first;
    second.comicId = 2;
    second.sourceAbsolute = base + QStringLiteral("/in/b.cbz");
    second.baseName = QStringLiteral("b");

    writeFile(first.sourceAbsolute);
    writeFile(second.sourceAbsolute);

    PlanBuilder builder({ first, second }, base, Mode::Organize);
    const auto moves = builder.build(QStringLiteral("{publisher}/{series}/{number:000}"), { });

    QCOMPARE(moves.size(), 2);
    QCOMPARE(moves.at(0).destinationRelative, QStringLiteral("Marvel/Series/001 (1).cbz"));
    QCOMPARE(moves.at(0).status, PlannedMove::Status::Renamed);
    QCOMPARE(moves.at(1).destinationRelative, QStringLiteral("Marvel/Series/001 (2).cbz"));
    QCOMPARE(moves.at(1).status, PlannedMove::Status::Renamed);
}

void OrganizeFilesTest::freesTheNameOfAFileThatMovesAway()
{
    QTemporaryDir temporary;
    const QString base = temporary.path();

    ComicEntry mover;
    mover.comicId = 1;
    mover.sourceAbsolute = base + QStringLiteral("/Series/001.cbz");
    mover.extension = QStringLiteral(".cbz");
    mover.baseName = QStringLiteral("001");
    mover.series = QStringLiteral("Series");
    mover.number = QStringLiteral("1");
    writeFile(mover.sourceAbsolute);

    PlanBuilder builder({ mover }, base, Mode::Organize);
    const auto moves = builder.build(QStringLiteral("{series}/{number:000}"), { });

    QCOMPARE(moves.size(), 1);
    QCOMPARE(moves.at(0).status, PlannedMove::Status::Unchanged);
    QCOMPARE(moves.at(0).destinationRelative, QStringLiteral("Series/001.cbz"));
}

void OrganizeFilesTest::keepsOverridesAndExclusions()
{
    QTemporaryDir temporary;
    const QString base = temporary.path();

    auto entry = spiderMan();
    entry.sourceAbsolute = base + QStringLiteral("/in/asm42.cbz");
    writeFile(entry.sourceAbsolute);

    Overrides overrides;
    overrides[entry.sourceAbsolute].destinationRelative = QStringLiteral("Renamed/By hand.cbz");

    PlanBuilder builder({ entry }, base, Mode::Organize);

    auto moves = builder.build(QStringLiteral("{publisher}/{series}/{number:000}"), overrides);
    QCOMPARE(moves.at(0).destinationRelative, QStringLiteral("Renamed/By hand.cbz"));

    moves = builder.build(QStringLiteral("{series}/{filename}"), overrides);
    QCOMPARE(moves.at(0).destinationRelative, QStringLiteral("Renamed/By hand.cbz"));

    overrides[entry.sourceAbsolute].excluded = true;
    moves = builder.build(QStringLiteral("{series}/{filename}"), overrides);
    QCOMPARE(moves.at(0).status, PlannedMove::Status::Excluded);
}

void OrganizeFilesTest::renameModeKeepsEveryFileInItsFolder()
{
    QTemporaryDir temporary;
    const QString base = temporary.path();

    ComicEntry deep;
    deep.comicId = 1;
    deep.sourceAbsolute = base + QStringLiteral("/3x3 ojos/scans/raw01.cbz");
    deep.baseName = QStringLiteral("raw01");
    deep.extension = QStringLiteral(".cbz");
    deep.series = QStringLiteral("3x3 Eyes");
    deep.number = QStringLiteral("1");
    writeFile(deep.sourceAbsolute);

    ComicEntry atBase;
    atBase.comicId = 2;
    atBase.sourceAbsolute = base + QStringLiteral("/loose.cbz");
    atBase.baseName = QStringLiteral("loose");
    atBase.extension = QStringLiteral(".cbz");
    atBase.series = QStringLiteral("Loose");
    atBase.number = QStringLiteral("7");
    writeFile(atBase.sourceAbsolute);

    PlanBuilder builder({ deep, atBase }, base, Mode::Rename);
    const auto moves = builder.build(QStringLiteral("{series}< #{number:000}>"), { });

    QCOMPARE(moves.size(), 2);

    // The folder structure is untouched; only the file name changes.
    QCOMPARE(moves.at(0).destinationRelative, QStringLiteral("3x3 ojos/scans/3x3 Eyes #001.cbz"));

    // A comic already sitting in the base gets no empty leading segment.
    QCOMPARE(moves.at(1).destinationRelative, QStringLiteral("Loose #007.cbz"));

    for (const auto &move : moves)
        QCOMPARE(QFileInfo(move.destinationRelative).path(), QFileInfo(QDir(base).relativeFilePath(move.sourceAbsolute)).path());
}

void OrganizeFilesTest::rejectsSeparatorsInAFilenamePattern()
{
    QVERIFY(patternCreatesFolders(QStringLiteral("{series}/{number:000}")));
    QVERIFY(!patternCreatesFolders(QStringLiteral("{series} #{number:000}")));

    QVERIFY(!patternCreatesFolders(defaultPattern(Mode::Rename)));
    QVERIFY(patternCreatesFolders(defaultPattern(Mode::Organize)));

    for (const auto &preset : presets(Mode::Rename)) {
        QVERIFY2(!patternCreatesFolders(preset.second), qPrintable(preset.second));
        QVERIFY2(invalidTokens(preset.second).isEmpty(), qPrintable(preset.second));
    }

    for (const auto &preset : presets(Mode::Organize))
        QVERIFY2(invalidTokens(preset.second).isEmpty(), qPrintable(preset.second));

    QVERIFY(!knownTokens().contains(QStringLiteral("folder")));
    QVERIFY(invalidTokens(defaultPattern(Mode::Rename)).isEmpty());
    QVERIFY(invalidTokens(defaultPattern(Mode::Organize)).isEmpty());
}

void OrganizeFilesTest::claimsThePathOfAnExcludedComic()
{
    QTemporaryDir temporary;
    const QString base = temporary.path();

    // The excluded comic already sits on the path the pattern gives the other one.
    // It never moves, so the other one cannot have that path.
    ComicEntry staying;
    staying.comicId = 1;
    staying.sourceAbsolute = QDir::cleanPath(base + QStringLiteral("/Series/001.cbz"));
    staying.baseName = QStringLiteral("001");
    staying.extension = QStringLiteral(".cbz");
    staying.series = QStringLiteral("Series");
    staying.number = QStringLiteral("1");

    ComicEntry moving;
    moving.comicId = 2;
    moving.sourceAbsolute = QDir::cleanPath(base + QStringLiteral("/Unsorted/loose.cbz"));
    moving.baseName = QStringLiteral("loose");
    moving.extension = QStringLiteral(".cbz");
    moving.series = QStringLiteral("Series");
    moving.number = QStringLiteral("1");

    writeFile(staying.sourceAbsolute);
    writeFile(moving.sourceAbsolute);

    Overrides overrides;
    overrides[staying.sourceAbsolute].excluded = true;

    PlanBuilder builder({ staying, moving }, base, Mode::Organize);
    const auto moves = builder.build(QStringLiteral("{series}/{number:000}"), overrides);

    QCOMPARE(moves.size(), 2);
    QCOMPARE(moves.at(0).status, PlannedMove::Status::Excluded);
    QCOMPARE(moves.at(1).status, PlannedMove::Status::Renamed);
    QCOMPARE(moves.at(1).destinationRelative, QStringLiteral("Series/001 (1).cbz"));

    // The same has to hold when the entries arrive the other way round. The old
    // single pass made the answer depend on the order of the list.
    PlanBuilder reversed({ moving, staying }, base, Mode::Organize);
    const auto reversedMoves = reversed.build(QStringLiteral("{series}/{number:000}"), overrides);

    QCOMPARE(reversedMoves.size(), 2);
    QCOMPARE(reversedMoves.at(0).status, PlannedMove::Status::Renamed);
    QCOMPARE(reversedMoves.at(0).destinationRelative, QStringLiteral("Series/001 (1).cbz"));
    QCOMPARE(reversedMoves.at(1).status, PlannedMove::Status::Excluded);
}

void OrganizeFilesTest::renamesWhenOnlyTheCaseChanges()
{
    QTemporaryDir temporary;
    const QString base = temporary.path();

    ComicEntry entry;
    entry.comicId = 1;
    entry.sourceAbsolute = QDir::cleanPath(base + QStringLiteral("/Series/spider-man 001.cbz"));
    entry.baseName = QStringLiteral("spider-man 001");
    entry.extension = QStringLiteral(".cbz");
    entry.series = QStringLiteral("Spider-Man");
    entry.number = QStringLiteral("001");

    writeFile(entry.sourceAbsolute);

    PlanBuilder builder({ entry }, base, Mode::Rename);
    const auto moves = builder.build(QStringLiteral("{series} {number}"), Overrides());

    QCOMPARE(moves.size(), 1);
    // Folded paths would call this unchanged on Windows and macOS, and fixing
    // capitalisation is a normal reason to rename.
    QCOMPARE(moves.at(0).status, PlannedMove::Status::Move);
    QCOMPARE(moves.at(0).destinationRelative, QStringLiteral("Series/Spider-Man 001.cbz"));

    // And the move itself has to go through, which a plain rename cannot do on a
    // file system that ignores case.
    QString reason;
    const QString destination = QDir::cleanPath(base + QStringLiteral("/Series/Spider-Man 001.cbz"));
    QVERIFY2(OrganizeFiles::moveFile(entry.sourceAbsolute, destination, &reason), qPrintable(reason));
    QCOMPARE(QDir(base + QStringLiteral("/Series")).entryList(QDir::Files), QStringList { QStringLiteral("Spider-Man 001.cbz") });
}

void OrganizeFilesTest::adoptsTheOnDiskCasingOfExistingFolders()
{
    QTemporaryDir temporary;
    const QString base = QDir::cleanPath(temporary.path());

    // The destination folder already exists on disk with a different casing.
    // mkpath() cannot re-case it, so the files will land in "marvel" whatever the
    // pattern says, and the plan and the database have to say the same.
    writeFile(base + QStringLiteral("/marvel/existing.cbz"));

    ComicEntry entry;
    entry.comicId = 1;
    entry.sourceAbsolute = QDir::cleanPath(base + QStringLiteral("/Unsorted/a.cbz"));
    entry.baseName = QStringLiteral("a");
    entry.extension = QStringLiteral(".cbz");
    entry.publisher = QStringLiteral("Marvel");
    entry.number = QStringLiteral("1");
    writeFile(entry.sourceAbsolute);

    PlanBuilder builder({ entry }, base, Mode::Organize);
    const auto moves = builder.build(QStringLiteral("{publisher}/{number:000}"), { });

    QCOMPARE(moves.size(), 1);
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    QCOMPARE(moves.at(0).destinationRelative, QStringLiteral("marvel/001.cbz"));
#else
    // On a case-sensitive file system "Marvel" really is a different directory.
    QCOMPARE(moves.at(0).destinationRelative, QStringLiteral("Marvel/001.cbz"));
#endif
}

void OrganizeFilesTest::mergesPlannedFolderCasingsIntoOne()
{
    QTemporaryDir temporary;
    const QString base = QDir::cleanPath(temporary.path());

    // Two casings of one new folder. On disk the second mkpath() is a no-op, so
    // only one directory appears, with the casing of whichever move ran first. The
    // plan has to agree with itself, or the database gets two folder rows for one
    // directory and the next update deletes one of them, comics included.
    ComicEntry first;
    first.comicId = 1;
    first.sourceAbsolute = QDir::cleanPath(base + QStringLiteral("/in/a.cbz"));
    first.baseName = QStringLiteral("a");
    first.extension = QStringLiteral(".cbz");
    first.publisher = QStringLiteral("Marvel");
    first.number = QStringLiteral("1");

    ComicEntry second = first;
    second.comicId = 2;
    second.sourceAbsolute = QDir::cleanPath(base + QStringLiteral("/in/b.cbz"));
    second.baseName = QStringLiteral("b");
    second.publisher = QStringLiteral("MARVEL");
    second.number = QStringLiteral("2");

    writeFile(first.sourceAbsolute);
    writeFile(second.sourceAbsolute);

    PlanBuilder builder({ first, second }, base, Mode::Organize);
    const auto moves = builder.build(QStringLiteral("{publisher}/{number:000}"), { });

    QCOMPARE(moves.size(), 2);
    QCOMPARE(moves.at(0).destinationRelative, QStringLiteral("Marvel/001.cbz"));
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    // The first appearance in the plan decides the casing.
    QCOMPARE(moves.at(1).destinationRelative, QStringLiteral("Marvel/002.cbz"));
#else
    QCOMPARE(moves.at(1).destinationRelative, QStringLiteral("MARVEL/002.cbz"));
#endif
}

void OrganizeFilesTest::ordersMovesSoNothingIsOverwritten()
{
    using OrganizeFiles::FileMove;

    // A chain: the second move has to free /b before the first can take it.
    const QList<FileMove> chain {
        { 1, QStringLiteral("/a.cbz"), QStringLiteral("/b.cbz") },
        { 2, QStringLiteral("/b.cbz"), QStringLiteral("/c.cbz") }
    };

    const auto orderedChain = OrganizeFiles::orderMoves(chain);
    QCOMPARE(orderedChain.size(), 2);
    QCOMPARE(orderedChain.at(0).move.comicId, 2ull);
    QCOMPARE(orderedChain.at(1).move.comicId, 1ull);
    QVERIFY(!orderedChain.at(0).viaTemporary);
    QVERIFY(!orderedChain.at(1).viaTemporary);

    // A swap cannot be ordered at all, so one file is parked under a temporary name
    // first. The parking move has to come first, or nothing else can proceed.
    const QList<FileMove> swap {
        { 1, QStringLiteral("/a.cbz"), QStringLiteral("/b.cbz") },
        { 2, QStringLiteral("/b.cbz"), QStringLiteral("/a.cbz") }
    };

    const auto orderedSwap = OrganizeFiles::orderMoves(swap);
    QCOMPARE(orderedSwap.size(), 2);
    QVERIFY(orderedSwap.at(0).viaTemporary);
    QVERIFY(!orderedSwap.at(1).viaTemporary);
    QCOMPARE(orderedSwap.at(1).move.comicId, 2ull);

    // Moves that have nothing to do with each other keep their order.
    const QList<FileMove> independent {
        { 1, QStringLiteral("/a.cbz"), QStringLiteral("/x.cbz") },
        { 2, QStringLiteral("/b.cbz"), QStringLiteral("/y.cbz") }
    };

    const auto orderedIndependent = OrganizeFiles::orderMoves(independent);
    QCOMPARE(orderedIndependent.size(), 2);
    QCOMPARE(orderedIndependent.at(0).move.comicId, 1ull);
    QCOMPARE(orderedIndependent.at(1).move.comicId, 2ull);
}

void OrganizeFilesTest::removesOnlyEmptyDirectoriesInsideTheBase()
{
    QTemporaryDir temporary;
    const QString root = QDir::cleanPath(temporary.path());
    const QString base = root + QStringLiteral("/base");

    QDir().mkpath(base + QStringLiteral("/empty/deeper"));
    QDir().mkpath(base + QStringLiteral("/kept"));
    QDir().mkpath(root + QStringLiteral("/outside"));
    writeFile(base + QStringLiteral("/kept/still here.cbz"));

    const auto removed = OrganizeFiles::removeEmptyDirectories(
            { base + QStringLiteral("/empty/deeper"), base + QStringLiteral("/kept"), root + QStringLiteral("/outside") },
            base);

    // The empty branch goes, and the walk up stops at the base.
    QVERIFY(!QDir(base + QStringLiteral("/empty/deeper")).exists());
    QVERIFY(!QDir(base + QStringLiteral("/empty")).exists());
    QCOMPARE(removed.size(), 2);

    QVERIFY(QDir(base).exists());
    QVERIFY(QDir(base + QStringLiteral("/kept")).exists());
    // Outside the base is none of this operation's business, even when it is empty.
    QVERIFY(QDir(root + QStringLiteral("/outside")).exists());
}

void OrganizeFilesTest::sweepsCreatedDirectoriesWhenAMoveFails()
{
    QTemporaryDir temporary;
    const QString library = QDir::cleanPath(temporary.path());

    // The first move fails: its source does not exist. The directory created for
    // it must not survive the run, or a library update right after would find a
    // folder the database knows nothing about.
    OrganizeFiles::FileMove failing;
    failing.comicId = 1;
    failing.source = library + QStringLiteral("/Unsorted/missing.cbz");
    failing.destination = library + QStringLiteral("/Marvel/Series/001.cbz");

    OrganizeFiles::FileMove moving;
    moving.comicId = 2;
    moving.source = library + QStringLiteral("/Unsorted/real.cbz");
    moving.destination = library + QStringLiteral("/DC/002.cbz");
    writeFile(moving.source);

    MoveWorker worker(library, library, { failing, moving }, true);
    worker.process();

    QCOMPARE(worker.completedMoves().size(), 1);
    QCOMPARE(worker.failures().size(), 1);

    // The created-and-unused branch is gone, the used one holds the file.
    QVERIFY(!QDir(library + QStringLiteral("/Marvel")).exists());
    QVERIFY(QFileInfo::exists(library + QStringLiteral("/DC/002.cbz")));

    // The emptied source directory was cleaned up as usual.
    QVERIFY(!QDir(library + QStringLiteral("/Unsorted")).exists());
}

void OrganizeFilesTest::undoRemovesOnlyTheDirectoriesTheRunCreated()
{
    QTemporaryDir temporary;
    const QString base = QDir::cleanPath(temporary.path());

    // The run created Marvel and Marvel/Spider-Man. It did not create Existing: that
    // folder was already there, empty, and the run only put a file in it.
    QDir().mkpath(base + QStringLiteral("/Marvel/Spider-Man"));
    QDir().mkpath(base + QStringLiteral("/Existing"));
    QDir().mkpath(base + QStringLiteral("/Marvel/Kept"));
    writeFile(base + QStringLiteral("/Marvel/Kept/other.cbz"));

    const auto removed = OrganizeFiles::removeCreatedDirectories(
            { base + QStringLiteral("/Marvel"), base + QStringLiteral("/Marvel/Spider-Man") });

    QVERIFY(!QDir(base + QStringLiteral("/Marvel/Spider-Man")).exists());
    QCOMPARE(removed.size(), 1);

    // Marvel still holds Kept, so it stays, and so does everything under it.
    QVERIFY(QDir(base + QStringLiteral("/Marvel")).exists());
    QVERIFY(QDir(base + QStringLiteral("/Marvel/Kept")).exists());

    // The folder that was there before the run is empty again, and it is still not
    // this operation's to delete.
    QVERIFY(QDir(base + QStringLiteral("/Existing")).exists());
}

void OrganizeFilesTest::movesComicRowWithoutLosingCuration()
{
    const QString connectionName = QStringLiteral("organize_move");
    {
        auto db = createDatabase(connectionName);

        const auto parentId = DBHelper::ensureFolderPath(QStringLiteral("/Marvel/The Amazing Spider-Man"), db);
        QVERIFY(parentId > 1);
        QVERIFY(DBHelper::moveComic(10, parentId, QStringLiteral("042.cbz"),
                                    QStringLiteral("/Marvel/The Amazing Spider-Man/042.cbz"), db));

        QSqlQuery query(db);
        query.exec("SELECT parentId, fileName, path FROM comic WHERE id = 10");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toULongLong(), parentId);
        QCOMPARE(query.value(1).toString(), QStringLiteral("042.cbz"));
        QCOMPARE(query.value(2).toString(), QStringLiteral("/Marvel/The Amazing Spider-Man/042.cbz"));

        query.exec("SELECT COUNT(*) FROM comic_label WHERE comic_id = 10");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);

        query.exec("SELECT COUNT(*) FROM comic_reading_list WHERE comic_id = 10");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);

        // /Unsorted is empty now that the comic moved out, so the row goes.
        QList<QVariantMap> removed;
        DBHelper::removeEmptyFolderPaths({ QStringLiteral("/Unsorted") }, db, &removed);
        QCOMPARE(removed.size(), 1);

        query.exec("SELECT COUNT(*) FROM folder WHERE path = '/Unsorted'");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 0);

        // Checked again after the folder row is gone. The cascade runs from folder
        // to comic and from comic to the curation tables, so this is the assertion
        // that says the cleanup did not reach the comic.
        query.exec("SELECT COUNT(*) FROM comic WHERE id = 10");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);

        query.exec("SELECT COUNT(*) FROM comic_label WHERE comic_id = 10");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);

        query.exec("SELECT COUNT(*) FROM comic_reading_list WHERE comic_id = 10");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void OrganizeFilesTest::keepsAFolderThatStillHoldsAComic()
{
    const QString connectionName = QStringLiteral("organize_keep_folder");
    {
        auto db = createDatabase(connectionName);

        // Comic 11 is in the library but not on disk, so the plan skips it. Its row
        // still lives in /Unsorted. When comic 10 moves out, the directory on disk
        // is empty, but the folder row is not.
        QSqlQuery seed(db);
        seed.exec("INSERT INTO comic_info VALUES (2, 'hash2', 600)");
        seed.exec("INSERT INTO comic VALUES (11, 2, 2, 'gone.cbz', '/Unsorted/gone.cbz')");
        seed.exec("INSERT INTO comic_label VALUES (2, 1, 11, 0)");

        const auto parentId = DBHelper::ensureFolderPath(QStringLiteral("/Marvel"), db);
        QVERIFY(DBHelper::moveComic(10, parentId, QStringLiteral("042.cbz"), QStringLiteral("/Marvel/042.cbz"), db));

        QList<QVariantMap> removed;
        DBHelper::removeEmptyFolderPaths({ QStringLiteral("/Unsorted") }, db, &removed);

        QVERIFY(removed.isEmpty());

        QSqlQuery query(db);
        query.exec("SELECT COUNT(*) FROM folder WHERE path = '/Unsorted'");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);

        // The whole point: the skipped comic and its label are still there.
        query.exec("SELECT COUNT(*) FROM comic WHERE id = 11");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);

        query.exec("SELECT COUNT(*) FROM comic_label WHERE comic_id = 11");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void OrganizeFilesTest::createsFolderRowsInheritingTheType()
{
    const QString connectionName = QStringLiteral("organize_folders");
    {
        auto db = createDatabase(connectionName);

        QList<qulonglong> created;
        const auto parentId = DBHelper::ensureFolderPath(QStringLiteral("/Manga/Series"), db, &created);
        QCOMPARE(created.size(), 2);

        QSqlQuery query(db);
        query.exec("SELECT type, path FROM folder WHERE id = " + QString::number(parentId));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);
        QCOMPARE(query.value(1).toString(), QStringLiteral("/Manga/Series"));

        QList<qulonglong> createdAgain;
        QCOMPARE(DBHelper::ensureFolderPath(QStringLiteral("/Manga/Series"), db, &createdAgain), parentId);
        QVERIFY(createdAgain.isEmpty());

        DBHelper::moveComic(10, parentId, QStringLiteral("a.cbz"), QStringLiteral("/Manga/Series/a.cbz"), db);
        DBHelper::syncFolderAddedFromContents({ parentId }, db);

        query.exec("SELECT added FROM folder WHERE id = " + QString::number(parentId));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toLongLong(), 500);
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void OrganizeFilesTest::removesOnlyEmptyCreatedFolderRows()
{
    const QString connectionName = QStringLiteral("organize_created_rows");
    {
        auto db = createDatabase(connectionName);

        // The run created /Marvel and /Marvel/Series and moved the comic in.
        QList<qulonglong> created;
        const auto parentId = DBHelper::ensureFolderPath(QStringLiteral("/Marvel/Series"), db, &created);
        QCOMPARE(created.size(), 2);
        QVERIFY(DBHelper::moveComic(10, parentId, QStringLiteral("042.cbz"), QStringLiteral("/Marvel/Series/042.cbz"), db));

        // While the comic is inside, the rows are not empty and must stay, even
        // when they are asked for by id.
        QList<qulonglong> reversed = created;
        std::reverse(reversed.begin(), reversed.end());
        DBHelper::removeEmptyFolderRows(reversed, db);

        QSqlQuery query(db);
        query.exec("SELECT COUNT(*) FROM folder WHERE path LIKE '/Marvel%'");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 2);

        // The undo moves the comic back. Now the created rows are empty, and the
        // undo deletes exactly them, children first.
        QVERIFY(DBHelper::moveComic(10, 2, QStringLiteral("asm42.cbz"), QStringLiteral("/Unsorted/asm42.cbz"), db));
        DBHelper::removeEmptyFolderRows(reversed, db);

        query.exec("SELECT COUNT(*) FROM folder WHERE path LIKE '/Marvel%'");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 0);

        // The folders that were there before the run are untouched.
        query.exec("SELECT COUNT(*) FROM folder");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 2);
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void OrganizeFilesTest::journalRoundTrip()
{
    QTemporaryDir temporary;
    const QString library = temporary.path();

    Journal journal(library);
    QVERIFY(journal.begin(library));
    journal.appendMove(10, library + QStringLiteral("/Unsorted/asm42.cbz"), library + QStringLiteral("/Marvel/042.cbz"));
    journal.appendRemovedDirectory(library + QStringLiteral("/Unsorted"));
    journal.finish();

    JournalData data;
    QVERIFY(Journal::read(library, journal.filePath(), &data));
    QVERIFY(data.complete);
    QCOMPARE(data.moves.size(), 1);
    QCOMPARE(data.moves.at(0).comicId, 10ull);
    QCOMPARE(data.moves.at(0).from, QStringLiteral("/Unsorted/asm42.cbz"));
    QCOMPARE(data.moves.at(0).to, QStringLiteral("/Marvel/042.cbz"));
    QCOMPARE(data.removedDirectories, QStringList { QStringLiteral("/Unsorted") });
    QCOMPARE(absoluteFromRelative(library, data.moves.at(0).to), QDir::cleanPath(library + QStringLiteral("/Marvel/042.cbz")));

    QCOMPARE(Journal::latestPath(library), journal.filePath());
}

void OrganizeFilesTest::journalCarriesTheFolderRowsItRemoved()
{
    QTemporaryDir temporary;
    const QString library = temporary.path();

    QString journalPath;

    {
        Journal journal(library);
        QVERIFY(journal.begin(library));
        journal.appendMove(10, library + QStringLiteral("/Unsorted/asm42.cbz"), library + QStringLiteral("/Marvel/042.cbz"));
        journal.finish();
        journalPath = journal.filePath();
    }

    // The database work runs after the files have moved and the journal is closed,
    // so it has to be able to add to the same record.
    {
        Journal journal(library);
        QVERIFY(journal.reopen(journalPath));

        QVariantMap row;
        row.insert(QStringLiteral("id"), 2);
        row.insert(QStringLiteral("parentId"), 1);
        row.insert(QStringLiteral("name"), QStringLiteral("Unsorted"));
        row.insert(QStringLiteral("path"), QStringLiteral("/Unsorted"));
        row.insert(QStringLiteral("type"), 3);
        row.insert(QStringLiteral("customImage"), QStringLiteral("cover.jpg"));

        journal.appendRemovedFolder(row);
        journal.appendCreatedFolder(7);
        journal.appendCreatedFolder(8);
        journal.finish();
    }

    JournalData data;
    QVERIFY(Journal::read(library, journalPath, &data));

    QCOMPARE(data.moves.size(), 1);
    QCOMPARE(data.removedFolders.size(), 1);

    // The created rows come back in creation order, so an undo can reverse the
    // list and delete children before parents.
    QCOMPARE(data.createdFolders, (QList<qulonglong> { 7, 8 }));

    const auto row = data.removedFolders.at(0);
    // The id has to survive the round trip through JSON as an integer, because it
    // goes straight back into an INTEGER PRIMARY KEY.
    QCOMPARE(row.value(QStringLiteral("id")).toULongLong(), 2ull);
    QCOMPARE(row.value(QStringLiteral("path")).toString(), QStringLiteral("/Unsorted"));
    QCOMPARE(row.value(QStringLiteral("type")).toInt(), 3);
    QCOMPARE(row.value(QStringLiteral("customImage")).toString(), QStringLiteral("cover.jpg"));
}

void OrganizeFilesTest::undoPutsTheDatabaseBackExactlyAsItWas()
{
    QTemporaryDir temporary;
    const QString library = temporary.path();

    const QString connectionName = QStringLiteral("organize_undo");
    {
        auto db = createDatabase(connectionName);

        const auto readFolder = [&db](const QString &path) {
            QSqlQuery query(db);
            query.prepare("SELECT id, parentId, name, path, type, customImage, added FROM folder WHERE path = :path");
            query.bindValue(":path", path);
            query.exec();
            QVariantList values;
            if (query.next()) {
                for (int i = 0; i < 7; ++i)
                    values << query.value(i);
            }
            return values;
        };

        const auto before = readFolder(QStringLiteral("/Unsorted"));
        QCOMPARE(before.size(), 7);

        // The run: the comic moves out and the folder it left is deleted.
        const auto destinationId = DBHelper::ensureFolderPath(QStringLiteral("/Marvel/The Amazing Spider-Man"), db);
        QVERIFY(DBHelper::moveComic(10, destinationId, QStringLiteral("042.cbz"),
                                    QStringLiteral("/Marvel/The Amazing Spider-Man/042.cbz"), db));

        QList<QVariantMap> removedFolders;
        DBHelper::removeEmptyFolderPaths({ QStringLiteral("/Unsorted") }, db, &removedFolders);
        QCOMPARE(removedFolders.size(), 1);
        QVERIFY(readFolder(QStringLiteral("/Unsorted")).isEmpty());

        // The record goes through the journal, exactly as it does in a real run, so
        // the JSON round trip is part of what is being tested here.
        Journal journal(library);
        QVERIFY(journal.begin(library));
        for (const auto &row : removedFolders)
            journal.appendRemovedFolder(row);
        journal.finish();

        JournalData data;
        QVERIFY(Journal::read(library, journal.filePath(), &data));
        QCOMPARE(data.removedFolders.size(), 1);

        // The undo.
        QVERIFY(DBHelper::restoreFolderRows(data.removedFolders, db));

        QList<qulonglong> created;
        const auto restoredId = DBHelper::ensureFolderPath(QStringLiteral("/Unsorted"), db, &created);
        // Nothing new was created: the original row is back, so the path resolves to
        // it. A new row would mean a new id, and the custom cover for this folder is
        // stored under the old one.
        QVERIFY(created.isEmpty());
        QCOMPARE(restoredId, before.at(0).toULongLong());

        QVERIFY(DBHelper::moveComic(10, restoredId, QStringLiteral("asm42.cbz"), QStringLiteral("/Unsorted/asm42.cbz"), db));

        const auto after = readFolder(QStringLiteral("/Unsorted"));
        QCOMPARE(after, before);

        QSqlQuery query(db);
        query.exec("SELECT parentId, fileName, path FROM comic WHERE id = 10");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toULongLong(), before.at(0).toULongLong());
        QCOMPARE(query.value(1).toString(), QStringLiteral("asm42.cbz"));
        QCOMPARE(query.value(2).toString(), QStringLiteral("/Unsorted/asm42.cbz"));

        query.exec("SELECT COUNT(*) FROM comic_label WHERE comic_id = 10");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);

        query.exec("SELECT COUNT(*) FROM comic_reading_list WHERE comic_id = 10");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);
    }
    QSqlDatabase::removeDatabase(connectionName);
}

QTEST_GUILESS_MAIN(OrganizeFilesTest)

#include "main.moc"
