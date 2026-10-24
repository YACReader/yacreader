#include "comic.h"
#include "comic_db.h"
#include "yacreader_global.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

class ComicStartPageTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanupTestCase();
    void choosesTheStartPage_data();
    void choosesTheStartPage();
    void choosesTheStartPageOfLibraryComics_data();
    void choosesTheStartPageOfLibraryComics();

private:
    QTemporaryDir comicFolder;
};

namespace {

constexpr int pageCount = 5;
constexpr int lastPage = pageCount - 1;
constexpr int atSavedProgress = -1;

QSize pageSize(int page)
{
    return QSize(10 * (page + 1), 20 * (page + 1));
}

QString bookmarksPath()
{
    return YACReader::getSettingsPath() + "/bookmarks.yacr";
}

// the page reported to Render, or -1 when it was not reported exactly once
int startPageOf(Comic &comic)
{
    QSignalSpy openAt(&comic, &Comic::openAt);
    comic.process();
    return openAt.count() == 1 ? openAt.at(0).at(0).toInt() : -1;
}

class LibraryComicProbe : public FileComic
{
public:
    using Comic::resolveStartPage;
    void setPageCount(int count) { _pages.resize(count); }
    int startPage() const { return _firstPage; }
};

}

void ComicStartPageTest::initTestCase()
{
    // keeps the bookmarks file that Comic reads and writes out of the user's data
    QStandardPaths::setTestModeEnabled(true);
    QVERIFY(QDir().mkpath(YACReader::getSettingsPath()));

    QVERIFY(comicFolder.isValid());
    for (int page = 0; page < pageCount; ++page) {
        QImage image(pageSize(page), QImage::Format_RGB32);
        image.fill(Qt::white);
        QVERIFY(image.save(comicFolder.filePath(QStringLiteral("%1.png").arg(page + 1))));
    }
}

void ComicStartPageTest::init()
{
    QFile::remove(bookmarksPath());
}

void ComicStartPageTest::cleanupTestCase()
{
    QFile::remove(bookmarksPath());
}

void ComicStartPageTest::choosesTheStartPage_data()
{
    QTest::addColumn<int>("requestedPage");
    QTest::addColumn<int>("savedPage");
    QTest::addColumn<int>("expectedPage");

    QTest::newRow("saved on a middle page") << atSavedProgress << 2 << 2;
    QTest::newRow("saved on the last page") << atSavedProgress << lastPage << lastPage;
    QTest::newRow("saved past the last page") << atSavedProgress << pageCount << 0;
    QTest::newRow("saved before the first page") << atSavedProgress << -1 << 0;
    QTest::newRow("requested middle page") << 2 << 3 << 2;
    QTest::newRow("requested page past the end") << pageCount << 2 << 0;
}

void ComicStartPageTest::choosesTheStartPage()
{
    QFETCH(int, requestedPage);
    QFETCH(int, savedPage);
    QFETCH(int, expectedPage);

    FolderComic comic;
    QVERIFY(comic.load(comicFolder.path(), requestedPage));
    comic.bm->setLastPage(savedPage, QImage());

    QCOMPARE(startPageOf(comic), expectedPage);
    QVERIFY(!comic.hasBeenAnErrorOpening());
    QCOMPARE(comic.numPages(), static_cast<unsigned int>(pageCount));
}

void ComicStartPageTest::choosesTheStartPageOfLibraryComics_data()
{
    QTest::addColumn<int>("currentPage");
    QTest::addColumn<int>("expectedPage");

    QTest::newRow("on a middle page") << 3 << 2;
    QTest::newRow("finished") << pageCount << lastPage;
    QTest::newRow("removed from the Reading list") << 0 << 0;
}

void ComicStartPageTest::choosesTheStartPageOfLibraryComics()
{
    QFETCH(int, currentPage);
    QFETCH(int, expectedPage);

    ComicDB comicDB;
    comicDB.info.currentPage = currentPage;

    LibraryComicProbe comic;
    QVERIFY(comic.load(comicFolder.filePath("1.png"), comicDB));
    comic.setPageCount(pageCount);
    comic.resolveStartPage();

    QCOMPARE(comic.startPage(), expectedPage);
}

QTEST_GUILESS_MAIN(ComicStartPageTest)

#include "main.moc"
