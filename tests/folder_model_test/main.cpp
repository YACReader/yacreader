#include "data_base_management.h"
#include "db_helper.h"
#include "folder_model.h"
#include "yacreader_global.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRandomGenerator>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

using namespace YACReader;

class FolderModelTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void reloadUpdatesAChangedFolder();
    void statusChangesUpdateTheRowsAndTheDatabase();
    void typeChangesTheWholeSubtree();
    void renameUpdatesThePathOfTheSubtree();
    void addedFolderStartsEmpty();
    void nullNumberOfChildrenIsZero();
    void deleteFolderDeletesItsUnusedMissingComics();
    void subfoldersHaveTheOrderOfTheFoldersTree();

private:
    std::unique_ptr<QTemporaryDir> root;
    QString library;
    QString dataPath;

    bool exec(const QString &sql);
    QString value(const QString &sql);
    QModelIndex indexOf(FolderModel &model, qulonglong id);
};

void FolderModelTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    library = QDir::cleanPath(root->filePath("library"));
    dataPath = LibraryPaths::libraryDataPath(library);
    QVERIFY(QDir().mkpath(dataPath));
    QVERIFY(QDir().mkpath(library + "/Series/Child/Leaf"));

    QString name;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(library));
        name = db.connectionName();
        db.close();
    }
    QSqlDatabase::removeDatabase(name);

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path, finished, completed, numChildren, type) VALUES "
                 "(2, 1, 'Series', '/Series', 0, 1, 1, 0), (3, 2, 'Child', '/Series/Child', 0, 1, 1, 0), (5, 3, 'Leaf', '/Series/Child/Leaf', 0, 1, 0, 0), (4, 1, 'Unknown', '/Unknown', 0, 1, NULL, 0)"));
}

void FolderModelTest::cleanup()
{
    root.reset();
}

bool FolderModelTest::exec(const QString &sql)
{
    bool ok = false;
    QString name;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        name = db.connectionName();
        QSqlQuery query(db);
        ok = query.exec(sql);
        if (!ok)
            qWarning() << query.lastError().text() << sql;
    }
    QSqlDatabase::removeDatabase(name);
    return ok;
}

QString FolderModelTest::value(const QString &sql)
{
    QString result;
    QString name;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        name = db.connectionName();
        QSqlQuery query(db);
        if (query.exec(sql) && query.next())
            result = query.value(0).toString();
    }
    QSqlDatabase::removeDatabase(name);
    return result;
}

QModelIndex FolderModelTest::indexOf(FolderModel &model, qulonglong id)
{
    return model.index(id);
}

void FolderModelTest::reloadUpdatesAChangedFolder()
{
    FolderModel model;
    model.setupModelData(dataPath);
    QSignalSpy changed(&model, &FolderModel::dataChanged);

    model.reload();
    QCOMPARE(changed.count(), 0);

    QVERIFY(exec("UPDATE folder SET finished = 1 WHERE id = 3"));
    model.reload();
    QCOMPARE(changed.count(), 1);
    QCOMPARE(model.data(indexOf(model, 3), FolderModel::FinishedRole).toBool(), true);
}

void FolderModelTest::statusChangesUpdateTheRowsAndTheDatabase()
{
    FolderModel model;
    model.setupModelData(dataPath);
    const auto series = indexOf(model, 2);

    model.updateFolderCompletedStatus({ series }, false);
    model.updateFolderFinishedStatus({ series }, true);

    QCOMPARE(model.data(series, FolderModel::CompletedRole).toBool(), false);
    QCOMPARE(model.data(series, FolderModel::FinishedRole).toBool(), true);
    QCOMPARE(value("SELECT completed FROM folder WHERE id = 2"), QString("0"));
    QCOMPARE(value("SELECT finished FROM folder WHERE id = 2"), QString("1"));
}

void FolderModelTest::typeChangesTheWholeSubtree()
{
    FolderModel model;
    model.setupModelData(dataPath);

    model.updateFolderType({ indexOf(model, 2) }, YACReader::FileType::Manga);

    QCOMPARE(model.data(indexOf(model, 2), FolderModel::TypeRole).value<YACReader::FileType>(), YACReader::FileType::Manga);
    QCOMPARE(model.data(indexOf(model, 3), FolderModel::TypeRole).value<YACReader::FileType>(), YACReader::FileType::Manga);
    QCOMPARE(value("SELECT type FROM folder WHERE id = 3"), QString::number(int(YACReader::FileType::Manga)));
}

void FolderModelTest::deleteFolderDeletesItsUnusedMissingComics()
{
    const auto cover = LibraryPaths::placeholderCoverPathFromDataPath(dataPath, "key");
    QVERIFY(QDir().mkpath(QFileInfo(cover).absolutePath()));
    QFile file(cover);
    QVERIFY(file.open(QIODevice::WriteOnly) && file.write("cover") > 0);
    file.close();
    QVERIFY(exec("INSERT INTO placeholder (id, series, coverKey) VALUES (5, 'Leaf', 'key'), (6, 'Unknown', NULL)"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (5, 5), (4, 6)"));

    FolderModel model;
    model.setupModelData(dataPath);
    model.deleteFolder(indexOf(model, 3));

    QCOMPARE(value("SELECT group_concat(id) FROM placeholder"), QString("6"));
    QCOMPARE(value("SELECT group_concat(folder_id) FROM folder_placeholder"), QString("4"));
    QVERIFY(!QFile::exists(cover));
}

void FolderModelTest::renameUpdatesThePathOfTheSubtree()
{
    FolderModel model;
    model.setupModelData(dataPath);

    QString error;
    QVERIFY2(model.renameFolder(indexOf(model, 3), "Renamed", &error), qPrintable(error));

    QCOMPARE(model.data(indexOf(model, 3), FolderModel::FolderNameRole).toString(), QString("Renamed"));
    QCOMPARE(model.getFolderPath(indexOf(model, 3)), QString("/Series/Renamed"));
    QCOMPARE(model.getFolderPath(indexOf(model, 5)), QString("/Series/Renamed/Leaf"));
    QCOMPARE(model.getFolder(indexOf(model, 5)).path, QString("/Series/Renamed/Leaf"));
    QCOMPARE(value("SELECT path FROM folder WHERE id = 5"), QString("/Series/Renamed/Leaf"));
}

void FolderModelTest::addedFolderStartsEmpty()
{
    FolderModel model;
    model.setupModelData(dataPath);

    const auto added = model.addFolderAtParent("New", indexOf(model, 2));
    QVERIFY(added.isValid());
    QCOMPARE(model.data(added, FolderModel::FolderNameRole).toString(), QString("New"));
    QCOMPARE(model.data(added, FolderModel::NumChildrenRole).toInt(), 0);
    QCOMPARE(model.data(added, FolderModel::FinishedRole).toBool(), false);
    QCOMPARE(model.data(added, FolderModel::CompletedRole).toBool(), true);
    QCOMPARE(model.getFolderPath(added), QString("/Series/New"));
    QCOMPARE(model.getFolder(added).parentId, FolderId(2));
}

void FolderModelTest::nullNumberOfChildrenIsZero()
{
    FolderModel model;
    model.setupModelData(dataPath);
    QCOMPARE(model.getFolder(indexOf(model, 4)).numChildren, 0);
    QCOMPARE(model.getFolder(indexOf(model, 2)).numChildren, 1);
}

// the server sends the subfolders of a folder in the order of the desktop tree
void FolderModelTest::subfoldersHaveTheOrderOfTheFoldersTree()
{
    QVERIFY(exec("DELETE FROM folder WHERE id <> 1"));
    const QStringList names = { "Batman", "batman", "BATMAN", "Vol 2", "Vol 10", "vol 2", "Été", "ete", "X", "x-men", "X-Men", "10", "9" };
    QRandomGenerator random(20261024);
    // as the scanner writes them: a parent has a lower id than its folders, and the names in one folder are different
    QList<int> added = { 1 };
    QHash<int, QStringList> namesIn;
    for (int id = 2; id <= 120; id++) {
        const auto parent = added.at(random.bounded(int(added.size())));
        auto &used = namesIn[parent];
        auto name = names.at(random.bounded(int(names.size())));
        if (used.contains(name))
            name += QString(" %1").arg(id);
        used.append(name);
        QVERIFY(exec(QString("INSERT INTO folder (id, parentId, name, path) VALUES (%1, %2, '%3', '/%1')").arg(id).arg(parent).arg(name)));
        added.append(id);
    }

    FolderModel model;
    model.setupModelData(dataPath);
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        connectionName = db.connectionName();
        for (const auto parent : std::as_const(added)) {
            const auto parentIndex = parent == 1 ? QModelIndex() : indexOf(model, parent);
            QList<qulonglong> tree;
            for (int row = 0; row < model.rowCount(parentIndex); row++)
                tree.append(model.data(model.index(row, 0, parentIndex), FolderModel::IdRole).toULongLong());

            const auto subfolders = DBHelper::loadSubfolders(FolderId(parent), db);
            QVERIFY(subfolders);
            QList<qulonglong> loaded;
            for (const auto &folder : *subfolders)
                loaded.append(folder.id.value());
            QCOMPARE(loaded, tree);
        }
    }
    QSqlDatabase::removeDatabase(connectionName);
}

QTEST_MAIN(FolderModelTest)

#include "main.moc"
