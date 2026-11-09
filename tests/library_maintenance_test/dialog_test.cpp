#include "add_library_dialog.h"
#include "create_library_dialog.h"
#include "export_library_dialog.h"
#include "import_library_dialog.h"
#include "library_maintenance_dialog.h"
#include "library_management_coordinator.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

class LibraryMaintenanceDialogTest : public QObject
{
    Q_OBJECT
private slots:
    void recovery_data();
    void recovery();
    void busyAndFilesystemErrors_data();
    void busyAndFilesystemErrors();
    void coordinatorRecovery_data();
    void coordinatorRecovery();
};

void LibraryMaintenanceDialogTest::recovery_data()
{
    QTest::addColumn<bool>("cancel");
    QTest::addColumn<bool>("changeOwner");
    QTest::newRow("recover") << false << false;
    QTest::newRow("cancel") << true << false;
    QTest::newRow("owner-changed") << false << true;
}

void LibraryMaintenanceDialogTest::recovery()
{
    QFETCH(bool, cancel);
    QFETCH(bool, changeOwner);
    QTemporaryDir root;
    QVERIFY(root.isValid());
    const auto data = YACReader::LibraryPaths::libraryDataPath(root.path());
    QVERIFY(QDir().mkpath(data));
    QFile record(QDir(data).filePath("maintenance.lock"));
    const QByteArray original("123\nold-app\nold-computer\nold-machine\n");
    const QByteArray successor("456\nnew-app\nnew-computer\nnew-machine\n");
    QVERIFY(record.open(QIODevice::WriteOnly));
    QCOMPARE(record.write(original), original.size());
    record.close();
    bool offeredRecovery = false;
    bool explainedChange = false;
    QTimer driver;
    connect(&driver, &QTimer::timeout, this, [&] {
        auto box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (!box)
            return;
        if (box->icon() == QMessageBox::Critical) {
            explainedChange = box->text().contains("owner changed");
            box->accept();
            return;
        }
        for (auto button : box->buttons()) {
            if (button->text() == "Recover and retry") {
                offeredRecovery = true;
                if (changeOwner) {
                    if (record.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                        record.write(successor);
                        record.close();
                    }
                }
                if (cancel)
                    box->button(QMessageBox::Cancel)->click();
                else
                    button->click();
                return;
            }
        }
        box->reject();
    });
    driver.start(10);
    LibraryMaintenanceLock lock(root.path());
    QCOMPARE(acquireLibraryMaintenanceWithRecovery(lock, root.path(), nullptr, "Test"), !cancel && !changeOwner);
    QVERIFY(offeredRecovery);
    if (cancel || changeOwner) {
        QVERIFY(record.open(QIODevice::ReadOnly));
        QCOMPARE(record.readAll(), changeOwner ? successor : original);
    } else {
        LibraryMaintenanceLock contender(root.path());
        QVERIFY(!contender.tryLock());
    }
    QCOMPARE(explainedChange, changeOwner);
}

void LibraryMaintenanceDialogTest::busyAndFilesystemErrors_data()
{
    QTest::addColumn<bool>("busy");
    QTest::newRow("busy") << true;
    QTest::newRow("missing-folder") << false;
}

void LibraryMaintenanceDialogTest::busyAndFilesystemErrors()
{
    QFETCH(bool, busy);
    QTemporaryDir root;
    QVERIFY(root.isValid());
    const auto data = YACReader::LibraryPaths::libraryDataPath(root.path());
    QLockFile owner(QDir(data).filePath("maintenance.lock"));
    if (busy) {
        QVERIFY(QDir().mkpath(data));
        QVERIFY(owner.tryLock());
    }
    bool sawDialog = false;
    bool offeredRecovery = false;
    bool offeredRetry = false;
    QTimer driver;
    connect(&driver, &QTimer::timeout, this, [&] {
        auto box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (!box)
            return;
        sawDialog = true;
        for (auto button : box->buttons()) {
            offeredRecovery |= button->text() == "Recover and retry";
            offeredRetry |= button->text() == "Retry";
        }
        box->reject();
    });
    driver.start(10);
    LibraryMaintenanceLock lock(root.path());
    QVERIFY(!acquireLibraryMaintenanceWithRecovery(lock, root.path(), nullptr, "Test"));
    QVERIFY(sawDialog);
    QVERIFY(!offeredRecovery);
    QCOMPARE(offeredRetry, busy);
}

void LibraryMaintenanceDialogTest::coordinatorRecovery_data()
{
    QTest::addColumn<QString>("scenario");
    QTest::newRow("missing-metadata") << QString("folder");
    QTest::newRow("missing-database") << QString("database");
    QTest::newRow("backup-failure") << QString("backup");
    QTest::newRow("recover-menu-reloads") << QString("recover");
}

void LibraryMaintenanceDialogTest::coordinatorRecovery()
{
    QFETCH(QString, scenario);
    QTemporaryDir root;
    QVERIFY(root.isValid());
    const auto data = YACReader::LibraryPaths::libraryDataPath(root.path());
    const auto database = YACReader::LibraryPaths::libraryDatabasePath(root.path());
    QVERIFY(QDir().mkpath(data));
    QString connection;
    {
        auto db = DataBaseManagement::createDatabase(database);
        QVERIFY(db.isOpen());
        connection = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connection);
    YACReaderLibraries libraries;
    libraries.addLibrary("Library", root.path());
    QCOMPARE(libraries.getPath("Library"), root.path());
    if (scenario == "folder") {
        QVERIFY(QDir(data).removeRecursively());
    } else if (scenario == "database") {
        QVERIFY(QFile::remove(database));
    } else if (scenario == "backup") {
        QFile blocker(QDir(data).filePath("backups"));
        QVERIFY(blocker.open(QIODevice::WriteOnly));
    } else {
        QVERIFY(QFile::rename(database, database + ".rollback"));
        QFile marker(database + ".restore");
        QVERIFY(marker.open(QIODevice::WriteOnly));
        QVERIFY(marker.write("original=1\n") > 0);
        QFile lock(QDir(data).filePath("maintenance.lock"));
        QVERIFY(lock.open(QIODevice::WriteOnly));
        QVERIFY(lock.write("123\nold-app\nold-host\nold-machine\n") > 0);
    }
    QWidget parent;
    CreateLibraryDialog create;
    AddLibraryDialog add;
    ExportLibraryDialog exportDialog;
    ImportLibraryDialog import;
    QSettings settings(root.filePath("settings.ini"), QSettings::IniFormat);
    LibraryManagementCoordinator coordinator(&settings, libraries, &parent, &create, &add, &exportDialog, &import, nullptr, [] { return QString("Library"); }, [] { return QModelIndex(); }, "Library info");
    QSignalSpy aborted(&coordinator, &LibraryManagementCoordinator::operationAborted);
    QSignalSpy failed(&coordinator, &LibraryManagementCoordinator::updateFailed);
    QSignalSpy ready(&coordinator, &LibraryManagementCoordinator::libraryReady);
    bool recoveryOffered = false;
    QTimer driver;
    connect(&driver, &QTimer::timeout, this, [&] {
        auto box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (!box)
            return;
        for (auto button : box->buttons()) {
            if (button->text() == "Recover and retry") {
                recoveryOffered = true;
                button->click();
                return;
            }
        }
        box->accept();
    });
    driver.start(10);
    if (scenario == "recover") {
        coordinator.recoverCurrentLibraryAccess();
        QVERIFY(recoveryOffered);
        QCOMPARE(ready.count(), 1);
        QCOMPARE(ready.first().first().toString(), data);
        QVERIFY(DataBaseManagement::isLibraryDatabaseValid(root.path()));
        QVERIFY(!QFile::exists(database + ".restore"));
    } else {
        coordinator.updateCurrentLibrary();
        QTRY_COMPARE_WITH_TIMEOUT(aborted.count(), 1, 10000);
        QCOMPARE(failed.count(), 1);
        QCOMPARE(ready.count(), 0);
        QVERIFY(!recoveryOffered);
    }
    coordinator.stop();
}

QTEST_MAIN(LibraryMaintenanceDialogTest)
#include "dialog_test.moc"
