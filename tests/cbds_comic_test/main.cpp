#include "comic.h"
#include "compressed_archive.h"
#include "initial_comic_info_extractor.h"
#include "library_creator.h"
#include "yacreader_global.h"

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QSettings>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>
#include <QtEndian>

#include <array>

class CbdsComicTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void readerShowsOnlyFullSizePagesInOrder_data();
    void readerShowsOnlyFullSizePagesInOrder();
    void readerAndLibraryAgreeOnPageNumbers();
    void pageOrderIgnoresPathSpelling();
    void coverAndPageCountForTheLibrary();
    void worksWithoutTheIni();
    void ignoresBrokenAndHugeIni();
    void detectsRenamedBooksByContent();
    void leavesRegularComicsUntouched();
    void reportsCorruptAndEmptyArchives();
    void survivesTruncatedArchives();
    void libraryImportsAndRescansBooks();

private:
    QTemporaryDir dir;
};

namespace {

constexpr int pageCount = 12;
// Pages are told apart by their red channel, previews are pure green.
constexpr int previewGreen = 255;

int pageRed(int page)
{
    return page * 20;
}

quint32 crc32(const QByteArray &data)
{
    static const std::array<quint32, 256> table = [] {
        std::array<quint32, 256> t { };
        for (quint32 i = 0; i < 256; ++i) {
            quint32 c = i;
            for (int k = 0; k < 8; ++k) {
                c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            }
            t[i] = c;
        }
        return t;
    }();

    quint32 crc = 0xFFFFFFFFu;
    for (const char byte : data) {
        crc = table[(crc ^ static_cast<quint8>(byte)) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFu;
}

template<typename T>
void append(QByteArray &out, T value)
{
    const T le = qToLittleEndian(value);
    out.append(reinterpret_cast<const char *>(&le), sizeof(T));
}

enum class ZipLayout {
    Stored,
    // Like the books written by PictoDS: Deflate with the sizes and CRC in a
    // data descriptor after each entry (general purpose flag bit 3).
    DeflateWithDataDescriptor,
};

// Raw Deflate stream: qCompress() adds a 4 byte length, a 2 byte zlib header
// and a 4 byte Adler-32 trailer around it.
QByteArray rawDeflate(const QByteArray &data)
{
    const QByteArray zlib = qCompress(data, 9);
    return zlib.mid(6, zlib.size() - 10);
}

// Minimal zip writer, enough to build test archives without depending on an
// archiving library.
QByteArray zipArchive(const QList<QPair<QString, QByteArray>> &entries, ZipLayout layout = ZipLayout::Stored)
{
    const bool deflate = layout == ZipLayout::DeflateWithDataDescriptor;
    const quint16 flags = deflate ? 0x0008 : 0;
    const quint16 method = deflate ? 8 : 0;

    QByteArray archive;
    QByteArray centralDirectory;

    for (const auto &[name, data] : entries) {
        const QByteArray encodedName = name.toUtf8();
        const quint32 crc = crc32(data);
        const QByteArray stored = deflate ? rawDeflate(data) : data;
        const auto offset = static_cast<quint32>(archive.size());

        append<quint32>(archive, 0x04034b50);
        append<quint16>(archive, 20); // version needed
        append<quint16>(archive, flags);
        append<quint16>(archive, method);
        append<quint16>(archive, 0); // time
        append<quint16>(archive, 0x21); // date: 1980-01-01
        append<quint32>(archive, deflate ? 0 : crc);
        append<quint32>(archive, deflate ? 0 : stored.size());
        append<quint32>(archive, deflate ? 0 : data.size());
        append<quint16>(archive, encodedName.size());
        append<quint16>(archive, 0);
        archive.append(encodedName);
        archive.append(stored);
        if (deflate) {
            append<quint32>(archive, 0x08074b50);
            append<quint32>(archive, crc);
            append<quint32>(archive, stored.size());
            append<quint32>(archive, data.size());
        }

        append<quint32>(centralDirectory, 0x02014b50);
        append<quint16>(centralDirectory, 20); // version made by
        append<quint16>(centralDirectory, 20); // version needed
        append<quint16>(centralDirectory, flags);
        append<quint16>(centralDirectory, method);
        append<quint16>(centralDirectory, 0);
        append<quint16>(centralDirectory, 0x21);
        append<quint32>(centralDirectory, crc);
        append<quint32>(centralDirectory, stored.size());
        append<quint32>(centralDirectory, data.size());
        append<quint16>(centralDirectory, encodedName.size());
        append<quint16>(centralDirectory, 0); // extra
        append<quint16>(centralDirectory, 0); // comment
        append<quint16>(centralDirectory, 0); // disk
        append<quint16>(centralDirectory, 0); // internal attributes
        append<quint32>(centralDirectory, 0); // external attributes
        append<quint32>(centralDirectory, offset);
        centralDirectory.append(encodedName);
    }

    const auto centralDirectoryOffset = static_cast<quint32>(archive.size());
    archive.append(centralDirectory);
    append<quint32>(archive, 0x06054b50);
    append<quint16>(archive, 0);
    append<quint16>(archive, 0);
    append<quint16>(archive, entries.size());
    append<quint16>(archive, entries.size());
    append<quint32>(archive, centralDirectory.size());
    append<quint32>(archive, centralDirectoryOffset);
    append<quint16>(archive, 0);
    return archive;
}

QByteArray image(int red, int green)
{
    QImage img(16, 24, QImage::Format_RGB32);
    img.fill(qRgb(red, green, 0));
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    return data;
}

QByteArray bookIni(int pages)
{
    return QStringLiteral("; ComicBookDS ini file\r\n"
                          "CbCredits1 = Synthetic test book\r\n"
                          "CbCredits2 = Unknown\r\n"
                          "CbCredits3 = Unknown\r\n"
                          "LeftToRight = 1\r\n"
                          "NbPages = %1\r\n"
                          "Version = 200\r\n")
            .arg(pages)
            .toLatin1();
}

// Same layout as the books written by PictoDS: the ini, then each folder with
// its files numbered from 1, listed in the lexicographic order of real books.
QList<QPair<QString, QByteArray>> bookEntries(const QByteArray &ini = bookIni(pageCount))
{
    QList<QPair<QString, QByteArray>> entries;
    if (!ini.isNull()) {
        entries.append({ "ComicBookDS_book.ini", ini });
    }

    QStringList numbers;
    for (int page = 1; page <= pageCount; ++page) {
        numbers.append(QString::number(page));
    }
    numbers.sort();

    for (const QString &number : std::as_const(numbers)) {
        entries.append({ QString("IMAGE/%1.png").arg(number), image(pageRed(number.toInt()), 0) });
    }
    for (const QString &number : std::as_const(numbers)) {
        entries.append({ QString("NAME/%1.txt").arg(number), QString("original_%1.jpg\r\n").arg(number).toLatin1() });
    }
    for (const QString folder : { "SMALL_N", "SMALL_R", "THMB_N", "THMB_R" }) {
        for (const QString &number : std::as_const(numbers)) {
            entries.append({ QString("%1/%2.png").arg(folder, number), image(0, previewGreen) });
        }
    }
    return entries;
}

QString writeFile(const QString &path, const QByteArray &data)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size()) {
        return { };
    }
    return path;
}

struct OpenResult {
    bool failed = false;
    QList<QByteArray> pages;
};

OpenResult openInReader(const QString &path)
{
    FileComic comic;
    QSignalSpy errorSpy(&comic, static_cast<void (Comic::*)()>(&Comic::errorOpening));
    QSignalSpy errorMessageSpy(&comic, static_cast<void (Comic::*)(QString)>(&Comic::errorOpening));
    QSignalSpy pagesSpy(&comic, static_cast<void (Comic::*)(unsigned int)>(&Comic::numPages));

    OpenResult result;
    if (!comic.load(path, 0)) {
        result.failed = true;
        return result;
    }
    comic.process();

    result.failed = !errorSpy.isEmpty() || !errorMessageSpy.isEmpty() || pagesSpy.isEmpty();
    if (!result.failed) {
        for (int page = 0; page < static_cast<int>(pagesSpy.first().first().toUInt()); ++page) {
            result.pages.append(comic.getRawPage(page));
        }
    }
    return result;
}

QColor colorOf(const QByteArray &data)
{
    QImage img;
    if (!img.loadFromData(data)) {
        return { };
    }
    return img.pixelColor(img.width() / 2, img.height() / 2);
}

}

void CbdsComicTest::initTestCase()
{
    QVERIFY(dir.isValid());

    const QString probe = writeFile(dir.filePath("probe.cbz"), zipArchive({ { "1.png", image(1, 0) } }));
    CompressedArchive archive(probe);
    if (!archive.toolsLoaded()) {
        QSKIP("The decompression backend library is not deployed next to the test executable");
    }
    QVERIFY(archive.isValid());
}

void CbdsComicTest::readerShowsOnlyFullSizePagesInOrder_data()
{
    QTest::addColumn<bool>("deflate");
    QTest::newRow("stored") << false;
    QTest::newRow("deflate with data descriptor") << true;
}

void CbdsComicTest::readerShowsOnlyFullSizePagesInOrder()
{
    QFETCH(bool, deflate);
    const ZipLayout layout = deflate ? ZipLayout::DeflateWithDataDescriptor : ZipLayout::Stored;
    const QString path = writeFile(dir.filePath(deflate ? "book-deflate.cbds" : "book-stored.cbds"), zipArchive(bookEntries(), layout));

    const OpenResult result = openInReader(path);

    QVERIFY(!result.failed);
    QCOMPARE(result.pages.size(), pageCount);
    for (int page = 0; page < pageCount; ++page) {
        const QColor color = colorOf(result.pages.at(page));
        QVERIFY2(color.isValid(), qPrintable(QString("page %1 is not an image").arg(page + 1)));
        QCOMPARE(color.green(), 0);
        QCOMPARE(color.red(), pageRed(page + 1));
    }
}

void CbdsComicTest::readerAndLibraryAgreeOnPageNumbers()
{
    // Page numbers are sequence numbers. A name like 1011 must not be taken for a
    // spread of pages 10 and 11, the reader has to use the order the library uses.
    QList<QPair<QString, QByteArray>> entries { { "ComicBookDS_book.ini", bookIni(pageCount + 1) } };
    for (int page = 1; page <= pageCount; ++page) {
        entries.append({ QString("IMAGE/%1.png").arg(page), image(pageRed(page), 0) });
    }
    entries.append({ "IMAGE/1011.png", image(255, 0) });
    const QString path = writeFile(dir.filePath("numbers.cbds"), zipArchive(entries));

    const OpenResult result = openInReader(path);

    QVERIFY(!result.failed);
    QCOMPARE(result.pages.size(), pageCount + 1);
    for (int page = 0; page < pageCount; ++page) {
        QCOMPARE(colorOf(result.pages.at(page)).red(), pageRed(page + 1));
    }
    QCOMPARE(colorOf(result.pages.last()).red(), 255);

    YACReader::InitialComicInfoExtractor secondPage(path, "", 2);
    secondPage.extract();
    QCOMPARE(secondPage.getCoverImage().pixelColor(0, 0).red(), colorOf(result.pages.at(1)).red());
}

void CbdsComicTest::pageOrderIgnoresPathSpelling()
{
    const QString path = writeFile(dir.filePath("spelling.cbds"),
                                   zipArchive({ { "/IMAGE/3.png", image(pageRed(3), 0) },
                                                { "image\\1.png", image(pageRed(1), 0) },
                                                { "IMAGE/2.png", image(pageRed(2), 0) } }));

    const OpenResult result = openInReader(path);

    QVERIFY(!result.failed);
    QCOMPARE(result.pages.size(), 3);
    for (int page = 0; page < 3; ++page) {
        QCOMPARE(colorOf(result.pages.at(page)).red(), pageRed(page + 1));
    }

    YACReader::InitialComicInfoExtractor extractor(path);
    extractor.extract();
    QCOMPARE(extractor.getNumPages(), 3);
    QCOMPARE(extractor.getCoverImage().pixelColor(0, 0).red(), pageRed(1));
}

void CbdsComicTest::coverAndPageCountForTheLibrary()
{
    const QString path = writeFile(dir.filePath("cover.cbds"), zipArchive(bookEntries(), ZipLayout::DeflateWithDataDescriptor));

    YACReader::InitialComicInfoExtractor extractor(path);
    extractor.extract();

    QVERIFY(extractor.isFileSupported());
    QCOMPARE(extractor.getNumPages(), pageCount);
    QVERIFY(extractor.hasValidCover());
    QCOMPARE(extractor.getCoverImage().pixelColor(0, 0).red(), pageRed(1));

    YACReader::InitialComicInfoExtractor thirdPage(path, "", 3);
    thirdPage.extract();
    QCOMPARE(thirdPage.getCoverImage().pixelColor(0, 0).red(), pageRed(3));
}

void CbdsComicTest::worksWithoutTheIni()
{
    const QString path = writeFile(dir.filePath("noini.cbds"), zipArchive(bookEntries(QByteArray())));

    const OpenResult result = openInReader(path);

    QVERIFY(!result.failed);
    QCOMPARE(result.pages.size(), pageCount);
    QCOMPARE(colorOf(result.pages.first()).red(), pageRed(1));
}

void CbdsComicTest::ignoresBrokenAndHugeIni()
{
    QByteArray garbage(8 * 1024 * 1024, '\0');
    for (int i = 0; i < garbage.size(); i += 7) {
        garbage[i] = static_cast<char>(i % 251);
    }
    const QString huge = writeFile(dir.filePath("hugeini.cbds"), zipArchive(bookEntries(garbage)));
    const QString wrong = writeFile(dir.filePath("wrongini.cbds"), zipArchive(bookEntries("NbPages = -99999999999999999999\r\n[[[\r\n=\r\nLeftToRight")));

    for (const QString &path : { huge, wrong }) {
        const OpenResult result = openInReader(path);
        QVERIFY(!result.failed);
        QCOMPARE(result.pages.size(), pageCount);
    }
}

void CbdsComicTest::detectsRenamedBooksByContent()
{
    const QString path = writeFile(dir.filePath("renamed.cbz"), zipArchive(bookEntries()));

    const OpenResult result = openInReader(path);

    QVERIFY(!result.failed);
    QCOMPARE(result.pages.size(), pageCount);
}

void CbdsComicTest::leavesRegularComicsUntouched()
{
    // Without the ini a .cbz is a regular comic, every image in it is a page.
    QList<QPair<QString, QByteArray>> entries = bookEntries(QByteArray());
    const QString path = writeFile(dir.filePath("regular.cbz"), zipArchive(entries));

    const OpenResult result = openInReader(path);

    QVERIFY(!result.failed);
    QCOMPARE(result.pages.size(), pageCount * 5);

    const QString plain = writeFile(dir.filePath("plain.cbz"), zipArchive({ { "b/02.png", image(2, 0) }, { "a/01.png", image(1, 0) }, { "ComicInfo.xml", "<ComicInfo/>" } }));
    const OpenResult plainResult = openInReader(plain);
    QVERIFY(!plainResult.failed);
    QCOMPARE(plainResult.pages.size(), 2);
    QCOMPARE(colorOf(plainResult.pages.first()).red(), 1);
}

void CbdsComicTest::reportsCorruptAndEmptyArchives()
{
    QByteArray noise(64 * 1024, '\0');
    for (int i = 0; i < noise.size(); ++i) {
        noise[i] = static_cast<char>((i * 7919) % 256);
    }
    const QString corrupt = writeFile(dir.filePath("corrupt.cbds"), noise);
    const QString empty = writeFile(dir.filePath("empty.cbds"), zipArchive({ }));
    const QString iniOnly = writeFile(dir.filePath("inionly.cbds"), zipArchive({ { "ComicBookDS_book.ini", bookIni(0) } }));
    const QString zeroBytes = writeFile(dir.filePath("zero.cbds"), QByteArray());

    for (const QString &path : { corrupt, empty, iniOnly, zeroBytes }) {
        QVERIFY2(openInReader(path).failed, qPrintable(path));

        YACReader::InitialComicInfoExtractor extractor(path);
        extractor.extract();
        QCOMPARE(extractor.getNumPages(), 0);
        QVERIFY(!extractor.hasValidCover());
    }
}

void CbdsComicTest::survivesTruncatedArchives()
{
    // Real damaged books end in zeros where the central directory should be.
    QByteArray data = zipArchive(bookEntries());
    const int keep = data.size() / 3;
    data = data.left(keep) + QByteArray(data.size() - keep, '\0');
    const QString path = writeFile(dir.filePath("truncated.cbds"), data);

    const OpenResult result = openInReader(path);

    // Backends differ in how much of a broken archive they recover; what matters
    // is a controlled result and that only real pages are ever shown.
    QVERIFY(result.pages.size() <= pageCount);
    for (const QByteArray &page : result.pages) {
        if (!page.isEmpty()) {
            QCOMPARE(colorOf(page).green(), 0);
        }
    }

    YACReader::InitialComicInfoExtractor extractor(path);
    extractor.extract();
    QVERIFY(extractor.getNumPages() <= pageCount);
}

void CbdsComicTest::libraryImportsAndRescansBooks()
{
    QTemporaryDir libraryDir;
    QVERIFY(libraryDir.isValid());
    QVERIFY(!writeFile(libraryDir.filePath("Synthetic.cbds"), zipArchive(bookEntries())).isEmpty());

    const QString dataPath = YACReader::LibraryPaths::libraryDataPath(libraryDir.path());
    QSettings settings(dir.filePath("settings.ini"), QSettings::IniFormat);

    const auto scan = [&](bool create) {
        LibraryCreator creator(&settings);
        if (create) {
            creator.createLibrary(libraryDir.path(), dataPath);
        } else {
            creator.updateLibrary(libraryDir.path(), dataPath);
        }
        creator.start();
        return creator.wait(60000);
    };

    struct Row {
        qulonglong id;
        QString fileName;
        int numPages;
        QString hash;
    };
    const auto rows = [&] {
        QList<Row> result;
        const QString connection = QStringLiteral("cbds_comic_test");
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connection);
            db.setDatabaseName(YACReader::LibraryPaths::libraryDatabasePath(libraryDir.path()));
            if (db.open()) {
                QSqlQuery query("SELECT c.id, c.fileName, ci.numPages, ci.hash FROM comic c INNER JOIN comic_info ci ON c.comicInfoId = ci.id", db);
                while (query.next()) {
                    result.append({ query.value(0).toULongLong(), query.value(1).toString(), query.value(2).toInt(), query.value(3).toString() });
                }
            }
        }
        QSqlDatabase::removeDatabase(connection);
        return result;
    };

    QVERIFY(scan(true));
    const QList<Row> created = rows();
    QCOMPARE(created.size(), 1);
    QCOMPARE(created.first().fileName, QString("Synthetic.cbds"));
    QCOMPARE(created.first().numPages, pageCount);

    const QImage cover(YACReader::LibraryPaths::coverPathFromLibraryDataPath(dataPath, created.first().hash));
    QVERIFY(!cover.isNull());
    QVERIFY(qAbs(cover.pixelColor(cover.width() / 2, cover.height() / 2).red() - pageRed(1)) <= 4);

    QVERIFY(scan(false));
    const QList<Row> updated = rows();
    QCOMPARE(updated.size(), 1);
    QCOMPARE(updated.first().id, created.first().id);
    QCOMPARE(updated.first().numPages, pageCount);
}

QTEST_GUILESS_MAIN(CbdsComicTest)

#include "main.moc"
