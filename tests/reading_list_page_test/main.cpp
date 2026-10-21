#include "reading_list_page.h"

#include <QBuffer>
#include <QImage>
#include <QLocale>
#include <QTemporaryDir>
#include <QTest>

class ReadingListPageTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void pageHasTheListAndItsRowsInOrder();
    void onlyMissingRowsHaveTheBadge();
    void coversAreInsideThePage();
    void textIsEscaped();
    void coversAreScaledJpegs();
    void details_data();
    void details();
};

void ReadingListPageTest::initTestCase()
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}

void ReadingListPageTest::pageHasTheListAndItsRowsInOrder()
{
    const auto page = QString::fromUtf8(writeReadingListPage("Civil War", { { "First", "Civil War #1", { }, false }, { "Second", "Civil War #2", { }, true }, { "Third", { }, { }, false } }, QDate(2026, 10, 21)));
    QVERIFY(page.startsWith("<!DOCTYPE html>"));
    QVERIFY(page.contains("<title>Civil War</title>"));
    QVERIFY(page.contains("<h1>Civil War</h1>"));
    QVERIFY(page.contains("3 comic(s) · 1 missing"));
    QVERIFY(page.indexOf(">1</span>") < page.indexOf(">First<"));
    QVERIFY(page.indexOf(">First<") < page.indexOf(">Second<"));
    QVERIFY(page.indexOf(">Second<") < page.indexOf(">Third<"));
    QVERIFY(page.contains("<span class=\"position\">3</span>"));
    QVERIFY(page.contains("<div class=\"details\">Civil War #2</div>"));
    // a row without details has no empty line
    QCOMPARE(page.count("class=\"details\""), 2);
    QVERIFY(page.contains("Exported from YACReader on Wednesday, October 21, 2026"));

    // without missing entries, the summary has only the count
    const auto complete = QString::fromUtf8(writeReadingListPage("List", { { "First", { }, { }, false } }, QDate(2026, 10, 21)));
    QVERIFY(complete.contains("<p class=\"summary\">1 comic(s)</p>"));
}

void ReadingListPageTest::onlyMissingRowsHaveTheBadge()
{
    const auto page = QString::fromUtf8(writeReadingListPage("List", { { "Here", { }, { }, false }, { "Gone", { }, { }, true } }, QDate(2026, 10, 21)));
    QCOMPARE(page.count("<li class=\"missing\">"), 1);
    QCOMPARE(page.count("<span class=\"badge\">Missing</span>"), 1);
    QVERIFY(page.indexOf("<li class=\"missing\">") > page.indexOf(">Here<"));
}

void ReadingListPageTest::coversAreInsideThePage()
{
    const QByteArray jpeg("jpeg bytes");
    const auto page = QString::fromUtf8(writeReadingListPage("List", { { "With", { }, jpeg, false }, { "Without", { }, { }, false } }, QDate(2026, 10, 21)));
    QVERIFY(page.contains("<img class=\"cover\" src=\"data:image/jpeg;base64," + QString::fromLatin1(jpeg.toBase64()) + "\" alt=\"\">"));
    QCOMPARE(page.count("<div class=\"cover\"></div>"), 1);
    // nothing comes from outside the page
    QVERIFY(!page.contains("http://"));
    QVERIFY(!page.contains("https://"));
}

void ReadingListPageTest::textIsEscaped()
{
    const auto page = QString::fromUtf8(writeReadingListPage("Tom & Jerry <script>", { { "<b>Title</b>", "A & B", { }, false } }, QDate(2026, 10, 21)));
    QVERIFY(page.contains("<h1>Tom &amp; Jerry &lt;script&gt;</h1>"));
    QVERIFY(page.contains("&lt;b&gt;Title&lt;/b&gt;"));
    QVERIFY(page.contains("A &amp; B"));
    QVERIFY(!page.contains("<script>"));
}

void ReadingListPageTest::coversAreScaledJpegs()
{
    QTemporaryDir dir;
    const auto path = dir.filePath("cover.png");
    QImage large(400, 600, QImage::Format_RGB32);
    large.fill(Qt::red);
    QVERIFY(large.save(path));

    const auto jpeg = readingListPageCover(path);
    QVERIFY(jpeg.startsWith("\xFF\xD8"));
    QCOMPARE(QImage::fromData(jpeg).size(), QSize(112, 168));

    // a small cover keeps its size
    QImage small(50, 75, QImage::Format_RGB32);
    small.fill(Qt::blue);
    QVERIFY(small.save(path));
    QCOMPARE(QImage::fromData(readingListPageCover(path)).size(), QSize(50, 75));

    QVERIFY(readingListPageCover(dir.filePath("none.jpg")).isEmpty());
}

void ReadingListPageTest::details_data()
{
    QTest::addColumn<QString>("series");
    QTest::addColumn<QString>("number");
    QTest::addColumn<QString>("volume");
    QTest::addColumn<QString>("date");
    QTest::addColumn<QString>("details");

    QTest::newRow("everything") << "Civil War" << "1" << "2006" << "5/7/2006" << "Civil War #1 (2006) · Jul 2006";
    QTest::newRow("a year alone") << "Civil War" << "1" << "" << "//2006" << "Civil War #1 · 2006";
    QTest::newRow("no date") << "Civil War" << "1" << "Vol. 2" << "" << "Civil War #1 (Vol. 2)";
    QTest::newRow("no series") << "" << "1" << "" << "" << "#1";
    QTest::newRow("only a date") << "" << "" << "" << "1/8/2006" << "Aug 2006";
    QTest::newRow("nothing") << "" << "" << "" << "" << "";
}

void ReadingListPageTest::details()
{
    QFETCH(QString, series);
    QFETCH(QString, number);
    QFETCH(QString, volume);
    QFETCH(QString, date);
    QFETCH(QString, details);
    QCOMPARE(readingListPageDetails(series, number, volume, date), details);
}

QTEST_MAIN(ReadingListPageTest)

#include "main.moc"
