#include "comic_db.h"
#include "comic_model.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "search_query.h"
#include "yacreader_global.h"
#include "yacreader_global_gui.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMimeData>
#include <QSet>
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
    void identityRolesFollowTheCase();
    void rowIdsNameEachOccurrence();
    void dragDataCarriesComicInfoAndEntries();
    void reorderMovesTheDraggedOccurrence();
    void listShowsTheFirstFileOfEachComicInfo();
    void removeFromAnAggregatedReadingListUsesTheOwningList();
    void removeOneOccurrenceOfARepeatedComic();
    void addAppendsTheComicInfoOfEachRow();
    void reorderWritesTheEntryOrder();
    void failedReorderShowsTheStoredOrder();
    void reorderMovesADeletedComic();
    void deletedComicRowsShowTheirInfoAndLastKnownFile();
    void deletedComicCoverIsTheHashCoverOrTheDefaultImage();
    void deletedComicsSupportOnlyComicInfoActions();
    void comicInfoActionsWriteDeletedComics();
    void reloadUpdatesChangedRows();
    void reloadAddsAndRemovesRows();
    void readingReloadKeepsTheLastOpenedOrder();
    void recentReloadKeepsTheAddedOrder();
    void comicUpdateChangesEveryRowOfTheComicInfo();
    void setReadChangesEveryRowOfTheComicInfo();
    void ratingIsARole();
    void sharedViewsShowComicsWithFiles_data();
    void sharedViewsShowComicsWithFiles();
    void sharedViewAfterAListHasNoDeletedRows_data();
    void sharedViewAfterAListHasNoDeletedRows();
    void favoritesUseTheFileOfTheRow();
    void removeFromReadingUsesTheComicOfTheRow();

private:
    void loadSource(ComicModel &model, const QString &source);
    std::unique_ptr<QTemporaryDir> root;
    QString dataPath;

    bool exec(const QString &sql);
    QList<int> ints(const QString &sql);
    QList<int> rowsOfComicInfo(ComicModel &model, qulonglong comicInfoId);
    QList<qulonglong> rowIds(ComicModel &model);
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
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES (1, 0, 3), (1, 1, 1)"));
    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Tag', 'red', 0)"));
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

QList<int> ComicModelTest::ints(const QString &sql)
{
    QList<int> values;
    QString name;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        name = db.connectionName();
        QSqlQuery query(db);
        if (!query.exec(sql))
            qWarning() << query.lastError().text() << sql;
        while (query.next())
            values.append(query.value(0).toInt());
    }
    QSqlDatabase::removeDatabase(name);
    return values;
}

QList<qulonglong> ComicModelTest::rowIds(ComicModel &model)
{
    QList<qulonglong> ids;
    for (int row = 0; row < model.rowCount(); row++)
        ids.append(model.data(model.index(row, 0), ComicModel::IdRole).toULongLong());
    return ids;
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
    QCOMPARE(model.columnCount(), int(ComicModel::State) + 1);
}

void ComicModelTest::identityRolesFollowTheCase()
{
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name) VALUES (2, 1, 0, 'Child')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, comic_info_id) VALUES (40, 2, 0, 4)"));
    const auto entryIds = ints("SELECT id FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering");

    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    const auto folderRow = model.index(0, 0);
    QCOMPARE(folderRow.data(ComicModel::KindRole).toString(), QString("Comic"));
    QCOMPARE(folderRow.data(ComicModel::AvailableRole).toBool(), true);
    QVERIFY(folderRow.data(ComicModel::LastKnownNameRole).isNull());
    QVERIFY(folderRow.data(ComicModel::ListItemIdRole).isNull());
    QVERIFY(folderRow.data(ComicModel::ListIdRole).isNull());

    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(model.rowCount(), 3);
    const QList<qulonglong> expectedEntries = { qulonglong(entryIds.at(0)), qulonglong(entryIds.at(1)), 40 };
    const QList<qulonglong> expectedLists = { 1, 1, 2 };
    for (int row = 0; row < model.rowCount(); row++) {
        const auto index = model.index(row, 0);
        QCOMPARE(index.data(ComicModel::KindRole).toString(), QString("ComicInList"));
        QCOMPARE(index.data(ComicModel::AvailableRole).toBool(), true);
        QVERIFY(index.data(ComicModel::LastKnownNameRole).isNull());
        QCOMPARE(index.data(ComicModel::ListItemIdRole).toULongLong(), expectedEntries.at(row));
        QCOMPARE(index.data(ComicModel::ListIdRole).toULongLong(), expectedLists.at(row));
    }

    const auto roles = model.roleNames();
    QCOMPARE(roles.value(ComicModel::KindRole), QByteArray("kind"));
    QCOMPARE(roles.value(ComicModel::AvailableRole), QByteArray("available"));
    QCOMPARE(roles.value(ComicModel::LastKnownNameRole), QByteArray("last_known_name"));
    QCOMPARE(roles.value(ComicModel::ListItemIdRole), QByteArray("list_item_id"));
    QCOMPARE(roles.value(ComicModel::ListIdRole), QByteArray("list_id"));
}

void ComicModelTest::rowIdsNameEachOccurrence()
{
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, comic_info_id) VALUES (30, 1, 2, 3)"));
    const auto entryIds = ints("SELECT id FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering");

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(rowIds(model), QList<qulonglong>({ 3, 1, 3 }));
    QVERIFY(model.rowId(model.index(2, 0)) == ComicRowId(ListItemId { ListKind::ReadingList, 30 }));

    const auto indexes = model.indexesOf({ ListItemId { ListKind::ReadingList, 30 }, ListItemId { ListKind::ReadingList, qulonglong(entryIds.first()) }, ListItemId { ListKind::Label, 30 }, ComicId(3) });
    QCOMPARE(indexes.size(), 4);
    QCOMPARE(indexes.at(0).row(), 2);
    QCOMPARE(indexes.at(1).row(), 0);
    QVERIFY(!indexes.at(2).isValid());
    QVERIFY(!indexes.at(3).isValid());
    QCOMPARE(model.indexOf(ListItemId { ListKind::ReadingList, 30 }).row(), 2);

    model.setupFolderModelData(2, dataPath);
    QVERIFY(model.rowId(model.index(0, 0)) == ComicRowId(ComicId(model.index(0, 0).data(ComicModel::IdRole).toULongLong())));
    QCOMPARE(model.indexOf(ComicId(4)).data(ComicModel::IdRole).toULongLong(), 4ULL);
}

void ComicModelTest::dragDataCarriesComicInfoAndEntries()
{
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, comic_info_id) VALUES (30, 1, 2, 3)"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    std::unique_ptr<QMimeData> data(model.mimeData({ model.index(2, 0), model.index(1, 0) }));
    QCOMPARE(YACReader::mimeDataToComicsIds(data.get()), QList<qulonglong>({ 3, 1 }));
    QCOMPARE(YACReader::mimeDataToComicInfoIds(data.get()), QList<ComicInfoId>({ ComicInfoId(3), ComicInfoId(1) }));
    const auto entries = YACReader::mimeDataToListEntries(data.get());
    QCOMPARE(entries.size(), 2);
    QVERIFY(entries.first() == (ListItemId { ListKind::ReadingList, 30 }));

    model.setupFolderModelData(2, dataPath);
    data.reset(model.mimeData({ model.index(0, 0) }));
    QCOMPARE(YACReader::mimeDataToComicInfoIds(data.get()).size(), 1);
    QVERIFY(YACReader::mimeDataToListEntries(data.get()).isEmpty());
}

void ComicModelTest::reorderMovesTheDraggedOccurrence()
{
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, comic_info_id) VALUES (30, 1, 2, 3)"));
    const auto entryIds = ints("SELECT id FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering");

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    std::unique_ptr<QMimeData> data(model.mimeData({ model.index(2, 0) }));

    QVERIFY(model.dropMimeData(data.get(), Qt::MoveAction, 1, 0, QModelIndex()));
    QCOMPARE(ints("SELECT id FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering"), QList<int>({ entryIds.at(0), 30, entryIds.at(1) }));
    QCOMPARE(model.rowId(model.index(1, 0)), ComicRowId(ListItemId { ListKind::ReadingList, 30 }));
}

void ComicModelTest::listShowsTheFirstFileOfEachComicInfo()
{
    // comic_info 1 has the files 1 and 5; comic_info 3 has no file after this
    QVERIFY(exec("DELETE FROM comic WHERE id IN (1, 3)"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(rowIds(model), QList<qulonglong>({ 0, 5 }));
    QCOMPARE(model.index(0, 0).data(ComicModel::KindRole).toString(), QString("DeletedComicInList"));
    QCOMPARE(model.index(1, 0).data(ComicModel::KindRole).toString(), QString("ComicInList"));
}

void ComicModelTest::removeFromAnAggregatedReadingListUsesTheOwningList()
{
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name) VALUES (2, 1, 0, 'Child')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES (2, 0, 4)"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(rowIds(model), QList<qulonglong>({ 3, 1, 4 }));

    model.deleteComicsFromLists({ model.index(2, 0) });
    QCOMPARE(rowIds(model), QList<qulonglong>({ 3, 1 }));
    QCOMPARE(ints("SELECT comic_info_id FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering"), QList<int>({ 3, 1 }));
    QCOMPARE(ints("SELECT COUNT(*) FROM reading_list_entry WHERE reading_list_id = 2"), QList<int>({ 0 }));
}

void ComicModelTest::removeOneOccurrenceOfARepeatedComic()
{
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES (1, 2, 3)"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(rowIds(model), QList<qulonglong>({ 3, 1, 3 }));

    // the same row twice is one removal
    model.deleteComicsFromLists({ model.index(2, 0), model.index(2, 0) });
    QCOMPARE(rowIds(model), QList<qulonglong>({ 3, 1 }));
    QCOMPARE(ints("SELECT ordering FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering"), QList<int>({ 0, 1 }));
}

void ComicModelTest::addAppendsTheComicInfoOfEachRow()
{
    ComicModel model;
    model.setupFolderModelData(2, dataPath);

    // comic 5 is a second file of comic_info 1
    model.addComicsToReadingList(model.getIndexesFromIds({ 5 }), 1);
    model.addComicsToLabel(model.getIndexesFromIds({ 5 }), 1);
    model.addComicsToLabel(model.getIndexesFromIds({ 1 }), 1);

    QCOMPARE(ints("SELECT comic_info_id FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering"), QList<int>({ 3, 1, 1 }));
    QCOMPARE(ints("SELECT comic_info_id FROM label_entry WHERE label_id = 1 ORDER BY ordering"), QList<int>({ 1 }));
}

void ComicModelTest::reorderWritesTheEntryOrder()
{
    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    std::unique_ptr<QMimeData> data(model.mimeData({ model.index(1, 0) }));

    QVERIFY(model.dropMimeData(data.get(), Qt::MoveAction, 0, 0, QModelIndex()));
    QCOMPARE(rowIds(model), QList<qulonglong>({ 1, 3 }));
    QCOMPARE(ints("SELECT comic_info_id FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering"), QList<int>({ 1, 3 }));
}

void ComicModelTest::failedReorderShowsTheStoredOrder()
{
    // a placeholder entry is not shown yet, so the view does not have every entry
    QVERIFY(exec("INSERT INTO placeholder (id, title) VALUES (1, 'Expected')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 2, 1)"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(rowIds(model), QList<qulonglong>({ 3, 1 }));
    std::unique_ptr<QMimeData> data(model.mimeData({ model.index(1, 0) }));

    QVERIFY(!model.dropMimeData(data.get(), Qt::MoveAction, 0, 0, QModelIndex()));
    QCOMPARE(rowIds(model), QList<qulonglong>({ 3, 1 }));
    QCOMPARE(ints("SELECT ordering FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY id"), QList<int>({ 0, 1, 2 }));
}

void ComicModelTest::reorderMovesADeletedComic()
{
    QVERIFY(exec("DELETE FROM comic WHERE id = 1"));
    QVERIFY(exec("DELETE FROM comic WHERE id = 5"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(model.rowCount(), 2);
    std::unique_ptr<QMimeData> data(model.mimeData({ model.index(1, 0) }));

    QVERIFY(model.dropMimeData(data.get(), Qt::MoveAction, 0, 0, QModelIndex()));
    QCOMPARE(ints("SELECT comic_info_id FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering"), QList<int>({ 1, 3 }));
    QCOMPARE(model.index(0, 0).data(ComicModel::KindRole).toString(), QString("DeletedComicInList"));
}

void ComicModelTest::deletedComicRowsShowTheirInfoAndLastKnownFile()
{
    QVERIFY(exec("DELETE FROM comic WHERE id = 3"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(model.rowCount(), 2);
    const auto deleted = model.index(0, 0);
    QCOMPARE(deleted.data(ComicModel::KindRole).toString(), QString("DeletedComicInList"));
    QCOMPARE(deleted.data(ComicModel::AvailableRole).toBool(), false);
    QCOMPARE(deleted.data(ComicModel::LastKnownNameRole).toString(), QString("c3.cbz"));
    QVERIFY(deleted.data(ComicModel::ListItemIdRole).isValid());
    QCOMPARE(deleted.data(ComicModel::ListIdRole).toULongLong(), 1ULL);
    QVERIFY(deleted.data(ComicModel::IdRole).isNull());
    QVERIFY(deleted.data(ComicModel::FileNameRole).isNull());
    QCOMPARE(deleted.data(ComicModel::TitleRole).toString(), QString("Title 3"));
    QCOMPARE(deleted.data(ComicModel::ReadableTitle).toString(), QString("#3 Title 3"));
    QCOMPARE(deleted.data(ComicModel::RatingRole).toInt(), 3);
    QCOMPARE(model.index(0, ComicModel::FileName).data().toString(), QString("c3.cbz"));
    QCOMPARE(model.index(0, ComicModel::Path).data().toString(), QString("/Series/c3.cbz"));
    QVERIFY(model.index(0, ComicModel::Id).data().isNull());
    QVERIFY(model.index(0, ComicModel::Parent_Id).data().isNull());
    QCOMPARE(model.index(0, ComicModel::Hash).data().toString(), QString("hash-3"));
    QCOMPARE(model.getComicPath(deleted), QString());
    QVERIFY(!model.getComic(deleted).has_value());
    const auto record = model.record(deleted);
    QVERIFY(std::holds_alternative<ComicRecordCase::DeletedComic>(record));
    QCOMPARE(std::get<ComicRecordCase::DeletedComic>(record).info.id, ComicInfoId(3));
    QCOMPARE(std::get<ComicRecordCase::DeletedComic>(record).lastKnownFile->name, QString("c3.cbz"));
    QVERIFY(std::holds_alternative<ComicRecordCase::Comic>(model.record(model.index(1, 0))));
    QVERIFY(model.getComics({ deleted, model.index(1, 0) }).size() == 1);

    QVERIFY(model.hasUnavailableRows());
    model.setupFolderModelData(2, dataPath);
    QVERIFY(!model.hasUnavailableRows());
}

void ComicModelTest::deletedComicCoverIsTheHashCoverOrTheDefaultImage()
{
    QVERIFY(exec("DELETE FROM comic WHERE id = 3"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(model.index(0, 0).data(ComicModel::CoverPathRole).toUrl(), QUrl("qrc:/images/unavailableCover.png"));
    // an available comic keeps the path of its hash
    QCOMPARE(model.index(1, 0).data(ComicModel::CoverPathRole).toUrl(), model.getCoverUrlPathForComicHash("hash-1"));
    QCOMPARE(model.getUnavailableList(), QList<bool>({ true, false }));
    const auto library = QFileInfo(dataPath).absolutePath();
    QCOMPARE(model.getPaths(library), QStringList({ ":/images/unavailableCover.png", LibraryPaths::coverPath(library, "hash-1") }));

    const auto coverPath = LibraryPaths::coverPathFromLibraryDataPath(dataPath, "hash-3");
    QVERIFY(QDir().mkpath(QFileInfo(coverPath).absolutePath()));
    QFile cover(coverPath);
    QVERIFY(cover.open(QIODevice::WriteOnly));
    cover.write("jpg");
    cover.close();
    QCOMPARE(model.index(0, 0).data(ComicModel::CoverPathRole).toUrl(), model.getCoverUrlPathForComicHash("hash-3"));
    QCOMPARE(model.getPaths(library).first(), LibraryPaths::coverPath(library, "hash-3"));
}

void ComicModelTest::deletedComicsSupportOnlyComicInfoActions()
{
    QVERIFY(exec("DELETE FROM comic WHERE id = 3"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    const auto deleted = model.index(0, 0);
    const auto available = model.index(1, 0);

    for (const auto action : { ComicAction::SaveCover, ComicAction::SetReadState, ComicAction::SetType, ComicAction::EditMetadata, ComicAction::DeleteMetadata, ComicAction::AssignNumbers, ComicAction::SearchComicVine, ComicAction::SetCustomCover, ComicAction::AddToList, ComicAction::RemoveFromList }) {
        QVERIFY(model.supports(deleted, action));
        QVERIFY(model.supports(available, action));
    }
    for (const auto action : { ComicAction::Open, ComicAction::OpenContainingFolder, ComicAction::ChangeFile, ComicAction::AddToFavorites }) {
        QVERIFY(!model.supports(deleted, action));
        QVERIFY(model.supports(available, action));
    }
    QCOMPARE(model.supportedIndexes({ deleted, available }, ComicAction::Open), QModelIndexList({ available }));

    model.setupFolderModelData(2, dataPath);
    QVERIFY(!model.supports(model.index(0, 0), ComicAction::RemoveFromList));
    QVERIFY(model.supports(model.index(0, 0), ComicAction::ChangeFile));
}

void ComicModelTest::comicInfoActionsWriteDeletedComics()
{
    QVERIFY(exec("DELETE FROM comic WHERE id = 3"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    const auto deleted = model.index(0, 0);

    model.setComicsRead({ deleted }, YACReader::Read);
    QCOMPARE(ints("SELECT read FROM comic_info WHERE id = 3"), QList<int>({ 1 }));
    QCOMPARE(model.index(0, 0).data(ComicModel::ReadColumnRole).toBool(), true);

    model.setComicsType({ deleted }, YACReader::FileType::Manga);
    QCOMPARE(ints("SELECT type FROM comic_info WHERE id = 3"), QList<int>({ int(YACReader::FileType::Manga) }));

    model.updateRating(5, deleted);
    QCOMPARE(ints("SELECT rating FROM comic_info WHERE id = 3"), QList<int>({ 5 }));

    model.asignNumbers({ deleted }, 40);
    QCOMPARE(ints("SELECT number FROM comic_info WHERE id = 3"), QList<int>({ 40 }));

    model.deleteMetadata({ deleted });
    QCOMPARE(ints("SELECT COUNT(*) FROM comic_info WHERE id = 3 AND title IS NULL"), QList<int>({ 1 }));
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

void ComicModelTest::loadSource(ComicModel &model, const QString &source)
{
    if (source == "folder") {
        model.setupFolderModelData(2, dataPath);
    } else if (source == "favorites") {
        model.setupFavoritesModelData(dataPath);
    } else if (source == "reading") {
        model.setupReadingModelData(dataPath);
    } else if (source == "recent") {
        model.setupRecentModelData(dataPath);
    } else if (source == "search") {
        // the steps of ComicQueryResultProcessor, without its worker thread
        QList<ComicItem> items;
        QString name;
        {
            auto db = DataBaseManagement::loadDatabase(dataPath);
            name = db.connectionName();
            auto query = comicsSearchQuery(db, "Title");
            items = ComicModel::sortedForFolder(ComicModel::createItems(query));
        }
        QSqlDatabase::removeDatabase(name);
        model.setModelData(new QList<ComicItem>(items), dataPath);
    } else {
        QFAIL(qPrintable("unknown source " + source));
    }
}

void ComicModelTest::sharedViewsShowComicsWithFiles_data()
{
    QTest::addColumn<QString>("source");
    QTest::addColumn<QList<qulonglong>>("comicIds");
    QTest::newRow("folder") << "folder" << QList<qulonglong>({ 1, 2, 3, 5 });
    QTest::newRow("favorites") << "favorites" << QList<qulonglong>({ 1, 2, 5 });
    QTest::newRow("reading") << "reading" << QList<qulonglong>({ 1, 2, 3, 5 });
    QTest::newRow("recent") << "recent" << QList<qulonglong>({ 1, 2, 3, 5 });
    QTest::newRow("search") << "search" << QList<qulonglong>({ 1, 2, 3, 5 });
}

// folders, search, Favorites, Reading, and Recent only have comics with a file, as before the entry tables
void ComicModelTest::sharedViewsShowComicsWithFiles()
{
    QFETCH(QString, source);
    QFETCH(QList<qulonglong>, comicIds);

    // comic_info 4 loses its file but stays, because a list refers to it
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES (1, 2, 4)"));
    QVERIFY(exec("DELETE FROM comic WHERE id = 4"));
    QVERIFY(exec("INSERT INTO comic_default_reading_list (comic_id, default_reading_list_id, ordering) VALUES (1, 1, 0), (2, 1, 1), (5, 1, 2)"));
    QVERIFY(exec(QString("UPDATE comic_info SET added = %1").arg(QDateTime::currentSecsSinceEpoch())));

    ComicModel model;
    loadSource(model, source);
    if (QTest::currentTestFailed())
        return;

    const auto ids = rowIds(model);
    QCOMPARE(QSet<qulonglong>(ids.cbegin(), ids.cend()), QSet<qulonglong>(comicIds.cbegin(), comicIds.cend()));
    QCOMPARE(ids.size(), comicIds.size());

    QVERIFY(!model.hasUnavailableRows());
    QCOMPARE(model.getUnavailableList(), QList<bool>(model.rowCount(), false));

    const auto library = QFileInfo(dataPath).absolutePath();
    const auto paths = model.getPaths(library);
    QModelIndexList all;
    for (int row = 0; row < model.rowCount(); row++) {
        const auto index = model.index(row, 0);
        all.append(index);
        const auto id = ids.at(row);

        QCOMPARE(index.data(ComicModel::KindRole).toString(), QString("Comic"));
        QVERIFY(index.data(ComicModel::AvailableRole).toBool());
        QVERIFY(index.data(ComicModel::ListItemIdRole).isNull());
        QVERIFY(model.index(row, ComicModel::State).data(Qt::DisplayRole).isNull());
        QVERIFY(model.rowId(index) == ComicRowId(ComicId(id)));

        const auto record = model.record(index);
        QVERIFY(recordComic(record) != nullptr);
        QCOMPARE(recordComic(record)->file.id, ComicId(id));

        QCOMPARE(paths.at(row), LibraryPaths::coverPath(library, index.data(ComicModel::HashRole).toString()));

        for (const auto action : { ComicAction::Open, ComicAction::OpenContainingFolder, ComicAction::ChangeFile, ComicAction::SaveCover,
                                   ComicAction::SetReadState, ComicAction::SetType, ComicAction::EditMetadata, ComicAction::DeleteMetadata,
                                   ComicAction::SearchComicVine, ComicAction::AssignNumbers, ComicAction::SetCustomCover,
                                   ComicAction::AddToFavorites, ComicAction::AddToList })
            QVERIFY(model.supports(index, action));
        QVERIFY(!model.supports(index, ComicAction::RemoveFromList));
    }

    std::unique_ptr<QMimeData> data(model.mimeData(all));
    QCOMPARE(YACReader::mimeDataToComicsIds(data.get()), ids);
    QVERIFY(YACReader::mimeDataToListEntries(data.get()).isEmpty());
}

void ComicModelTest::sharedViewAfterAListHasNoDeletedRows_data()
{
    QTest::addColumn<QString>("source");
    QTest::newRow("folder") << "folder";
    QTest::newRow("favorites") << "favorites";
    QTest::newRow("reading") << "reading";
    QTest::newRow("recent") << "recent";
    QTest::newRow("search") << "search";
}

// one model goes from a list with a deleted comic to a shared view and back
void ComicModelTest::sharedViewAfterAListHasNoDeletedRows()
{
    QFETCH(QString, source);

    QVERIFY(exec("DELETE FROM comic WHERE id = 3"));
    QVERIFY(exec("INSERT INTO comic_default_reading_list (comic_id, default_reading_list_id, ordering) VALUES (1, 1, 0)"));
    QVERIFY(exec(QString("UPDATE comic_info SET added = %1").arg(QDateTime::currentSecsSinceEpoch())));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QVERIFY(model.hasUnavailableRows());

    loadSource(model, source);
    if (QTest::currentTestFailed())
        return;
    QVERIFY(model.rowCount() > 0);
    QVERIFY(!model.hasUnavailableRows());
    for (int row = 0; row < model.rowCount(); row++) {
        QCOMPARE(model.index(row, 0).data(ComicModel::KindRole).toString(), QString("Comic"));
        QVERIFY(model.index(row, 0).data(ComicModel::IdRole).toULongLong() != 3);
    }

    model.setupReadingListModelData(1, dataPath);
    QVERIFY(model.hasUnavailableRows());
    QCOMPARE(model.index(0, 0).data(ComicModel::KindRole).toString(), QString("DeletedComicInList"));
}

// Favorites keeps comic ids, so a copy of a file is added and removed on its own
void ComicModelTest::favoritesUseTheFileOfTheRow()
{
    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    const auto copy = model.indexOf(ComicRowId(ComicId(5)));
    QVERIFY(copy.isValid());

    QSignalSpy changed(&model, &ComicModel::favoritesChanged);
    model.addComicsToFavorites(QList<QModelIndex> { copy });
    QCOMPARE(ints("SELECT comic_id FROM comic_default_reading_list WHERE default_reading_list_id = 1"), QList<int>({ 5 }));
    QCOMPARE(changed.size(), 1);
    QCOMPARE(changed.first().first().value<QList<qulonglong>>(), QList<qulonglong>({ 5 }));

    model.setupFavoritesModelData(dataPath);
    QCOMPARE(rowIds(model), QList<qulonglong>({ 5 }));

    model.deleteComicsFromFavorites({ model.index(0, 0) });
    QCOMPARE(ints("SELECT COUNT(*) FROM comic_default_reading_list"), QList<int>({ 0 }));
    QCOMPARE(model.rowCount(), 0);
}

void ComicModelTest::removeFromReadingUsesTheComicOfTheRow()
{
    ComicModel model;
    model.setupReadingModelData(dataPath);
    const auto row = model.indexOf(ComicRowId(ComicId(2)));
    QVERIFY(row.isValid());
    const auto count = model.rowCount();

    model.deleteComicsFromReading({ row });
    QCOMPARE(model.rowCount(), count - 1);
    QVERIFY(!model.indexOf(ComicRowId(ComicId(2))).isValid());
    QCOMPARE(ints("SELECT hasBeenOpened FROM comic_info WHERE id = 2"), QList<int>({ 0 }));
    QCOMPARE(ints("SELECT hasBeenOpened FROM comic_info WHERE id = 3"), QList<int>({ 1 }));
    QCOMPARE(ints("SELECT rating FROM comic_info WHERE id = 2"), QList<int>({ 2 }));
}

QTEST_MAIN(ComicModelTest)

#include "main.moc"
