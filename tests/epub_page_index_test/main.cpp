#include "epub_page_index.h"

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QStringList>
#include <QTest>

class EpubPageIndexTest : public QObject
{
    Q_OBJECT

private slots:
    void followsSpineOrderAndMetadata();
    void resolvesRelativeAndEncodedPaths();
    void readsObjectWrapper();
    void toleratesHtmlNamedEntities();
    void preservesDuplicateSpineReferences();
    void skipsBrokenSpineItems();
    void rejectsWrappersWithMultipleImages();
    void rejectsPathsOutsideTheArchive();
    void rejectsIllustratedTextBooks();
    void rejectsPagesWithBodyText();
    void ignoresStyleScriptAndHiddenText();
    void ignoresNavigationDocuments();
    void toleratesAFewNonImagePages();
    void scanInfoMatchesThePageIndex();
};

namespace {

YACReaderEpub::PageIndex readIndex(const QStringList &fileNames, const QHash<QString, QByteArray> &files)
{
    return YACReaderEpub::readPageIndex(fileNames, [&](int index) { return files.value(fileNames.at(index)); });
}

YACReaderEpub::ScanInfo readScan(const QStringList &fileNames, const QHash<QString, QByteArray> &files, int coverPage = 1)
{
    return YACReaderEpub::readScanInfo(
            fileNames, [&](int index) { return files.value(fileNames.at(index)); }, coverPage, { });
}

QByteArray containerXml()
{
    return R"(<?xml version="1.0"?>
        <container xmlns="urn:oasis:names:tc:opendocument:xmlns:container">
          <rootfiles>
            <rootfile full-path="OEBPS/content.opf" media-type="application/oebps-package+xml"/>
          </rootfiles>
        </container>)";
}

}

void EpubPageIndexTest::followsSpineOrderAndMetadata()
{
    const QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package xmlns="http://www.idpf.org/2007/opf">
            <metadata>
              <meta property="rendition:layout">pre-paginated</meta>
            </metadata>
            <manifest>
              <item id="second" href="pages/b.xhtml" media-type="application/xhtml+xml"/>
              <item id="first" href="pages/a.xhtml" media-type="application/xhtml+xml"/>
              <item id="notes" href="pages/notes.xhtml" media-type="application/xhtml+xml"/>
              <item id="cover" href="images/cover.jpg" media-type="image/jpeg" properties="cover-image"/>
            </manifest>
            <spine>
              <itemref idref="first"/>
              <itemref idref="notes" linear="no"/>
              <itemref idref="second"/>
            </spine>
          </package>)" },
        { "OEBPS/pages/a.xhtml", R"(<html><body><img src="../images/z.jpg"/></body></html>)" },
        { "OEBPS/pages/b.xhtml", R"(<html><body><img src="../images/a.jpg"/></body></html>)" },
        { "OEBPS/pages/notes.xhtml", R"(<html><body><img src="../images/notes.jpg"/></body></html>)" },
        { "OEBPS/images/a.jpg", "a" },
        { "OEBPS/images/z.jpg", "z" },
        { "OEBPS/images/notes.jpg", "notes" },
        { "OEBPS/images/cover.jpg", "cover" },
    };
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY2(result.isValid(), qPrintable(result.error));
    QCOMPARE(result.pages.size(), 2);
    QCOMPARE(result.pages.at(0).fileName, QString("OEBPS/images/z.jpg"));
    QCOMPARE(result.pages.at(1).fileName, QString("OEBPS/images/a.jpg"));
    QVERIFY(result.fixedLayout);
    QCOMPARE(result.coverPath, QString("OEBPS/images/cover.jpg"));
}

void EpubPageIndexTest::resolvesRelativeAndEncodedPaths()
{
    const QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><manifest>
              <item id="page" href="pages/page.svg" media-type="image/svg+xml"/>
            </manifest><spine><itemref idref="page"/></spine></package>)" },
        { "OEBPS/pages/page.svg", R"(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
              <image xlink:href="../images/Page%2001.jpg#image"/>
            </svg>)" },
        { "OEBPS/images/Page 01.jpg", "page" },
    };
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY2(result.isValid(), qPrintable(result.error));
    QCOMPARE(result.pages.constFirst().fileName, QString("OEBPS/images/Page 01.jpg"));
}

void EpubPageIndexTest::readsObjectWrapper()
{
    const QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><manifest>
              <item id="page" href="page.xhtml" media-type="application/xhtml+xml"/>
            </manifest><spine><itemref idref="page"/></spine></package>)" },
        { "OEBPS/page.xhtml", R"(<html><body><object data="images/page.jpg"/></body></html>)" },
        { "OEBPS/images/page.jpg", "page" },
    };
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY2(result.isValid(), qPrintable(result.error));
    QCOMPARE(result.pages.constFirst().fileName, QString("OEBPS/images/page.jpg"));
}

void EpubPageIndexTest::toleratesHtmlNamedEntities()
{
    const QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><manifest>
              <item id="page" href="page.xhtml" media-type="application/xhtml+xml"/>
            </manifest><spine><itemref idref="page"/></spine></package>)" },
        { "OEBPS/page.xhtml", R"(<html><body><p>&nbsp;</p><img src="images/page.jpg"/></body></html>)" },
        { "OEBPS/images/page.jpg", "page" },
    };
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY2(result.isValid(), qPrintable(result.error));
    QCOMPARE(result.pages.constFirst().fileName, QString("OEBPS/images/page.jpg"));
}

void EpubPageIndexTest::preservesDuplicateSpineReferences()
{
    const QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><manifest>
              <item id="page" href="images/page.jpg" media-type="image/jpeg"/>
            </manifest><spine><itemref idref="page"/><itemref idref="page"/></spine></package>)" },
        { "OEBPS/images/page.jpg", "page" },
    };
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY2(result.isValid(), qPrintable(result.error));
    QCOMPARE(result.pages.size(), 2);
    QCOMPARE(result.pages.at(0).archiveIndex, result.pages.at(1).archiveIndex);
}

void EpubPageIndexTest::skipsBrokenSpineItems()
{
    const QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><manifest>
              <item id="broken" href="broken.xhtml" media-type="application/xhtml+xml"/>
              <item id="valid" href="valid.xhtml" media-type="application/xhtml+xml"/>
            </manifest><spine><itemref idref="broken"/><itemref idref="valid"/></spine></package>)" },
        { "OEBPS/broken.xhtml", R"(<html><body><img src="one.jpg"/><img src="two.jpg"/></body></html>)" },
        { "OEBPS/valid.xhtml", R"(<html><body><img src="valid.jpg"/></body></html>)" },
        { "OEBPS/one.jpg", "one" },
        { "OEBPS/two.jpg", "two" },
        { "OEBPS/valid.jpg", "valid" },
    };
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY2(result.isValid(), qPrintable(result.error));
    QCOMPARE(result.pages.size(), 1);
    QCOMPARE(result.pages.constFirst().fileName, QString("OEBPS/valid.jpg"));
}

void EpubPageIndexTest::rejectsWrappersWithMultipleImages()
{
    const QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><manifest>
              <item id="page" href="page.xhtml" media-type="application/xhtml+xml"/>
            </manifest><spine><itemref idref="page"/></spine></package>)" },
        { "OEBPS/page.xhtml", R"(<html><body><img src="one.jpg"/><img src="two.jpg"/></body></html>)" },
        { "OEBPS/one.jpg", "one" },
        { "OEBPS/two.jpg", "two" },
    };
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY(!result.isValid());
    QVERIFY(result.error.contains("no usable image pages"));
}

void EpubPageIndexTest::rejectsPathsOutsideTheArchive()
{
    const QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><manifest>
              <item id="page" href="../../page.jpg" media-type="image/jpeg"/>
            </manifest><spine><itemref idref="page"/></spine></package>)" },
        { "page.jpg", "page" },
    };
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY(!result.isValid());
    QVERIFY(result.error.contains("no usable image pages"));
}

void EpubPageIndexTest::rejectsIllustratedTextBooks()
{
    // A novel whose chapters happen to open with an illustration is not a comic.
    QString manifest;
    QString spine;
    QHash<QString, QByteArray> files { { "META-INF/container.xml", containerXml() } };
    for (int chapter = 0; chapter < 10; ++chapter) {
        manifest += QString(R"(<item id="ch%1" href="chapters/ch%1.xhtml" media-type="application/xhtml+xml"/>)").arg(chapter);
        spine += QString(R"(<itemref idref="ch%1"/>)").arg(chapter);
        files.insert(QString("OEBPS/chapters/ch%1.xhtml").arg(chapter),
                     QString(R"(<html><body><img src="../images/ch%1.jpg"/>
                       <p>It was the best of times, it was the worst of times, it was the age of wisdom.</p>
                       </body></html>)")
                             .arg(chapter)
                             .toUtf8());
        files.insert(QString("OEBPS/images/ch%1.jpg").arg(chapter), "illustration");
    }
    files.insert("OEBPS/content.opf", QString(R"(<package><manifest>%1</manifest><spine>%2</spine></package>)").arg(manifest, spine).toUtf8());
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY(!result.isValid());
    QVERIFY2(result.error.contains("not image based"), qPrintable(result.error));
    QVERIFY(result.pages.isEmpty());
}

void EpubPageIndexTest::rejectsPagesWithBodyText()
{
    const QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><manifest>
              <item id="one" href="one.xhtml" media-type="application/xhtml+xml"/>
              <item id="two" href="two.xhtml" media-type="application/xhtml+xml"/>
              <item id="three" href="three.xhtml" media-type="application/xhtml+xml"/>
            </manifest><spine>
              <itemref idref="one"/><itemref idref="two"/><itemref idref="three"/>
            </spine></package>)" },
        { "OEBPS/one.xhtml", R"(<html><body><img src="one.jpg"/><p>A paragraph of real text that the reader would silently drop.</p></body></html>)" },
        { "OEBPS/two.xhtml", R"(<html><body><img src="two.jpg"/><p>Another paragraph of real text that would go missing.</p></body></html>)" },
        { "OEBPS/three.xhtml", R"(<html><body><img src="three.jpg"/></body></html>)" },
        { "OEBPS/one.jpg", "one" },
        { "OEBPS/two.jpg", "two" },
        { "OEBPS/three.jpg", "three" },
    };
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY(!result.isValid());
    QVERIFY2(result.error.contains("characters of text"), qPrintable(result.error));
}

void EpubPageIndexTest::ignoresStyleScriptAndHiddenText()
{
    const QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><manifest>
              <item id="page" href="page.xhtml" media-type="application/xhtml+xml"/>
            </manifest><spine><itemref idref="page"/></spine></package>)" },
        { "OEBPS/page.xhtml", R"(<html>
            <head>
              <title>Page 1 of the comic</title>
              <style>body { margin: 0; padding: 0; } img { width: 100%; height: auto; }</style>
            </head>
            <body>
              <script>var viewport = { width: 1200, height: 1800 };</script>
              <div class="ocr" style="display: none">An invisible transcription layer of the whole page.</div>
              <img src="images/page.jpg"/>
              <span hidden="hidden">Another hidden accessibility layer with plenty of words in it.</span>
            </body></html>)" },
        { "OEBPS/images/page.jpg", "page" },
    };
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY2(result.isValid(), qPrintable(result.error));
    QCOMPARE(result.pages.constFirst().fileName, QString("OEBPS/images/page.jpg"));
}

void EpubPageIndexTest::ignoresNavigationDocuments()
{
    // The nav document is structural: it must neither become a page nor spend the
    // tolerance that a real credits page needs.
    const QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><manifest>
              <item id="nav" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav"/>
              <item id="credits" href="credits.xhtml" media-type="application/xhtml+xml"/>
              <item id="one" href="one.xhtml" media-type="application/xhtml+xml"/>
              <item id="two" href="two.xhtml" media-type="application/xhtml+xml"/>
            </manifest><spine>
              <itemref idref="nav"/><itemref idref="credits"/><itemref idref="one"/><itemref idref="two"/>
            </spine></package>)" },
        { "OEBPS/nav.xhtml", R"(<html><body><nav><ol><li><a href="one.xhtml">Chapter one</a></li><li><a href="two.xhtml">Chapter two</a></li></ol></nav></body></html>)" },
        { "OEBPS/credits.xhtml", R"(<html><body><p>Script, art and lettering by someone. All rights reserved.</p></body></html>)" },
        { "OEBPS/one.xhtml", R"(<html><body><img src="one.jpg"/></body></html>)" },
        { "OEBPS/two.xhtml", R"(<html><body><img src="two.jpg"/></body></html>)" },
        { "OEBPS/one.jpg", "one" },
        { "OEBPS/two.jpg", "two" },
    };
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY2(result.isValid(), qPrintable(result.error));
    QCOMPARE(result.pages.size(), 2);
    QCOMPARE(result.pages.at(0).fileName, QString("OEBPS/one.jpg"));
    QCOMPARE(result.pages.at(1).fileName, QString("OEBPS/two.jpg"));
}

void EpubPageIndexTest::toleratesAFewNonImagePages()
{
    QString manifest = R"(<item id="credits" href="credits.xhtml" media-type="application/xhtml+xml"/>)";
    QString spine = R"(<itemref idref="credits"/>)";
    QHash<QString, QByteArray> files {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/credits.xhtml", R"(<html><body><p>Script, art and lettering by someone. All rights reserved.</p></body></html>)" },
    };
    for (int page = 0; page < 20; ++page) {
        manifest += QString(R"(<item id="p%1" href="images/p%1.jpg" media-type="image/jpeg"/>)").arg(page);
        spine += QString(R"(<itemref idref="p%1"/>)").arg(page);
        files.insert(QString("OEBPS/images/p%1.jpg").arg(page), "page");
    }
    files.insert("OEBPS/content.opf", QString(R"(<package><manifest>%1</manifest><spine>%2</spine></package>)").arg(manifest, spine).toUtf8());
    const QStringList fileNames = files.keys();

    const auto result = readIndex(fileNames, files);

    QVERIFY2(result.isValid(), qPrintable(result.error));
    QCOMPARE(result.pages.size(), 20);
}

void EpubPageIndexTest::scanInfoMatchesThePageIndex()
{
    const QHash<QString, QByteArray> comic {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><metadata>
              <meta property="rendition:layout">pre-paginated</meta>
            </metadata><manifest>
              <item id="one" href="one.xhtml" media-type="application/xhtml+xml"/>
              <item id="two" href="two.xhtml" media-type="application/xhtml+xml"/>
            </manifest><spine><itemref idref="one"/><itemref idref="two"/></spine></package>)" },
        { "OEBPS/one.xhtml", R"(<html><body><img src="one.jpg"/></body></html>)" },
        { "OEBPS/two.xhtml", R"(<html><body><img src="two.jpg"/></body></html>)" },
        { "OEBPS/one.jpg", "one" },
        { "OEBPS/two.jpg", "two" },
    };
    const QStringList comicNames = comic.keys();

    const auto comicScan = readScan(comicNames, comic);

    QVERIFY2(comicScan.isValid(), qPrintable(comicScan.error));
    QCOMPARE(comicScan.pageCount, static_cast<int>(readIndex(comicNames, comic).pages.size()));
    QCOMPARE(comicScan.coverArchiveIndex, static_cast<int>(comicNames.indexOf(QString("OEBPS/one.jpg"))));

    // A fixed layout book made of text pages used to be counted page by page.
    const QHash<QString, QByteArray> textBook {
        { "META-INF/container.xml", containerXml() },
        { "OEBPS/content.opf", R"(<package><metadata>
              <meta property="rendition:layout">pre-paginated</meta>
            </metadata><manifest>
              <item id="one" href="one.xhtml" media-type="application/xhtml+xml"/>
              <item id="two" href="two.xhtml" media-type="application/xhtml+xml"/>
            </manifest><spine><itemref idref="one"/><itemref idref="two"/></spine></package>)" },
        { "OEBPS/one.xhtml", R"(<html><body><h1>A chapter</h1><p>Page after page of prose with no pictures at all.</p></body></html>)" },
        { "OEBPS/two.xhtml", R"(<html><body><p>More prose, still with nothing for the reader to show.</p></body></html>)" },
    };
    const QStringList textBookNames = textBook.keys();

    const auto textBookScan = readScan(textBookNames, textBook);

    QVERIFY(!textBookScan.isValid());
    QCOMPARE(textBookScan.pageCount, 0);
    QCOMPARE(textBookScan.coverArchiveIndex, -1);
}

QTEST_GUILESS_MAIN(EpubPageIndexTest)

#include "main.moc"
