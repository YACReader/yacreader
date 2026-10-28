#include "comic_info_repairer.h"
#include "data_base_management.h"
#include "yacreader_global.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

using namespace YACReader;

class LibraryMaintenanceTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void backupAndRestoreKeepListData();
    void repairLeavesDeletedComicsAlone();

private:
    bool exec(const QString &sql);
    QList<QVariantList> rows(const QString &sql);
    QList<QVariantList> listData();

    std::unique_ptr<QTemporaryDir> root;
    QString libraryPath;
    QString dataPath;
};

void LibraryMaintenanceTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    libraryPath = QDir::cleanPath(root->filePath("library"));
    dataPath = LibraryPaths::libraryDataPath(libraryPath);
    QVERIFY(QDir().mkpath(LibraryPaths::libraryCoversPathFromLibraryDataPath(dataPath)));

    QString name;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(libraryPath));
        name = db.connectionName();
        db.close();
    }
    QSqlDatabase::removeDatabase(name);

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (2, 1, 'Series', '/Series')"));
    for (int i = 1; i <= 4; i++) {
        QVERIFY(exec(QString("INSERT INTO comic_info (id, hash, title, numPages, coverSizeRatio) VALUES (%1, 'hash-%1', 'Title %1', 20, 0.66)").arg(i)));
        QVERIFY(exec(QString("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (%1, 2, %1, 'c%1.cbz', '/Series/c%1.cbz')").arg(i)));
    }
    QVERIFY(exec("INSERT INTO placeholder (id, title, coverKey) VALUES (1, 'Expected issue', 'cover-1')"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'List')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id, placeholder_id) VALUES "
                 "(1, 0, 2, NULL), (1, 1, 4, NULL), (1, 2, NULL, 1), (1, 3, 4, NULL), (1, 4, 3, NULL)"));
    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Tag', 'red', 0)"));
    QVERIFY(exec("INSERT INTO label_entry (label_id, ordering, comic_info_id, placeholder_id) VALUES (1, 0, 4, NULL), (1, 1, NULL, 1), (1, 2, 3, NULL)"));

    // comic_info 4 loses its file but stays, because the lists refer to it
    QVERIFY(exec("DELETE FROM comic WHERE id = 4"));
}

void LibraryMaintenanceTest::cleanup()
{
    root.reset();
}

bool LibraryMaintenanceTest::exec(const QString &sql)
{
    QString name;
    bool ok = false;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        name = db.connectionName();
        QSqlQuery query(db);
        ok = query.exec(sql);
        if (!ok)
            qWarning() << sql;
    }
    QSqlDatabase::removeDatabase(name);
    return ok;
}

QList<QVariantList> LibraryMaintenanceTest::rows(const QString &sql)
{
    QList<QVariantList> result;
    QString name;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        name = db.connectionName();
        QSqlQuery query(db);
        if (query.exec(sql)) {
            const auto columns = query.record().count();
            while (query.next()) {
                QVariantList row;
                for (int i = 0; i < columns; i++)
                    row.append(query.value(i));
                result.append(row);
            }
        } else {
            qWarning() << sql;
        }
    }
    QSqlDatabase::removeDatabase(name);
    return result;
}

QList<QVariantList> LibraryMaintenanceTest::listData()
{
    return rows("SELECT id, reading_list_id, ordering, comic_info_id, placeholder_id, last_known_name, last_known_path FROM reading_list_entry ORDER BY id") +
            rows("SELECT id, label_id, ordering, comic_info_id, placeholder_id, last_known_name, last_known_path FROM label_entry ORDER BY id") +
            rows("SELECT id, title, coverKey FROM placeholder ORDER BY id") +
            rows("SELECT id, hash, title FROM comic_info ORDER BY id") +
            rows("SELECT name FROM sqlite_master WHERE type = 'trigger' ORDER BY name");
}

void LibraryMaintenanceTest::backupAndRestoreKeepListData()
{
    const auto before = listData();
    QCOMPARE(rows("SELECT last_known_name, last_known_path FROM reading_list_entry WHERE comic_info_id = 4"),
             QList<QVariantList>({ { "c4.cbz", "/Series/c4.cbz" }, { "c4.cbz", "/Series/c4.cbz" } }));

    const auto backupPath = root->filePath("backup.ydb");
    QString error;
    QVERIFY2(DataBaseManagement::backupLibrary(libraryPath, DatabaseBackupReason::Manual, &error, backupPath), qPrintable(error));

    QVERIFY(exec("DELETE FROM reading_list_entry"));
    QVERIFY(exec("DELETE FROM label_entry"));
    QVERIFY(exec("DELETE FROM placeholder"));
    QVERIFY(exec("DELETE FROM comic_info WHERE id = 4"));
    QVERIFY(listData() != before);

    const auto result = DataBaseManagement::restoreLibrary(libraryPath, backupPath);
    QVERIFY2(result.success(), qPrintable(result.error));
    QCOMPARE(listData(), before);

    // the trigger still records the last known file
    QVERIFY(exec("DELETE FROM comic WHERE id = 3"));
    QCOMPARE(rows("SELECT last_known_name FROM reading_list_entry WHERE comic_info_id = 3"), QList<QVariantList>({ { "c3.cbz" } }));
    QCOMPARE(rows("SELECT last_known_name FROM label_entry WHERE comic_info_id = 3"), QList<QVariantList>({ { "c3.cbz" } }));
}

void LibraryMaintenanceTest::repairLeavesDeletedComicsAlone()
{
    // a deleted comic that a repair would change if it had a file
    QVERIFY(exec("UPDATE comic_info SET numPages = NULL, coverSizeRatio = NULL WHERE id = 4"));
    const auto deletedInfo = rows("SELECT * FROM comic_info WHERE id = 4");

    // what a crash in a cover save leaves: only the copy of the previous cover
    const auto coverPath = LibraryPaths::coverPathFromLibraryDataPath(dataPath, "hash-4");
    QImage cover(60, 90, QImage::Format_RGB32);
    cover.fill(Qt::red);
    QVERIFY(cover.save(coverPath + ".old", "JPG"));

    YACReader::ComicInfoRepairer repairer(nullptr);
    repairer.repairLibrary(libraryPath, dataPath);
    QVERIFY(repairer.wait(30000));

    const auto summary = repairer.summary();
    QVERIFY2(summary.error.isEmpty(), qPrintable(summary.error));
    QCOMPARE(summary.repaired, 0);
    QCOMPARE(summary.failed, 0);
    // the comics with a file (that is not on disk); the deleted comic is not one of them
    QCOMPARE(summary.missingFiles, 3);
    QVERIFY(!summary.missingFilePaths.contains("/Series/c4.cbz"));

    QCOMPARE(rows("SELECT * FROM comic_info WHERE id = 4"), deletedInfo);
    QVERIFY(QFile::exists(coverPath));
    QVERIFY(!QFile::exists(coverPath + ".old"));
    QCOMPARE(rows("SELECT COUNT(*) FROM reading_list_entry WHERE comic_info_id = 4"), QList<QVariantList>({ { 2 } }));
}

QTEST_GUILESS_MAIN(LibraryMaintenanceTest)

#include "main.moc"
