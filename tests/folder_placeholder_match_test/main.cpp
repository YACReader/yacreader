#include "comic_db.h"
#include "data_base_management.h"
#include "folder_placeholder_match.h"
#include "library_comic_match.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

class FolderPlaceholderMatchTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void numbersCompareByValue();
    void comicVineIdComesFromTheNotesToo();
    void kavitaRules_data();
    void kavitaRules();
    void patternIsLearnedFromTheFolder();
    void patternNeedsThreeComicsThatAgree();
    void comicVineIdMatches();
    void seriesAndNumberMatch();
    void metadataNumberWithoutAMissingComicIsNotMatchedByName();
    void fileNameMatchesWithTheLearnedPattern();
    void fileNameOutsideTheLearnedPatternIsNotMatched();
    void fileNameMatchesWithKavitaRulesWithoutAPattern();
    void twoCandidatesAreNotMatched();
    void onlyMissingComicsOfTheFolderAreCandidates();
    void oneMatcherLearnsTheScanAsItGoes();

private:
    bool exec(const QString &sql);
    QList<QVariantList> rows(const QString &sql);
    // a new comic of the folder; `metadata` are "column = value" assignments for its comic_info
    ComicDB newComic(qulonglong folderId, const QString &fileName, const QString &metadata = QString());
    std::optional<PlaceholderId> match(const ComicDB &comic);

    std::unique_ptr<QTemporaryDir> root;
    QSqlDatabase db;
    int nextId = 100;
};

void FolderPlaceholderMatchTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    db = QSqlDatabase::addDatabase("QSQLITE", "folder_placeholder_match_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());
    QVERIFY(exec("PRAGMA foreign_keys = ON"));
    QVERIFY(DataBaseManagement::createTables(db));
    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/'), (2, 1, '52', '/52'), (3, 1, 'Other', '/Other')"));
    QVERIFY(exec("INSERT INTO placeholder (id, series, number, comicVineID) VALUES (40, '52', '40', '1040'), (41, '52', '41', '1041'), (42, '52', '42', NULL)"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 40), (2, 41), (2, 42)"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'Weekly')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 0, 40), (1, 1, 41), (1, 2, 42)"));
}

void FolderPlaceholderMatchTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("folder_placeholder_match_test");
    root.reset();
}

bool FolderPlaceholderMatchTest::exec(const QString &sql)
{
    QSqlQuery query(db);
    return query.exec(sql);
}

QList<QVariantList> FolderPlaceholderMatchTest::rows(const QString &sql)
{
    QList<QVariantList> result;
    QSqlQuery query(db);
    if (!query.exec(sql))
        return result;
    while (query.next()) {
        QVariantList row;
        for (int i = 0; i < query.record().count(); i++)
            row.append(query.value(i));
        result.append(row);
    }
    return result;
}

ComicDB FolderPlaceholderMatchTest::newComic(qulonglong folderId, const QString &fileName, const QString &metadata)
{
    const auto id = nextId++;
    exec(QString("INSERT INTO comic_info (id, hash) VALUES (%1, 'hash%1')").arg(id));
    if (!metadata.isEmpty())
        exec(QString("UPDATE comic_info SET %1 WHERE id = %2").arg(metadata).arg(id));
    QSqlQuery insert(db);
    insert.prepare("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (:id, :folder_id, :id, :name, :path)");
    insert.bindValue(":id", id);
    insert.bindValue(":folder_id", folderId);
    insert.bindValue(":name", fileName);
    insert.bindValue(":path", "/52/" + fileName);
    insert.exec();

    QSqlQuery select(db);
    select.prepare("SELECT " + ComicDB::selectList("c", "ci") + " FROM comic c INNER JOIN comic_info ci ON ci.id = c.comicInfoId WHERE c.id = :id");
    select.bindValue(":id", id);
    select.exec();
    select.next();
    return ComicDB::fromQuery(select, ComicDB::columnIndexes(select.record(), "c", "ci"));
}

std::optional<PlaceholderId> FolderPlaceholderMatchTest::match(const ComicDB &comic)
{
    ComicMatchFiles files;
    return MissingComicMatcher().match(comic, root->path(), db, files);
}

void FolderPlaceholderMatchTest::numbersCompareByValue()
{
    QVERIFY(sameIssueNumber("012", "12"));
    QVERIFY(sameIssueNumber(" 7 ", "007"));
    QVERIFY(sameIssueNumber("0", "000"));
    QVERIFY(sameIssueNumber("12a", "12A"));
    QVERIFY(!sameIssueNumber("12", "120"));
    QVERIFY(!sameIssueNumber("1", "1.5"));
}

void FolderPlaceholderMatchTest::comicVineIdComesFromTheNotesToo()
{
    DescriptiveComicMetadata metadata;
    metadata.notes = "Tagged with ComicTagger 1.6.0 using info from Comic Vine on 2024-01-01. [Issue ID 734524] Scraped metadata from ComicVine [CVDB734524]";
    QCOMPARE(comicVineIdOf(metadata), QString("734524"));
    metadata.comicVineID = "1000";
    QCOMPARE(comicVineIdOf(metadata), QString("1000"));
}

void FolderPlaceholderMatchTest::kavitaRules_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("number");

    QTest::newRow("of") << "Batman & Wildcat (1 of 3).cbz" << "1";
    QTest::newRow("of in brackets") << "Batman Beyond 04 (of 6) (1999).cbz" << "04";
    QTest::newRow("series with a decimal") << "Batman Beyond 2.0 001 (2013).cbz" << "001";
    QTest::newRow("volume") << "Teen Titans v1 001 (1966-02) (digital) (OkC.O.M.P.U.T.O.-Novus).cbz" << "001";
    QTest::newRow("hash") << "Batman & Robin the Teen Wonder #0.cbz" << "0";
    QTest::newRow("hash after volume") << "Batgirl Vol.2000 #57 (December, 2004).cbz" << "57";
    QTest::newRow("issue") << "Batman 2016 - Issue #01.cbz" << "01";
    QTest::newRow("chapter") << "Blade Runner 2019 - Ch. 01.cbz" << "01";
    QTest::newRow("before a title") << "Invincible 070.5 - Invincible Returns 1 (2010) (digital) (Minutemen-InnerDemons).cbr" << "070.5";
    QTest::newRow("after a year") << "Batman & Grendel (1996) 01 - Devil's Bones.cbz" << "01";
    QTest::newRow("before a year") << "Saga 001 (2012) (Digital) (Empire-Zone).cbr" << "001";
    QTest::newRow("dash") << "spawn-123.cbz" << "123";
    QTest::newRow("underscores") << "Batman_012_(2016).cbz" << "012";
    QTest::newRow("week") << "52 - Semana 05 - por Wild(CRG).cbr" << "05";
    QTest::newRow("week with tags") << "52 - Semana 18 [por elerober][CRG-FPJ].cbr" << "18";
    QTest::newRow("annual") << "Batman Annual 2 (2017).cbz" << "";
    QTest::newRow("one-shot") << "Batman One-Shot 1.cbz" << "";
    QTest::newRow("tpb") << "Saga TPB 01.cbz" << "";
    QTest::newRow("special marker") << "Batman SP01.cbz" << "";
    QTest::newRow("no number") << "Batman.cbz" << "";
}

void FolderPlaceholderMatchTest::kavitaRules()
{
    QFETCH(QString, fileName);
    QFETCH(QString, number);
    QCOMPARE(issueNumberInFileName(fileName).value_or(QString()), number);
}

void FolderPlaceholderMatchTest::patternIsLearnedFromTheFolder()
{
    const FileNamePattern pattern({
            { "52 - Semana 05 - por Wild(CRG).cbr", "5" },
            { "52 - Semana 06- por AsartA(CRG).cbr", "6" },
            { "52 - Semana 1- por Jiman(CRG).cbr", "1" },
            { "52 - Semana 10 corregido [por Jiman][CRG].cbr", "10" },
            { "52 - Semana 17 - Jiman(CRG - FPJ).cbr", "17" },
            { "52 - Semana 26  por Wild(CRG).cbr", "26" },
    });
    QVERIFY(pattern.isLearned());
    QCOMPARE(pattern.issueNumber("52 - Semana 40 [por elerober][CRG-FPJ].cbr").value_or(QString()), QString("40"));
    QCOMPARE(pattern.issueNumber("52_-_semana_041.cbz").value_or(QString()), QString("041"));
    QVERIFY(!pattern.issueNumber("52 - Especial 1.cbr"));
    QVERIFY(!pattern.issueNumber("Semana 40.cbr"));
}

void FolderPlaceholderMatchTest::patternNeedsThreeComicsThatAgree()
{
    QVERIFY(!FileNamePattern({ { "Batman 001.cbz", "1" }, { "Batman 002.cbz", "2" } }).isLearned());
    QVERIFY(FileNamePattern({ { "Batman 001.cbz", "1" }, { "Batman 002.cbz", "2" }, { "Batman 003.cbz", "3" } }).isLearned());
    // a comic with another number after the same prefix
    QVERIFY(!FileNamePattern({ { "Batman 001.cbz", "1" }, { "Batman 002.cbz", "2" }, { "Batman 003.cbz", "3" }, { "Batman 004.cbz", "40" } }).isLearned());
}

void FolderPlaceholderMatchTest::comicVineIdMatches()
{
    const auto comic = newComic(2, "whatever.cbz", "notes = 'Scraped metadata from ComicVine [CVDB1041]', number = '41'");
    QCOMPARE(match(comic), std::optional(PlaceholderId(41)));
    QCOMPARE(rows("SELECT placeholder_id FROM folder_placeholder ORDER BY placeholder_id"), QList<QVariantList>({ { 40 }, { 42 } }));
    QCOMPARE(rows("SELECT comic_info_id FROM reading_list_entry WHERE ordering = 1"), QList<QVariantList>({ { comic.info.id.value() } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder WHERE id = 41"), QList<QVariantList>({ { 0 } }));
}

void FolderPlaceholderMatchTest::seriesAndNumberMatch()
{
    const auto comic = newComic(2, "whatever.cbz", "series = '52', number = '042'");
    QCOMPARE(match(comic), std::optional(PlaceholderId(42)));
    QVERIFY(!match(newComic(2, "other.cbz", "series = 'Countdown', number = '40'")));
}

void FolderPlaceholderMatchTest::metadataNumberWithoutAMissingComicIsNotMatchedByName()
{
    QVERIFY(!match(newComic(2, "52 - Semana 40.cbr", "number = '7'")));
    QCOMPARE(rows("SELECT COUNT(*) FROM folder_placeholder"), QList<QVariantList>({ { 3 } }));
}

void FolderPlaceholderMatchTest::fileNameMatchesWithTheLearnedPattern()
{
    for (const auto number : { 5, 6, 7 })
        newComic(2, QString("52 - Semana %1 - por Wild(CRG).cbr").arg(number, 2, 10, QChar('0')), QString("number = '%1'").arg(number));

    QCOMPARE(match(newComic(2, "52 - Semana 41 [por elerober][CRG-FPJ].cbr")), std::optional(PlaceholderId(41)));
}

void FolderPlaceholderMatchTest::fileNameOutsideTheLearnedPatternIsNotMatched()
{
    for (const auto number : { 5, 6, 7 })
        newComic(2, QString("52 - Semana %1.cbr").arg(number), QString("number = '%1'").arg(number));

    // Kavita's rules would read 41, but the folder names its issues in another way
    QVERIFY(!match(newComic(2, "Countdown 41.cbr")));
}

void FolderPlaceholderMatchTest::fileNameMatchesWithKavitaRulesWithoutAPattern()
{
    QCOMPARE(match(newComic(2, "52 #40 (2007).cbz")), std::optional(PlaceholderId(40)));
    QVERIFY(!match(newComic(2, "52 Annual 41.cbz")));
}

void FolderPlaceholderMatchTest::twoCandidatesAreNotMatched()
{
    QVERIFY(exec("INSERT INTO placeholder (id, series, number) VALUES (43, '52', '40')"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 43)"));

    QVERIFY(!match(newComic(2, "52 #40.cbz")));
    QVERIFY(!match(newComic(2, "x.cbz", "series = '52', number = '40'")));
    // the Comic Vine id still tells them apart
    QCOMPARE(match(newComic(2, "y.cbz", "comicVineID = '1040', number = '40'")), std::optional(PlaceholderId(40)));
}

void FolderPlaceholderMatchTest::onlyMissingComicsOfTheFolderAreCandidates()
{
    QVERIFY(!match(newComic(3, "52 #40.cbz", "comicVineID = '1040'")));
    QCOMPARE(rows("SELECT COUNT(*) FROM folder_placeholder"), QList<QVariantList>({ { 3 } }));
}

void FolderPlaceholderMatchTest::oneMatcherLearnsTheScanAsItGoes()
{
    for (const auto number : { 5, 6 })
        newComic(2, QString("52 - Semana %1.cbr").arg(number, 2, 10, QChar('0')), QString("number = '%1'").arg(number));

    MissingComicMatcher matcher;
    ComicMatchFiles files;
    // two comics of the folder do not make a pattern yet
    QVERIFY(!matcher.match(newComic(2, "Extra.cbr"), root->path(), db, files));
    // a new comic of the scan is the third one
    QVERIFY(!matcher.match(newComic(2, "52 - Semana 07.cbr", "number = '7'"), root->path(), db, files));
    // Kavita's rules would read 41, but the folder now has a pattern
    QVERIFY(!matcher.match(newComic(2, "Countdown 41.cbr"), root->path(), db, files));
    QCOMPARE(matcher.match(newComic(2, "52 - Semana 40.cbr"), root->path(), db, files), std::optional(PlaceholderId(40)));
    // a matched missing comic is not a candidate again
    QVERIFY(!matcher.match(newComic(2, "x.cbz", "series = '52', number = '40'"), root->path(), db, files));
}

QTEST_GUILESS_MAIN(FolderPlaceholderMatchTest)
#include "main.moc"
