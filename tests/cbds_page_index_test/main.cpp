#include "cbds_page_index.h"

#include <QObject>
#include <QStringList>
#include <QTest>

class CbdsPageIndexTest : public QObject
{
    Q_OBJECT

private slots:
    void detectsBooksByIniOrSuffix();
    void keepsOnlyFullSizePages();
    void returnsNamesAsListedByTheArchive();
    void ignoresNestedAndEscapingEntries();
    void fallsBackWhenThereAreNoPages();
    void leavesOtherArchivesUntouched();
    void sortsPagesByNumber();
};

namespace {

QStringList bookEntries(int pages)
{
    QStringList entries { "ComicBookDS_book.ini" };
    const QStringList folders { "IMAGE", "NAME", "SMALL_N", "SMALL_R", "THMB_N", "THMB_R" };
    for (const QString &folder : folders) {
        for (int page = 1; page <= pages; ++page) {
            entries.append(QString("%1/%2.%3").arg(folder).arg(page).arg(folder == "NAME" ? "txt" : "jpg"));
        }
    }
    return entries;
}

}

void CbdsPageIndexTest::detectsBooksByIniOrSuffix()
{
    QVERIFY(YACReaderCbds::isCbds("book.cbds", { }));
    QVERIFY(YACReaderCbds::isCbds("/comics/BOOK.CBDS", { "IMAGE/1.jpg" }));
    QVERIFY(YACReaderCbds::isCbds("renamed.cbz", { "IMAGE/1.jpg", "ComicBookDS_book.ini" }));
    QVERIFY(YACReaderCbds::isCbds("renamed.zip", { "comicbookds_book.INI" }));
    QVERIFY(YACReaderCbds::isCbds("renamed.zip", { "/ComicBookDS_book.ini" }));

    // Only the ini at the archive root marks a book.
    QVERIFY(!YACReaderCbds::isCbds("comic.cbz", { "extras/ComicBookDS_book.ini", "IMAGE/1.jpg" }));
    QVERIFY(!YACReaderCbds::isCbds("comic.cbz", { "IMAGE/1.jpg", "THMB_N/1.jpg" }));
    QVERIFY(!YACReaderCbds::isCbds("comic.cbds.cbz", { "IMAGE/1.jpg" }));

    // Backends that list directories must not turn a folder with that name into a book.
    QVERIFY(!YACReaderCbds::isCbds("comic.cbz", { "ComicBookDS_book.ini/", "IMAGE/1.jpg", "cover.jpg" }));
    QVERIFY(!YACReaderCbds::isCbds("comic.cbz", { "ComicBookDS_book.ini\\", "IMAGE/1.jpg" }));
}

void CbdsPageIndexTest::keepsOnlyFullSizePages()
{
    const QStringList entries = bookEntries(12);

    const QStringList pages = YACReaderCbds::pageCandidates("book.cbds", entries);

    QCOMPARE(pages.size(), 12);
    for (const QString &page : pages) {
        QVERIFY2(page.startsWith("IMAGE/"), qPrintable(page));
    }
    QCOMPARE(pages.count("IMAGE/1.jpg"), 1);
    QCOMPARE(pages.count("IMAGE/12.jpg"), 1);
}

void CbdsPageIndexTest::returnsNamesAsListedByTheArchive()
{
    const QStringList entries { "ComicBookDS_book.ini", "image\\2.jpg", "Image/1.jpg", "/IMAGE/3.jpg", "THMB_R\\1.jpg" };

    const QStringList pages = YACReaderCbds::pageCandidates("book.cbds", entries);

    QCOMPARE(pages, QStringList({ "image\\2.jpg", "Image/1.jpg", "/IMAGE/3.jpg" }));
    for (const QString &page : pages) {
        QVERIFY(entries.contains(page));
    }
}

void CbdsPageIndexTest::ignoresNestedAndEscapingEntries()
{
    const QStringList entries {
        "ComicBookDS_book.ini",
        "IMAGE/1.jpg",
        "IMAGE/..",
        "IMAGE/.",
        "IMAGE/",
        "../IMAGE/2.jpg",
        "IMAGE/../../outside.jpg",
        "IMAGE/sub/3.jpg",
        "IMAGE/folder.jpg/",
        "other/IMAGE/4.jpg",
        "C:/IMAGE/5.jpg",
    };

    QCOMPARE(YACReaderCbds::pageCandidates("book.cbds", entries), QStringList({ "IMAGE/1.jpg" }));
}

void CbdsPageIndexTest::fallsBackWhenThereAreNoPages()
{
    const QStringList withoutPages { "ComicBookDS_book.ini", "pages/1.jpg", "pages/2.jpg" };
    QCOMPARE(YACReaderCbds::pageCandidates("book.cbds", withoutPages), withoutPages);

    QCOMPARE(YACReaderCbds::pageCandidates("empty.cbds", { }), QStringList());
}

void CbdsPageIndexTest::leavesOtherArchivesUntouched()
{
    // A regular comic that happens to use the same folder names keeps all its images.
    QStringList entries = bookEntries(3);
    entries.removeFirst();

    QCOMPARE(YACReaderCbds::pageCandidates("comic.cbz", entries), entries);
    QCOMPARE(YACReaderCbds::pageCandidates("comic.cbr", entries), entries);
    QCOMPARE(YACReaderCbds::pageCandidates("comic.cb7", entries), entries);

    const QStringList plain { "001.jpg", "002.jpg", "ComicInfo.xml" };
    QCOMPARE(YACReaderCbds::pageCandidates("comic.cbz", plain), plain);
}

void CbdsPageIndexTest::sortsPagesByNumber()
{
    // Archive order of real books, plus a number that looks like a spread of 10 and 11.
    QStringList pages { "IMAGE/1.jpg", "IMAGE/10.jpg", "IMAGE/1011.jpg", "IMAGE/11.jpg", "IMAGE/2.jpg" };
    YACReaderCbds::sortPages(pages);
    QCOMPARE(pages, QStringList({ "IMAGE/1.jpg", "IMAGE/2.jpg", "IMAGE/10.jpg", "IMAGE/11.jpg", "IMAGE/1011.jpg" }));

    // Every spelling pageCandidates() accepts sorts by the page number, the names stay as listed.
    QStringList mixed { "/IMAGE/3.jpg", "image\\1.jpg", "IMAGE/2.jpg", "\\Image\\10.jpg" };
    YACReaderCbds::sortPages(mixed);
    QCOMPARE(mixed, QStringList({ "image\\1.jpg", "IMAGE/2.jpg", "/IMAGE/3.jpg", "\\Image\\10.jpg" }));
}

QTEST_GUILESS_MAIN(CbdsPageIndexTest)

#include "main.moc"
