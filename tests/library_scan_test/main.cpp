#include "data_base_management.h"
#include "db_helper.h"
#include "library_creator.h"
#include "yacreader_global.h"

#include <QBuffer>
#include <QColor>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTemporaryDir>
#include <QtTest>

#include <functional>
#include <memory>

using namespace YACReader;

namespace {

quint32 crc32(const QByteArray &data)
{
    quint32 crc = 0xFFFFFFFF;
    for (const auto byte : data) {
        crc ^= static_cast<quint8>(byte);
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : crc >> 1;
        }
    }
    return ~crc;
}

// A ZIP archive with stored (uncompressed) entries.
QByteArray storedZip(const QList<QPair<QString, QByteArray>> &entries)
{
    QByteArray archive;
    QByteArray centralDirectory;
    QDataStream out(&archive, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    QDataStream central(&centralDirectory, QIODevice::WriteOnly);
    central.setByteOrder(QDataStream::LittleEndian);

    for (const auto &[name, data] : entries) {
        const auto fileName = name.toUtf8();
        const auto crc = crc32(data);
        const auto offset = static_cast<quint32>(archive.size());

        out << quint32(0x04034b50) << quint16(20) << quint16(0) << quint16(0) << quint16(0) << quint16(0x21)
            << crc << quint32(data.size()) << quint32(data.size()) << quint16(fileName.size()) << quint16(0);
        out.writeRawData(fileName.constData(), fileName.size());
        out.writeRawData(data.constData(), data.size());

        central << quint32(0x02014b50) << quint16(20) << quint16(20) << quint16(0) << quint16(0) << quint16(0) << quint16(0x21)
                << crc << quint32(data.size()) << quint32(data.size()) << quint16(fileName.size()) << quint16(0) << quint16(0)
                << quint16(0) << quint16(0) << quint32(0) << offset;
        central.writeRawData(fileName.constData(), fileName.size());
    }

    const auto centralOffset = static_cast<quint32>(archive.size());
    out.writeRawData(centralDirectory.constData(), centralDirectory.size());
    out << quint32(0x06054b50) << quint16(0) << quint16(0) << quint16(entries.size()) << quint16(entries.size())
        << quint32(centralDirectory.size()) << centralOffset << quint16(0);

    return archive;
}

QByteArray png(const QColor &color)
{
    QImage image(32, 48, QImage::Format_RGB32);
    image.fill(color);
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return data;
}

}

class LibraryScanTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void createAddsFoldersAndComics();
    void updateMergesDiskAndLibrary();
    void folderContentLoadersReturnSortedValues();
    void replacementDecision();

private:
    std::unique_ptr<QTemporaryDir> root;
    QString source;

    bool writeComic(const QString &relativePath, const QColor &color, int padding = 0);
    bool removePath(const QString &relativePath);
    bool scan(bool create);
    bool withDatabase(const std::function<bool(QSqlDatabase &)> &function);
    QStringList rows(const QString &sql);
    QString value(const QString &sql);
};

void LibraryScanTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    source = QDir::cleanPath(root->filePath("library"));
    QVERIFY(QDir().mkpath(source));

    QVERIFY(writeComic("a.cbz", Qt::red));
    QVERIFY(writeComic("b/b1.cbz", Qt::green));
    QVERIFY(writeComic("b/b2.cbz", Qt::blue));
    QVERIFY(writeComic("b.cbz", Qt::yellow));
    QVERIFY(writeComic("c/d/d1.cbz", Qt::cyan));
    QVERIFY(writeComic("z.cbz", Qt::magenta));
    QVERIFY(scan(true));
}

void LibraryScanTest::cleanup()
{
    root.reset();
}

bool LibraryScanTest::writeComic(const QString &relativePath, const QColor &color, int padding)
{
    QList<QPair<QString, QByteArray>> entries = { { "page.png", png(color) } };
    if (padding > 0) {
        entries.append({ "padding.txt", QByteArray(padding, 'x') });
    }

    const auto path = source + "/" + relativePath;
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) && file.write(storedZip(entries)) > 0;
}

bool LibraryScanTest::removePath(const QString &relativePath)
{
    const QFileInfo info(source + "/" + relativePath);
    return info.isDir() ? QDir(info.absoluteFilePath()).removeRecursively() : QFile::remove(info.absoluteFilePath());
}

bool LibraryScanTest::scan(bool create)
{
    QSettings settings(root->filePath("settings.ini"), QSettings::IniFormat);
    settings.setValue(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, false);
    settings.sync();

    LibraryCreator creator(&settings);
    bool failed = false;
    connect(&creator, &LibraryCreator::failedCreatingDB, this, [&failed](const QString &error) { qWarning() << error; failed = true; }, Qt::DirectConnection);
    connect(&creator, &LibraryCreator::failedOpeningDB, this, [&failed](const QString &error) { qWarning() << error; failed = true; }, Qt::DirectConnection);

    if (create) {
        creator.createLibrary(source, LibraryPaths::libraryDataPath(source));
    } else {
        creator.updateLibrary(source, LibraryPaths::libraryDataPath(source));
    }
    creator.start();
    return creator.wait(120000) && !failed;
}

bool LibraryScanTest::withDatabase(const std::function<bool(QSqlDatabase &)> &function)
{
    const QString connectionName = "library_scan_test";
    bool success = false;
    {
        auto db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(LibraryPaths::libraryDatabasePath(source));
        success = db.open() && function(db);
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return success;
}

// Each row as its values joined with '|'.
QStringList LibraryScanTest::rows(const QString &sql)
{
    QStringList result;
    withDatabase([&sql, &result](QSqlDatabase &db) {
        QSqlQuery query(db);
        if (!query.exec(sql)) {
            qWarning() << query.lastError().text() << sql;
            return false;
        }
        while (query.next()) {
            QStringList values;
            for (int i = 0; i < query.record().count(); i++) {
                values.append(query.value(i).isNull() ? "NULL" : query.value(i).toString());
            }
            result.append(values.join('|'));
        }
        return true;
    });
    return result;
}

QString LibraryScanTest::value(const QString &sql)
{
    const auto result = rows(sql);
    return result.isEmpty() ? QString() : result.first();
}

static const QString comicsWithFolders = "SELECT c.path, f.path FROM comic c INNER JOIN folder f ON c.parentId = f.id ORDER BY c.path";
static const QString folders = "SELECT path, numChildren FROM folder WHERE id <> 1 ORDER BY path";

void LibraryScanTest::createAddsFoldersAndComics()
{
    QCOMPARE(rows(comicsWithFolders), QStringList({
                                              "/a.cbz|/",
                                              "/b.cbz|/",
                                              "/b/b1.cbz|/b",
                                              "/b/b2.cbz|/b",
                                              "/c/d/d1.cbz|/c/d",
                                              "/z.cbz|/",
                                      }));
    QCOMPARE(rows(folders), QStringList({ "/b|2", "/c|1", "/c/d|1" }));
}

void LibraryScanTest::updateMergesDiskAndLibrary()
{
    const auto keptComics = rows("SELECT id, comicInfoId, path FROM comic WHERE path IN ('/b.cbz', '/b/b1.cbz', '/b/b2.cbz') ORDER BY path");
    const auto keptFolder = value("SELECT id FROM folder WHERE path = '/b'");
    const auto replacedInfo = value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'");
    QCOMPARE(keptComics.size(), 3);

    QVERIFY(removePath("z.cbz"));
    QVERIFY(removePath("c"));
    QVERIFY(writeComic("b/b0.cbz", Qt::white));
    QVERIFY(writeComic("e/e1.cbz", Qt::black));
    QVERIFY(writeComic("a.cbz", Qt::red, 4096));
    QVERIFY(scan(false));

    QCOMPARE(rows(comicsWithFolders), QStringList({
                                              "/a.cbz|/",
                                              "/b.cbz|/",
                                              "/b/b0.cbz|/b",
                                              "/b/b1.cbz|/b",
                                              "/b/b2.cbz|/b",
                                              "/e/e1.cbz|/e",
                                      }));
    QCOMPARE(rows(folders), QStringList({ "/b|3", "/e|1" }));

    QCOMPARE(rows("SELECT id, comicInfoId, path FROM comic WHERE path IN ('/b.cbz', '/b/b1.cbz', '/b/b2.cbz') ORDER BY path"), keptComics);
    QCOMPARE(value("SELECT id FROM folder WHERE path = '/b'"), keptFolder);

    const auto newInfo = value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'");
    QVERIFY(!newInfo.isEmpty());
    QVERIFY(newInfo != replacedInfo);
    QCOMPARE(value("SELECT numPages FROM comic_info WHERE id = " + newInfo), QString("1"));
}

void LibraryScanTest::folderContentLoadersReturnSortedValues()
{
    const auto folderB = value("SELECT id FROM folder WHERE path = '/b'").toULongLong();

    QList<Folder> subfolders;
    QList<ComicDB> comics;
    QVERIFY(withDatabase([&](QSqlDatabase &db) {
        subfolders = DBHelper::getFoldersFromParent(1, db, true);
        comics = DBHelper::getComicsFromParent(folderB, db, true);
        return true;
    }));

    QStringList folderNames;
    for (const auto &folder : subfolders)
        folderNames.append(folder.name);
    QCOMPARE(folderNames, QStringList({ "b", "c" }));

    QStringList comicNames;
    for (const auto &comic : comics) {
        comicNames.append(comic.file.fileName);
        QCOMPARE(comic.file.folderId, FolderId(folderB));
        QVERIFY(comic.info.id != ComicInfoId());
        QVERIFY(!comic.info.hash.isEmpty());
    }
    QCOMPARE(comicNames, QStringList({ "b1.cbz", "b2.cbz" }));
}

void LibraryScanTest::replacementDecision()
{
    QCOMPARE(classifyReplaceComic(ComicInfoId(1), std::nullopt, false), ReplaceComicResult::NotInserted);
    QCOMPARE(classifyReplaceComic(ComicInfoId(1), ComicInfoId(1), true), ReplaceComicResult::SameComicInfo);
    QCOMPARE(classifyReplaceComic(ComicInfoId(1), ComicInfoId(2), false), ReplaceComicResult::NewComicInfo);
    QCOMPARE(classifyReplaceComic(ComicInfoId(1), ComicInfoId(2), true), ReplaceComicResult::KnownComicInfo);
}

QTEST_GUILESS_MAIN(LibraryScanTest)

#include "main.moc"
