#include "comic.h"
#include "comic_db.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPdfWriter>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

class ComicFirstPageTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void opensFolderComics_data();
    void opensFolderComics();
    void loadsTheLastReadPage_data();
    void loadsTheLastReadPage();
#ifndef NO_PDF
    void opensLibraryPdfs_data();
    void opensLibraryPdfs();
#endif

private:
    QTemporaryDir dir;
    QString folderPath;
    QString archivePath;
    QString pdfPath;
};

namespace {

constexpr int pageCount = 5;
constexpr int atSavedPage = -1;

QList<int> openedPages(Comic &comic)
{
    QSignalSpy openAt(&comic, &Comic::openAt);
    comic.process();

    QList<int> pages;
    for (const QList<QVariant> &arguments : std::as_const(openAt)) {
        pages << arguments.at(0).toInt();
    }
    return pages;
}

void writeBlankPdf(const QString &path)
{
    QPdfWriter pdf(path);
    QPainter painter(&pdf);
    for (int page = 1; page < pageCount; ++page) {
        pdf.newPage();
    }
}

}

void ComicFirstPageTest::initTestCase()
{
    // every Comic reads bookmarks.yacr from the settings path
    QStandardPaths::setTestModeEnabled(true);

    QVERIFY(dir.isValid());
    folderPath = dir.filePath("folder");
    archivePath = dir.filePath("comic.cbz");
    pdfPath = dir.filePath("comic.pdf");

    QVERIFY(QDir().mkpath(folderPath));
    QImage page(10, 10, QImage::Format_RGB32);
    page.fill(Qt::white);
    for (int number = 1; number <= pageCount; ++number) {
        QVERIFY(page.save(dir.filePath(QStringLiteral("folder/%1.png").arg(number))));
    }

    // FileComic::load only checks that the file exists
    QVERIFY(QFile(archivePath).open(QIODevice::WriteOnly));

    writeBlankPdf(pdfPath);
}

void ComicFirstPageTest::opensFolderComics_data()
{
    QTest::addColumn<int>("requestedPage");
    QTest::addColumn<int>("savedPage");
    QTest::addColumn<int>("expectedPage");

    QTest::newRow("saved on a middle page") << atSavedPage << 2 << 2;
    QTest::newRow("saved on the last page") << atSavedPage << pageCount - 1 << pageCount - 1;
    QTest::newRow("saved past the last page") << atSavedPage << pageCount << 0;
    QTest::newRow("saved before the start") << atSavedPage << -1 << 0;
    QTest::newRow("requested middle page") << 2 << 3 << 2;
    QTest::newRow("requested past the last page") << pageCount << 2 << 0;
    QTest::newRow("requested before the start") << -2 << 2 << 0;
}

void ComicFirstPageTest::opensFolderComics()
{
    QFETCH(int, requestedPage);
    QFETCH(int, savedPage);
    QFETCH(int, expectedPage);

    FolderComic comic;
    QVERIFY(comic.load(folderPath, requestedPage));
    comic.bm->setLastPage(savedPage, QImage());

    QCOMPARE(openedPages(comic), QList<int> { expectedPage });
}

void ComicFirstPageTest::loadsTheLastReadPage_data()
{
    // currentPage counts from 1, pages from 0
    QTest::addColumn<int>("currentPage");
    QTest::addColumn<int>("expectedLastPage");

    QTest::newRow("on page 3") << 3 << 2;
    QTest::newRow("removed from the Reading special list") << 0 << 0;
    QTest::newRow("negative progress from a sync client") << -5 << 0;
}

void ComicFirstPageTest::loadsTheLastReadPage()
{
    QFETCH(int, currentPage);
    QFETCH(int, expectedLastPage);

    ComicDB comicDB;
    comicDB.info.currentPage = currentPage;

    FileComic fileComic;
    QVERIFY(fileComic.load(archivePath, comicDB));
    QCOMPARE(fileComic.bm->getLastPage(), expectedLastPage);

#ifndef NO_PDF
    PDFComic pdfComic;
    QVERIFY(pdfComic.load(pdfPath, comicDB));
    QCOMPARE(pdfComic.bm->getLastPage(), expectedLastPage);
#endif
}

#ifndef NO_PDF
void ComicFirstPageTest::opensLibraryPdfs_data()
{
    QTest::addColumn<int>("currentPage");
    QTest::addColumn<int>("expectedPage");

    QTest::newRow("on page 3") << 3 << 2;
    QTest::newRow("finished") << pageCount << pageCount - 1;
    QTest::newRow("removed from the Reading special list") << 0 << 0;
    QTest::newRow("negative progress from a sync client") << -5 << 0;
    QTest::newRow("progress past the last page") << pageCount + 1 << 0;
}

void ComicFirstPageTest::opensLibraryPdfs()
{
    QFETCH(int, currentPage);
    QFETCH(int, expectedPage);

    ComicDB comicDB;
    comicDB.info.currentPage = currentPage;

    PDFComic comic;
    QVERIFY(comic.load(pdfPath, comicDB));

    QCOMPARE(openedPages(comic), QList<int> { expectedPage });
}
#endif

QTEST_GUILESS_MAIN(ComicFirstPageTest)

#include "main.moc"
