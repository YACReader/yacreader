#include "yacreader_libraries.h"

#include <QDir>
#include <QTemporaryDir>
#include <QTest>

class YACReaderLibrariesTest : public QObject
{
    Q_OBJECT

private slots:
    void renamePreservesLibraryIdentityAndPath();
    void removingUnknownLibraryDoesNothing();
};

void YACReaderLibrariesTest::renamePreservesLibraryIdentityAndPath()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const auto comicsPath = temporaryDirectory.filePath("comics");
    const auto mangaPath = temporaryDirectory.filePath("manga");
    QVERIFY(QDir().mkpath(QDir(comicsPath).filePath(".yacreaderlibrary")));
    QVERIFY(QDir().mkpath(QDir(mangaPath).filePath(".yacreaderlibrary")));

    YACReaderLibraries libraries;
    libraries.addLibrary("Comics", comicsPath);
    libraries.addLibrary("Manga", mangaPath);

    const auto comicsId = libraries.getUuid("Comics");
    const auto comicsLegacyId = libraries.getId("Comics");
    const auto mangaId = libraries.getUuid("Manga");
    const auto mangaLegacyId = libraries.getId("Manga");

    libraries.rename("Comics", "Renamed Comics");

    QCOMPARE(libraries.getPath("Renamed Comics"), comicsPath);
    QCOMPARE(libraries.getUuid("Renamed Comics"), comicsId);
    QCOMPARE(libraries.getId("Renamed Comics"), comicsLegacyId);
    QCOMPARE(libraries.getPath("Manga"), mangaPath);
    QCOMPARE(libraries.getUuid("Manga"), mangaId);
    QCOMPARE(libraries.getId("Manga"), mangaLegacyId);
}

void YACReaderLibrariesTest::removingUnknownLibraryDoesNothing()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const auto libraryPath = temporaryDirectory.filePath("library");
    QVERIFY(QDir().mkpath(QDir(libraryPath).filePath(".yacreaderlibrary")));

    YACReaderLibraries libraries;
    libraries.addLibrary("Library", libraryPath);

    libraries.remove("Missing");

    QCOMPARE(libraries.getNames(), QList<QString> { "Library" });
    QCOMPARE(libraries.getPath("Library"), libraryPath);
}

QTEST_GUILESS_MAIN(YACReaderLibrariesTest)

#include "main.moc"
