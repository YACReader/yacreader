#include "comic_db.h"
#include "comic_model.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "yacreader_global.h"

#include <QDir>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

#include <functional>
#include <memory>

using namespace YACReader;

class ComicModelTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void listRowsHaveTheirList();
    void reloadUpdatesChangedRows();
    void reloadAddsAndRemovesRows();
    void readingReloadKeepsTheLastOpenedOrder();
    void recentReloadKeepsTheAddedOrder();
    void comicUpdateChangesEveryRowOfTheComicInfo();
    void setReadChangesEveryRowOfTheComicInfo();
    void ratingIsARole();

private:
    std::unique_ptr<QTemporaryDir> root;
    QString dataPath;

    bool exec(const QString &sql);
    QList<int> rowsOfComicInfo(ComicModel &model, qulonglong comicInfoId);
};

void ComicModelTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    const auto library = QDir::cleanPath(root->filePath("library"));
    dataPath = LibraryPaths::libraryDataPath(library);
    QVERIFY(QDir().mkpath(dataPath));

    QString name;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(library));
        name = db.connectionName();
        db.close();
    }
    QSqlDatabase::removeDatabase(name);

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (2, 1, 'Series', '/Series')"));
    for (int i = 1; i <= 4; i++) {
        QVERIFY(exec(QString("INSERT INTO comic_info (id, hash, title, number, numPages, currentPage, read, hasBeenOpened, rating, added, lastTimeOpened) "
                             "VALUES (%1, 'hash-%1', 'Title %1', '%1', 20, 3, 0, 1, %1, %2, %3)")
                             .arg(i)
                             .arg(1000 + i)
                             .arg(2000 + i)));
        QVERIFY(exec(QString("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (%1, 2, %1, 'c%1.cbz', '/Series/c%1.cbz')").arg(i)));
    }
    // a second file with the same content shares comic_info 1
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (5, 2, 1, 'c5.cbz', '/Series/c5.cbz')"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'List')"));
    QVERIFY(exec("INSERT INTO comic_reading_list (reading_list_id, comic_id, ordering) VALUES (1, 3, 0), (1, 1, 1)"));
}

void ComicModelTest::cleanup()
{
    root.reset();
}

bool ComicModelTest::exec(const QString &sql)
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

QList<int> ComicModelTest::rowsOfComicInfo(ComicModel &model, qulonglong comicInfoId)
{
    QList<int> rows;
    for (int row = 0; row < model.rowCount(); row++) {
        if (model.data(model.index(row, ComicModel::Hash), Qt::DisplayRole).toString() == QString("hash-%1").arg(comicInfoId))
            rows.append(row);
    }
    return rows;
}

void ComicModelTest::listRowsHaveTheirList()
{
    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0, 0), ComicModel::IdRole).toULongLong(), 3ULL);
    QCOMPARE(model.data(model.index(1, 0), ComicModel::IdRole).toULongLong(), 1ULL);
    QCOMPARE(model.columnCount(), int(ComicModel::Size) + 1);
}

void ComicModelTest::reloadUpdatesChangedRows()
{
    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    QCOMPARE(model.rowCount(), 5);
    QSignalSpy changed(&model, &ComicModel::dataChanged);

    model.reload();
    QCOMPARE(changed.count(), 0);

    QVERIFY(exec("UPDATE comic_info SET title = 'Changed' WHERE id = 2"));
    model.reload();
    QCOMPARE(changed.count(), 1);
    const auto row = changed.first().at(0).toModelIndex().row();
    QCOMPARE(model.data(model.index(row, ComicModel::Title), Qt::DisplayRole).toString(), QString("Changed"));
}

void ComicModelTest::reloadAddsAndRemovesRows()
{
    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    QCOMPARE(model.rowCount(), 5);

    QVERIFY(exec("DELETE FROM comic WHERE id = 3"));
    QVERIFY(exec("INSERT INTO comic_info (id, hash, title, number, numPages) VALUES (6, 'hash-6', 'Title 6', '6', 20)"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (6, 2, 6, 'c6.cbz', '/Series/c6.cbz')"));
    model.reload();

    QList<qulonglong> ids;
    for (int row = 0; row < model.rowCount(); row++)
        ids.append(model.data(model.index(row, 0), ComicModel::IdRole).toULongLong());
    QCOMPARE(ids, QList<qulonglong>({ 1, 5, 2, 4, 6 }));
}

void ComicModelTest::readingReloadKeepsTheLastOpenedOrder()
{
    ComicModel model;
    model.setupReadingModelData(dataPath);
    QCOMPARE(model.data(model.index(0, 0), ComicModel::IdRole).toULongLong(), 4ULL);

    // comic 2 is opened again, so it moves to the top: one insert and one removal, the other rows stay
    QVERIFY(exec("UPDATE comic_info SET lastTimeOpened = 3000 WHERE id = 2"));
    QSignalSpy inserted(&model, &ComicModel::rowsInserted);
    QSignalSpy removed(&model, &ComicModel::rowsRemoved);
    model.reload();
    QCOMPARE(inserted.count(), 1);
    QCOMPARE(removed.count(), 1);

    QList<qulonglong> ids;
    for (int row = 0; row < model.rowCount(); row++)
        ids.append(model.data(model.index(row, 0), ComicModel::IdRole).toULongLong());
    QCOMPARE(ids.first(), 2ULL);
    QCOMPARE(ids.count(2), 1);
    QCOMPARE(ids.size(), 5);
}

void ComicModelTest::recentReloadKeepsTheAddedOrder()
{
    QVERIFY(exec(QString("UPDATE comic_info SET added = %1 - id").arg(QDateTime::currentSecsSinceEpoch())));
    ComicModel model;
    model.setupRecentModelData(dataPath);
    QCOMPARE(model.data(model.index(0, 0), ComicModel::IdRole).toULongLong(), 1ULL);

    QVERIFY(exec(QString("UPDATE comic_info SET added = %1 WHERE id = 2").arg(QDateTime::currentSecsSinceEpoch() + 10)));
    QSignalSpy inserted(&model, &ComicModel::rowsInserted);
    QSignalSpy removed(&model, &ComicModel::rowsRemoved);
    model.reload();
    QCOMPARE(inserted.count(), 1);
    QCOMPARE(removed.count(), 1);

    QList<qulonglong> ids;
    for (int row = 0; row < model.rowCount(); row++)
        ids.append(model.data(model.index(row, 0), ComicModel::IdRole).toULongLong());
    QCOMPARE(ids.first(), 2ULL);
    QCOMPARE(ids.count(2), 1);
    QCOMPARE(ids.size(), 5);
}

void ComicModelTest::comicUpdateChangesEveryRowOfTheComicInfo()
{
    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    const auto rows = rowsOfComicInfo(model, 1);
    QCOMPARE(rows.size(), 2);

    ComicDB comic;
    comic.file.id = ComicId(1);
    comic.info.id = ComicInfoId(1);
    comic.info.read = true;
    comic.info.currentPage = 20;

    QSignalSpy changed(&model, &ComicModel::dataChanged);
    model.reload(comic);
    QCOMPARE(changed.count(), 2);
    for (const auto row : rows) {
        QCOMPARE(model.data(model.index(row, 0), ComicModel::ReadColumnRole).toBool(), true);
        QCOMPARE(model.data(model.index(row, 0), ComicModel::CurrentPageRole).toInt(), 20);
    }
}

void ComicModelTest::setReadChangesEveryRowOfTheComicInfo()
{
    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    const auto rows = rowsOfComicInfo(model, 1);
    QCOMPARE(rows.size(), 2);

    model.setComicsRead({ model.index(rows.first(), 0) }, YACReader::Unread);
    for (const auto row : rows) {
        QCOMPARE(model.data(model.index(row, 0), ComicModel::ReadColumnRole).toBool(), false);
        QCOMPARE(model.data(model.index(row, 0), ComicModel::CurrentPageRole).toInt(), 1);
        QCOMPARE(model.data(model.index(row, 0), ComicModel::HasBeenOpenedRole).toInt(), 0);
    }

    model.setComicsRead({ model.index(rows.last(), 0) }, YACReader::Read);
    for (const auto row : rows)
        QCOMPARE(model.data(model.index(row, 0), ComicModel::ReadColumnRole).toBool(), true);
}

void ComicModelTest::ratingIsARole()
{
    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    const auto rows = rowsOfComicInfo(model, 3);
    QCOMPARE(rows.size(), 1);
    QCOMPARE(model.data(model.index(rows.first(), ComicModel::Rating), ComicModel::RatingRole).toInt(), 3);

    model.updateRating(5, model.index(rows.first(), 0));
    QCOMPARE(model.data(model.index(rows.first(), ComicModel::Rating), ComicModel::RatingRole).toInt(), 5);
}

QTEST_MAIN(ComicModelTest)

#include "main.moc"
