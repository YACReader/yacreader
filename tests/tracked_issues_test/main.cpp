#include "data_base_management.h"
#include "tracked_issues.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTest>

class TrackedIssuesTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void issues_data();
    void issues();
    void invalidIssues();
    void tooManyIssues();
    void repeatedIssuesAreCountedOnce();
    void compactIssuesJoinRuns();
    void gapsBecomeRanges();
    void folderNumbersAndRun();
    void trackingSkipsTheNumbersOfTheFolder();

private:
    bool exec(const QString &sql);
    QList<QVariantList> rows(const QString &sql);

    QSqlDatabase db;
};

void TrackedIssuesTest::init()
{
    db = QSqlDatabase::addDatabase("QSQLITE", "tracked_issues_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());
    QVERIFY(exec("PRAGMA foreign_keys = ON"));
    QVERIFY(DataBaseManagement::createTables(db));
    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/'), (2, 1, '52', '/52')"));
    QVERIFY(exec("INSERT INTO comic_info (id, hash, series, volume, publisher, number) VALUES "
                 "(1, 'h1', '52', '2006', 'DC', '1'), (2, 'h2', '52', '2006', 'DC', '002'), (3, 'h3', '52 Aftermath', NULL, 'DC', '5')"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (1, 2, 1, 'a.cbz', '/52/a.cbz'), (2, 2, 2, 'b.cbz', '/52/b.cbz'), (3, 2, 3, 'c.cbz', '/52/c.cbz')"));
    QVERIFY(exec("INSERT INTO placeholder (id, number) VALUES (7, '7')"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 7)"));
}

void TrackedIssuesTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("tracked_issues_test");
}

bool TrackedIssuesTest::exec(const QString &sql)
{
    QSqlQuery query(db);
    return query.exec(sql);
}

QList<QVariantList> TrackedIssuesTest::rows(const QString &sql)
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

void TrackedIssuesTest::issues_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QStringList>("numbers");

    QTest::newRow("one") << "12" << QStringList({ "12" });
    QTest::newRow("range") << "3-6" << QStringList({ "3", "4", "5", "6" });
    QTest::newRow("range with spaces") << " 3 - 5 " << QStringList({ "3", "4", "5" });
    QTest::newRow("leading zeros") << "008-010" << QStringList({ "8", "9", "10" });
    QTest::newRow("list") << "1-2, 8, 0.5, Annual 1" << QStringList({ "1", "2", "8", "0.5", "Annual 1" });
    QTest::newRow("negative") << "-1, 0" << QStringList({ "-1", "0" });
    QTest::newRow("text with a dash") << "1-A" << QStringList({ "1-A" });
    QTest::newRow("decimals are one issue") << "1.5-2" << QStringList({ "1.5-2" });
    QTest::newRow("single range") << "4-4" << QStringList({ "4" });
    QTest::newRow("empty") << "  " << QStringList();
}

void TrackedIssuesTest::issues()
{
    QFETCH(QString, text);
    QFETCH(QStringList, numbers);
    const auto issues = parseTrackedIssues(text);
    QVERIFY(issues.valid);
    QCOMPARE(issues.numbers, numbers);
}

void TrackedIssuesTest::invalidIssues()
{
    QVERIFY(!parseTrackedIssues("10-5").valid);
    QVERIFY(!parseTrackedIssues("1,,3").valid);
    QVERIFY(!parseTrackedIssues("1, ").valid);
}

void TrackedIssuesTest::tooManyIssues()
{
    QCOMPARE(parseTrackedIssues("1-3000").numbers.size(), 3000);
    QVERIFY(!parseTrackedIssues("1-3000").tooMany);
    QVERIFY(!tooManyTrackedIssues("1-3000, , 7-5"));
    for (const auto *text : { "1-3001", "1-9999", "1-2000, 2001-3001", "1-3000, Annual 1", "1-99999999999999999999" }) {
        const auto issues = parseTrackedIssues(text);
        QVERIFY2(issues.tooMany, text);
        QVERIFY(!issues.valid);
        QVERIFY(issues.numbers.isEmpty());
        QVERIFY2(tooManyTrackedIssues(text), text);
    }
}

void TrackedIssuesTest::repeatedIssuesAreCountedOnce()
{
    const auto issues = parseTrackedIssues("1-5, 3, 05, Annual 1, annual 1");
    QCOMPARE(issues.numbers, QStringList({ "1", "2", "3", "4", "5", "Annual 1" }));
    QCOMPARE(issues.repeated, 3);
}

void TrackedIssuesTest::compactIssuesJoinRuns()
{
    QCOMPARE(compactIssues({ "40", "41", "42", "50", "0.5", "Annual 1" }), QString("40\u201342, 50, 0.5, Annual 1"));
    QCOMPARE(compactIssues({ "1", "3", "5" }, 2), QString("1, 3, \u2026"));
}

void TrackedIssuesTest::gapsBecomeRanges()
{
    QCOMPARE(missingIssueRanges({ "1", "2", "5", "007", "Annual 1", "0.5" }), QString("3-4, 6"));
    QCOMPARE(missingIssueRanges({ "2" }), QString("1"));
    QCOMPARE(missingIssueRanges({ "1", "2" }), QString());
    QCOMPARE(missingIssueRanges({ }), QString());
    QCOMPARE(missingIssueRanges({ "3001" }), QString("1-3000"));
    QCOMPARE(missingIssueRanges({ "3002" }), QString());
    QCOMPARE(missingIssueRanges({ "1", "999999999" }), QString());
}

void TrackedIssuesTest::folderNumbersAndRun()
{
    auto numbers = folderIssueNumbers(FolderId(2), db);
    QVERIFY(numbers);
    numbers->sort();
    QCOMPARE(*numbers, QStringList({ "002", "1", "5", "7" }));

    const auto run = folderRunMetadata(FolderId(2), db);
    QCOMPARE(run.series.toString(), QString("52"));
    QCOMPARE(run.volume.toString(), QString("2006"));
    QCOMPARE(run.publisher.toString(), QString("DC"));
}

void TrackedIssuesTest::trackingSkipsTheNumbersOfTheFolder()
{
    DescriptiveMetadataPatch patch;
    patch.set(&DescriptiveComicMetadata::series, "52");
    patch.set(&DescriptiveComicMetadata::writer, "Writer");

    QCOMPARE(trackIssuesInFolder(FolderId(2), { "1", "2", "3", "7", "Annual 1" }, patch, std::nullopt, db), std::optional(2));
    QCOMPARE(rows("SELECT p.series, p.number, p.writer FROM placeholder p INNER JOIN folder_placeholder fp ON fp.placeholder_id = p.id "
                  "WHERE fp.folder_id = 2 AND p.id <> 7 ORDER BY p.id"),
             QList<QVariantList>({ { "52", "3", "Writer" }, { "52", "Annual 1", "Writer" } }));

    QCOMPARE(trackIssuesInFolder(FolderId(2), { "4" }, patch, QString("cover4"), db), std::optional(1));
    QCOMPARE(rows("SELECT coverKey FROM placeholder WHERE number = '4'"), QList<QVariantList>({ { "cover4" } }));
}

QTEST_GUILESS_MAIN(TrackedIssuesTest)
#include "main.moc"
