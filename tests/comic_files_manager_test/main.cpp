#include "comic_files_manager.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

class ComicFilesManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void copiesComicAndReportsDestinationFolder();
    void movesComicAndRemovesSource();
};

namespace {
QString createSourceComic(QTemporaryDir &temporaryDir, const QString &name)
{
    const QString path = temporaryDir.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write("comic") == -1)
        return QString();
    return path;
}
}

void ComicFilesManagerTest::copiesComicAndReportsDestinationFolder()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());
    const QString source = createSourceComic(temporaryDir, QStringLiteral("source.cbz"));
    QVERIFY(!source.isEmpty());

    ComicFilesManager manager;
    QSignalSpy successSpy(&manager, &ComicFilesManager::success);
    QSignalSpy finishedSpy(&manager, &ComicFilesManager::finished);
    manager.copyComicsTo({ { source, QStringLiteral("Series") } }, temporaryDir.filePath(QStringLiteral("destination")), 42);

    manager.process();

    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(successSpy.first().first().toULongLong(), 42ULL);
    QCOMPARE(finishedSpy.count(), 1);
    QVERIFY(QFile::exists(source));
    QVERIFY(QFile::exists(temporaryDir.filePath(QStringLiteral("destination/Series/source.cbz"))));
}

void ComicFilesManagerTest::movesComicAndRemovesSource()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());
    const QString source = createSourceComic(temporaryDir, QStringLiteral("source.cbz"));
    QVERIFY(!source.isEmpty());

    ComicFilesManager manager;
    QSignalSpy successSpy(&manager, &ComicFilesManager::success);
    manager.moveComicsTo({ { source, QString() } }, temporaryDir.filePath(QStringLiteral("destination")), 84);

    manager.process();

    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(successSpy.first().first().toULongLong(), 84ULL);
    QVERIFY(!QFile::exists(source));
    QVERIFY(QFile::exists(temporaryDir.filePath(QStringLiteral("destination/source.cbz"))));
}

QTEST_GUILESS_MAIN(ComicFilesManagerTest)

#include "main.moc"
