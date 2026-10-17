#include "staged_cover.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>

class StagedCoverTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void finishKeepsTheNewCover();
    void restoreAfterInstallKeepsThePreviousCover();
    void restoreBeforeInstallRemovesTheStagedFile();
    void restoreWithoutPreviousCoverLeavesNoCover();
    void installNeedsAStagedFile();

private:
    static QImage image(const QColor &color);
    static bool isMostly(const QString &path, Qt::GlobalColor color);
    bool hasLeftovers() const;

    QTemporaryDir directory;
    QString coverPath;
};

QImage StagedCoverTest::image(const QColor &color)
{
    QImage result(60, 90, QImage::Format_RGB32);
    result.fill(color);
    return result;
}

bool StagedCoverTest::isMostly(const QString &path, Qt::GlobalColor color)
{
    const QImage loaded(path);
    if (loaded.isNull())
        return false;
    const QColor pixel = loaded.pixelColor(loaded.width() / 2, loaded.height() / 2);
    return color == Qt::red ? pixel.red() > 200 && pixel.blue() < 60 : pixel.blue() > 200 && pixel.red() < 60;
}

bool StagedCoverTest::hasLeftovers() const
{
    return QFile::exists(coverPath + ".new.jpg") || QFile::exists(coverPath + ".old");
}

void StagedCoverTest::init()
{
    QVERIFY(directory.isValid());
    coverPath = QDir(directory.path()).filePath(QString("covers/%1.jpg").arg(QTest::currentTestFunction()));
}

void StagedCoverTest::finishKeepsTheNewCover()
{
    QVERIFY(QDir().mkpath(QFileInfo(coverPath).absolutePath()));
    QVERIFY(image(Qt::red).save(coverPath));

    StagedCover staged(coverPath);
    QVERIFY(staged.stage(image(Qt::blue)));
    QVERIFY(staged.isStaged());
    QVERIFY(isMostly(coverPath, Qt::red));

    QVERIFY(staged.install());
    QVERIFY(isMostly(coverPath, Qt::blue));
    staged.finish();

    QVERIFY(isMostly(coverPath, Qt::blue));
    QVERIFY(!hasLeftovers());
}

void StagedCoverTest::restoreAfterInstallKeepsThePreviousCover()
{
    QVERIFY(QDir().mkpath(QFileInfo(coverPath).absolutePath()));
    QVERIFY(image(Qt::red).save(coverPath));

    StagedCover staged(coverPath);
    QVERIFY(staged.stage(image(Qt::blue)));
    QVERIFY(staged.install());
    staged.restore();

    QVERIFY(isMostly(coverPath, Qt::red));
    QVERIFY(!hasLeftovers());
}

void StagedCoverTest::restoreBeforeInstallRemovesTheStagedFile()
{
    QVERIFY(QDir().mkpath(QFileInfo(coverPath).absolutePath()));
    QVERIFY(image(Qt::red).save(coverPath));

    StagedCover staged(coverPath);
    QVERIFY(staged.stage(image(Qt::blue)));
    staged.restore();

    QVERIFY(isMostly(coverPath, Qt::red));
    QVERIFY(!hasLeftovers());
}

void StagedCoverTest::restoreWithoutPreviousCoverLeavesNoCover()
{
    StagedCover staged(coverPath);
    QVERIFY(staged.stage(image(Qt::blue)));
    QVERIFY(staged.install());
    QVERIFY(isMostly(coverPath, Qt::blue));
    staged.restore();

    QVERIFY(!QFile::exists(coverPath));
    QVERIFY(!hasLeftovers());
}

void StagedCoverTest::installNeedsAStagedFile()
{
    QVERIFY(QDir().mkpath(QFileInfo(coverPath).absolutePath()));
    QVERIFY(image(Qt::red).save(coverPath));

    StagedCover staged(coverPath);
    QVERIFY(!staged.isStaged());
    QVERIFY(!staged.install());
    staged.restore();

    QVERIFY(isMostly(coverPath, Qt::red));
}

QTEST_GUILESS_MAIN(StagedCoverTest)
#include "main.moc"
