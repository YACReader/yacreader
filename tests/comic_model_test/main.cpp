#include "comic_db.h"
#include "comic_model.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "search_query.h"
#include "yacreader_comics_selection_helper.h"
#include "yacreader_global.h"
#include "yacreader_global_gui.h"

#include <QAbstractItemModelTester>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
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
    void dragDataCarriesComicsAndEntries();
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
    void placeholderRowsShowTheirMetadata();
    void placeholderCoverIsItsFileOrTheDefaultImage();
    void placeholdersSupportMetadataAndListActions();
    void placeholderCoverChangeUpdatesEveryRow();
    void reorderAndRemoveAPlaceholder();
    void sharedPlaceholderShowsInEveryList();
    void assignNumbersOncePerComicOrPlaceholder();
    void addToAListAppendsWhatEachRowRefersTo();
    void folderShowsItsMissingComicsInNumberOrder();
    void missingComicsInFoldersSupportMetadataAndDelete();
    void missingComicsInFoldersAreDraggedAndAddedToLists();
    void removingMissingComicsFromAFolderKeepsTheComics();
    void rowsAreRemovedInRuns();
    void updatesAreSentForRunsOfRows();
    void selectionSummaryCountsMissingComics();
    void searchResultsKeepEachSeriesTogether();
    void searchLimitKeepsTheFirstSeries();
    void smartListShowsTheMatchingComicsInOrder();
    void smartListReloads();
    void smartListComicsHaveFolderActions();
    void unreadableOrGoneSmartList();

private:
    QList<qulonglong> searchComicInfoIds(const QString &text);
    void insertOrderedComics();
    qulonglong insertSmartList(const QString &name, const QString &query);
    static QList<qulonglong> comicInfoIds(ComicModel &model);
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

void ComicModelTest::dragDataCarriesComicsAndEntries()
{
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, comic_info_id) VALUES (30, 1, 2, 3)"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    std::unique_ptr<QMimeData> data(model.mimeData({ model.index(2, 0), model.index(1, 0) }));
    QCOMPARE(YACReader::mimeDataToComicsIds(data.get()), QList<qulonglong>({ 3, 1 }));
    QVERIFY(YACReader::mimeDataToRows(data.get()) == QList<ComicRowId>({ model.rowId(model.index(2, 0)), model.rowId(model.index(1, 0)) }));
    QVERIFY(YACReader::mimeDataToRows(data.get()).first() == ComicRowId(ListItemId { ListKind::ReadingList, 30 }));

    model.setupFolderModelData(2, dataPath);
    data.reset(model.mimeData({ model.index(0, 0) }));
    QVERIFY(YACReader::mimeDataToRows(data.get()) == QList<ComicRowId>({ ComicId(model.index(0, 0).data(ComicModel::IdRole).toULongLong()) }));
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
    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(rowIds(model), QList<qulonglong>({ 3, 1 }));

    // an entry added after the view loaded: the view does not have every entry
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES (1, 2, 2)"));
    std::unique_ptr<QMimeData> data(model.mimeData({ model.index(1, 0) }));

    QVERIFY(!model.dropMimeData(data.get(), Qt::MoveAction, 0, 0, QModelIndex()));
    QCOMPARE(rowIds(model), QList<qulonglong>({ 3, 1, 2 }));
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
    // the file is checked once, until a cover or the rows change
    QCOMPARE(model.index(0, 0).data(ComicModel::CoverPathRole).toUrl(), QUrl("qrc:/images/unavailableCover.png"));
    QCOMPARE(model.getPaths(library).first(), ":/images/unavailableCover.png");

    model.notifyCoverChange({ ComicInfoId(3) });
    auto hashCover = model.getCoverUrlPathForComicHash("hash-3");
    hashCover.setQuery("revision=1");
    QCOMPARE(model.index(0, 0).data(ComicModel::CoverPathRole).toUrl(), hashCover);
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
    QVERIFY(model.supports(deleted, ComicAction::MatchToLibraryComic));
    QVERIFY(!model.supports(available, ComicAction::MatchToLibraryComic));

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
    // the two rows are consecutive, so one signal covers them
    QCOMPARE(changed.count(), 1);
    QCOMPARE(changed.at(0).at(0).toModelIndex().row(), rows.first());
    QCOMPARE(changed.at(0).at(1).toModelIndex().row(), rows.last());
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
            items = ComicModel::sortedForSearch(ComicModel::createItems(query));
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
        QCOMPARE(model.supports(index, ComicAction::RemoveFromList), source == "favorites" || source == "reading");
        QVERIFY(!model.supports(index, ComicAction::MatchToLibraryComic));
    }

    std::unique_ptr<QMimeData> data(model.mimeData(all));
    QCOMPARE(YACReader::mimeDataToComicsIds(data.get()), ids);
    QList<ComicRowId> rows;
    for (const auto id : ids)
        rows.append(ComicId(id));
    QVERIFY(YACReader::mimeDataToRows(data.get()) == rows);
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

void ComicModelTest::placeholderRowsShowTheirMetadata()
{
    QVERIFY(exec("INSERT INTO placeholder (id, series, number, volume, date) VALUES (1, 'Expected Series', '7', '2020', '01/02/2020')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, placeholder_id) VALUES (40, 1, 2, 1)"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(model.rowCount(), 3);
    const auto index = model.index(2, 0);

    QCOMPARE(index.data(ComicModel::KindRole).toString(), QString("PlaceholderInList"));
    QCOMPARE(index.data(ComicModel::AvailableRole).toBool(), false);
    QCOMPARE(index.data(ComicModel::ListItemIdRole).toULongLong(), 40ULL);
    QCOMPARE(index.data(ComicModel::ListIdRole).toULongLong(), 1ULL);
    QVERIFY(model.rowId(index) == ComicRowId(ListItemId { ListKind::ReadingList, 40 }));

    // no file, comic id, or comic state
    for (const int role : std::initializer_list<int> { ComicModel::IdRole, ComicModel::FileNameRole, ComicModel::LastKnownNameRole, ComicModel::HashRole, ComicModel::SizeRole, ComicModel::NumPagesRole })
        QVERIFY2(index.data(role).isNull(), qPrintable(QString::number(role)));
    for (const int column : std::initializer_list<int> { ComicModel::Id, ComicModel::Path, ComicModel::ReadColumn, ComicModel::Hash })
        QVERIFY2(model.index(2, column).data(Qt::DisplayRole).isNull(), qPrintable(QString::number(column)));
    // the table shows "-" for the values a placeholder does not have
    for (const int column : std::initializer_list<int> { ComicModel::FileName, ComicModel::NumPages, ComicModel::CurrentPage, ComicModel::Size })
        QCOMPARE(model.index(2, column).data(Qt::DisplayRole).toString(), QString("-"));
    QVERIFY(!(model.flags(model.index(2, ComicModel::Rating)) & Qt::ItemIsEditable));

    // the descriptive metadata, and "Series #Number" when there is no title, also in the table
    QCOMPARE(index.data(ComicModel::NumberRole).toString(), QString("7"));
    QCOMPARE(index.data(ComicModel::SeriesRole).toString(), QString("Expected Series"));
    QCOMPARE(index.data(ComicModel::VolumeRole).toString(), QString("2020"));
    QCOMPARE(index.data(ComicModel::TitleRole).toString(), QString("Expected Series #7"));
    QCOMPARE(index.data(ComicModel::ReadableTitle).toString(), QString("Expected Series #7"));
    QCOMPARE(model.index(2, ComicModel::Title).data(Qt::DisplayRole).toString(), QString("Expected Series #7"));

    QVERIFY(model.hasUnavailableRows());
    QCOMPARE(model.getUnavailableList(), QList<bool>({ false, false, true }));

    const auto record = model.record(index);
    QVERIFY(recordInfo(record) == nullptr);
    QVERIFY(recordComic(record) == nullptr);
    QCOMPARE(recordPlaceholder(record)->id, PlaceholderId(1));
    QCOMPARE(recordMetadata(record).series.toString(), QString("Expected Series"));
}

void ComicModelTest::placeholderCoverIsItsFileOrTheDefaultImage()
{
    QVERIFY(exec("INSERT INTO placeholder (id, title, coverKey) VALUES (1, 'With a cover', 'cover-1'), (2, 'Without a cover', NULL)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 2, 1), (1, 3, 2)"));
    const auto library = QFileInfo(dataPath).absolutePath();
    const auto coverPath = LibraryPaths::placeholderCoverPathFromDataPath(dataPath, "cover-1");

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    const QUrl defaultCover("qrc:/images/unavailableCover.png");
    QCOMPARE(model.index(2, 0).data(ComicModel::CoverPathRole).toUrl(), defaultCover);
    QCOMPARE(model.index(3, 0).data(ComicModel::CoverPathRole).toUrl(), defaultCover);
    QCOMPARE(model.getPaths(library).mid(2), QStringList({ ":/images/unavailableCover.png", ":/images/unavailableCover.png" }));

    QVERIFY(QDir().mkpath(QFileInfo(coverPath).absolutePath()));
    QImage cover(10, 15, QImage::Format_RGB32);
    cover.fill(Qt::red);
    QVERIFY(cover.save(coverPath));
    // the file is checked once, until a cover or the rows change
    QCOMPARE(model.index(2, 0).data(ComicModel::CoverPathRole).toUrl(), defaultCover);

    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(model.index(2, 0).data(ComicModel::CoverPathRole).toUrl(), QUrl::fromLocalFile(coverPath));
    QCOMPARE(model.getPaths(library).at(2), coverPath);
    QCOMPARE(model.index(3, 0).data(ComicModel::CoverPathRole).toUrl(), defaultCover);
}

void ComicModelTest::placeholdersSupportMetadataAndListActions()
{
    QVERIFY(exec("INSERT INTO placeholder (id, title, coverKey) VALUES (1, 'With a cover', 'cover-1'), (2, 'Without a cover', NULL)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 2, 1), (1, 3, 2)"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    const auto withCover = model.index(2, 0);
    const auto withoutCover = model.index(3, 0);

    for (const auto action : { ComicAction::EditMetadata, ComicAction::SearchComicVine, ComicAction::SetCustomCover, ComicAction::AssignNumbers, ComicAction::AddToList, ComicAction::RemoveFromList,
                               ComicAction::MatchToLibraryComic }) {
        QVERIFY(model.supports(withCover, action));
        QVERIFY(model.supports(withoutCover, action));
    }
    QVERIFY(model.supports(withCover, ComicAction::SaveCover));
    QVERIFY(!model.supports(withoutCover, ComicAction::SaveCover));
    for (const auto action : { ComicAction::Open, ComicAction::OpenContainingFolder, ComicAction::ChangeFile, ComicAction::SetReadState, ComicAction::SetType,
                               ComicAction::DeleteMetadata, ComicAction::AddToFavorites })
        QVERIFY(!model.supports(withCover, action));
}

void ComicModelTest::placeholderCoverChangeUpdatesEveryRow()
{
    QVERIFY(exec("INSERT INTO placeholder (id, title) VALUES (1, 'Shared'), (2, 'Other')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 2, 1), (1, 3, 2), (1, 4, 1)"));
    const auto coverPath = LibraryPaths::placeholderCoverPathFromDataPath(dataPath, "new-key");
    QVERIFY(QDir().mkpath(QFileInfo(coverPath).absolutePath()));
    QImage cover(10, 15, QImage::Format_RGB32);
    cover.fill(Qt::red);
    QVERIFY(cover.save(coverPath));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);

    model.notifyPlaceholderCoverChange({ { PlaceholderId(1), QString("new-key") } });
    QCOMPARE(changed.count(), 2);
    QCOMPARE(model.index(2, 0).data(ComicModel::CoverPathRole).toUrl(), QUrl::fromLocalFile(coverPath));
    QCOMPARE(model.index(4, 0).data(ComicModel::CoverPathRole).toUrl(), QUrl::fromLocalFile(coverPath));
    QCOMPARE(model.index(3, 0).data(ComicModel::CoverPathRole).toUrl(), QUrl("qrc:/images/unavailableCover.png"));
    QVERIFY(model.supports(model.index(2, 0), ComicAction::SaveCover));

    model.notifyPlaceholderCoverChange({ { PlaceholderId(1), std::nullopt } });
    QCOMPARE(model.index(2, 0).data(ComicModel::CoverPathRole).toUrl(), QUrl("qrc:/images/unavailableCover.png"));
    QVERIFY(!model.supports(model.index(2, 0), ComicAction::SaveCover));
}

void ComicModelTest::reorderAndRemoveAPlaceholder()
{
    QVERIFY(exec("INSERT INTO placeholder (id, title, coverKey) VALUES (1, 'Expected', 'cover-1')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, placeholder_id) VALUES (40, 1, 2, 1)"));
    const auto coverPath = LibraryPaths::placeholderCoverPathFromDataPath(dataPath, "cover-1");
    QVERIFY(QDir().mkpath(QFileInfo(coverPath).absolutePath()));
    QImage cover(10, 15, QImage::Format_RGB32);
    cover.fill(Qt::red);
    QVERIFY(cover.save(coverPath));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    std::unique_ptr<QMimeData> data(model.mimeData({ model.index(2, 0) }));
    QVERIFY(model.dropMimeData(data.get(), Qt::MoveAction, 0, 0, QModelIndex()));
    QCOMPARE(model.index(0, 0).data(ComicModel::KindRole).toString(), QString("PlaceholderInList"));
    QCOMPARE(ints("SELECT COALESCE(comic_info_id, 0) FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering"), QList<int>({ 0, 3, 1 }));

    model.deleteComicsFromLists({ model.index(0, 0) });
    QCOMPARE(rowIds(model), QList<qulonglong>({ 3, 1 }));
    QCOMPARE(ints("SELECT COUNT(*) FROM placeholder"), QList<int>({ 0 }));
    QVERIFY(!QFile::exists(coverPath));
}

void ComicModelTest::sharedPlaceholderShowsInEveryList()
{
    QVERIFY(exec("INSERT INTO placeholder (id, title) VALUES (1, 'Shared')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 2, 1)"));
    QVERIFY(exec("INSERT INTO label_entry (label_id, ordering, placeholder_id) VALUES (1, 0, 1)"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(model.index(2, 0).data(ComicModel::TitleRole).toString(), QString("Shared"));
    model.setupLabelModelData(1, dataPath);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).data(ComicModel::KindRole).toString(), QString("PlaceholderInList"));
    QCOMPARE(model.index(0, 0).data(ComicModel::TitleRole).toString(), QString("Shared"));
}

// each comic_info or placeholder gets one number, at its first appearance
void ComicModelTest::assignNumbersOncePerComicOrPlaceholder()
{
    QVERIFY(exec("INSERT INTO placeholder (id, title) VALUES (1, 'Expected')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 2, 1)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES (1, 3, 3)"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(model.rowCount(), 4);
    model.asignNumbers({ model.index(0, 0), model.index(1, 0), model.index(2, 0), model.index(3, 0) }, 10);

    QCOMPARE(ints("SELECT number FROM comic_info WHERE id IN (3, 1) ORDER BY id DESC"), QList<int>({ 10, 11 }));
    QCOMPARE(ints("SELECT number FROM placeholder WHERE id = 1"), QList<int>({ 12 }));
    QCOMPARE(ints("SELECT edited FROM comic_info WHERE id IN (3, 1)"), QList<int>({ 1, 1 }));
}

// in order, also placeholders and deleted comics; labels skip repeats
void ComicModelTest::addToAListAppendsWhatEachRowRefersTo()
{
    QVERIFY(exec("INSERT INTO placeholder (id, title) VALUES (1, 'Expected')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 2, 1)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES (1, 3, 4)"));
    QVERIFY(exec("DELETE FROM comic WHERE id = 4"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (2, 'Other')"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    QCOMPARE(model.rowCount(), 4);
    const QList<QModelIndex> rows = { model.index(2, 0), model.index(0, 0), model.index(3, 0), model.index(1, 0) };

    model.addComicsToLabel(rows, 1);
    model.addComicsToLabel(rows, 1);
    QCOMPARE(ints("SELECT COALESCE(comic_info_id, 0) FROM label_entry WHERE label_id = 1 ORDER BY ordering"), QList<int>({ 0, 3, 4, 1 }));
    QCOMPARE(ints("SELECT COALESCE(placeholder_id, 0) FROM label_entry WHERE label_id = 1 ORDER BY ordering"), QList<int>({ 1, 0, 0, 0 }));

    model.addComicsToReadingList(rows, 2);
    model.addComicsToReadingList(rows, 2);
    QCOMPARE(ints("SELECT COALESCE(comic_info_id, 0) FROM reading_list_entry WHERE reading_list_id = 2 ORDER BY ordering"), QList<int>({ 0, 3, 4, 1, 0, 3, 4, 1 }));

    // a folder row is its comic, which adds its comic_info
    model.setupFolderModelData(2, dataPath);
    model.addComicsToReadingList({ model.indexOf(ComicId(5)) }, 2);
    QCOMPARE(ints("SELECT comic_info_id FROM reading_list_entry WHERE reading_list_id = 2 ORDER BY ordering DESC LIMIT 1"), QList<int>({ 1 }));
}

void ComicModelTest::folderShowsItsMissingComicsInNumberOrder()
{
    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (3, 1, 'Other', '/Other')"));
    QVERIFY(exec("INSERT INTO placeholder (id, series, number) VALUES (1, 'Series', '6'), (2, 'Series', '0'), (3, 'Other', '9')"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 1), (2, 2), (3, 3)"));

    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    QCOMPARE(model.rowCount(), 7);
    QStringList kinds;
    QStringList numbers;
    for (int row = 0; row < model.rowCount(); row++) {
        kinds.append(model.index(row, 0).data(ComicModel::KindRole).toString());
        numbers.append(model.index(row, 0).data(ComicModel::NumberRole).toString());
    }
    QCOMPARE(kinds, QStringList({ "PlaceholderInFolder", "Comic", "Comic", "Comic", "Comic", "Comic", "PlaceholderInFolder" }));
    QCOMPARE(numbers, QStringList({ "0", "1", "1", "2", "3", "4", "6" }));

    const auto missing = model.index(6, 0);
    QCOMPARE(missing.data(ComicModel::AvailableRole).toBool(), false);
    QCOMPARE(missing.data(ComicModel::TitleRole).toString(), QString("Series #6"));
    QVERIFY(missing.data(ComicModel::IdRole).isNull());
    QVERIFY(missing.data(ComicModel::ListItemIdRole).isNull());
    QCOMPARE(model.index(6, ComicModel::FileName).data(Qt::DisplayRole).toString(), QString("-"));
    QVERIFY(model.rowId(missing) == ComicRowId(PlaceholderId(1)));
    QCOMPARE(model.indexOf(PlaceholderId(2)).row(), 0);
    QCOMPARE(recordPlaceholder(model.record(missing))->id, PlaceholderId(1));
    QVERIFY(!model.listEntry(missing));
    QCOMPARE(model.getUnavailableList(), QList<bool>({ true, false, false, false, false, false, true }));
}

void ComicModelTest::missingComicsInFoldersSupportMetadataAndDelete()
{
    QVERIFY(exec("INSERT INTO placeholder (id, number, coverKey) VALUES (1, '6', 'cover-1'), (2, '7', NULL)"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 1), (2, 2)"));

    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    const auto withCover = model.indexOf(PlaceholderId(1));
    const auto withoutCover = model.indexOf(PlaceholderId(2));

    for (const auto action : { ComicAction::EditMetadata, ComicAction::SearchComicVine, ComicAction::SetCustomCover, ComicAction::AssignNumbers, ComicAction::AddToList, ComicAction::RemoveFromFolder,
                               ComicAction::MatchToLibraryComic }) {
        QVERIFY(model.supports(withCover, action));
        QVERIFY(model.supports(withoutCover, action));
    }
    QVERIFY(model.supports(withCover, ComicAction::SaveCover));
    QVERIFY(!model.supports(withoutCover, ComicAction::SaveCover));
    for (const auto action : { ComicAction::Open, ComicAction::OpenContainingFolder, ComicAction::ChangeFile, ComicAction::SetReadState, ComicAction::SetType,
                               ComicAction::DeleteMetadata, ComicAction::AddToFavorites, ComicAction::RemoveFromList })
        QVERIFY(!model.supports(withCover, action));
    QVERIFY(!model.supports(model.indexOf(ComicId(1)), ComicAction::RemoveFromFolder));
}

void ComicModelTest::missingComicsInFoldersAreDraggedAndAddedToLists()
{
    QVERIFY(exec("INSERT INTO placeholder (id, number) VALUES (1, '6')"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 1)"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (2, 'Other')"));

    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    const QList<QModelIndex> rows = { model.indexOf(PlaceholderId(1)), model.indexOf(ComicId(2)) };
    std::unique_ptr<QMimeData> data(model.mimeData(rows));
    QCOMPARE(YACReader::mimeDataToComicsIds(data.get()), QList<qulonglong>({ 2 }));
    QVERIFY(YACReader::mimeDataToRows(data.get()) == QList<ComicRowId>({ PlaceholderId(1), ComicId(2) }));

    model.addComicsToReadingList(rows, 2);
    model.addComicsToLabel(rows, 1);
    QCOMPARE(ints("SELECT COALESCE(placeholder_id, 0) FROM reading_list_entry WHERE reading_list_id = 2 ORDER BY ordering"), QList<int>({ 1, 0 }));
    QCOMPARE(ints("SELECT COALESCE(comic_info_id, 0) FROM reading_list_entry WHERE reading_list_id = 2 ORDER BY ordering"), QList<int>({ 0, 2 }));
    QCOMPARE(ints("SELECT COALESCE(placeholder_id, 0) FROM label_entry WHERE label_id = 1 ORDER BY ordering"), QList<int>({ 1, 0 }));
}

void ComicModelTest::removingMissingComicsFromAFolderKeepsTheComics()
{
    const auto coverPath = LibraryPaths::placeholderCoverPathFromDataPath(dataPath, "cover-1");
    QVERIFY(QDir().mkpath(QFileInfo(coverPath).absolutePath()));
    QImage cover(10, 15, QImage::Format_RGB32);
    cover.fill(Qt::red);
    QVERIFY(cover.save(coverPath));
    QVERIFY(exec("INSERT INTO placeholder (id, number, coverKey) VALUES (1, '6', 'cover-1'), (2, '7', NULL)"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 1), (2, 2)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 2, 2)"));

    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    QCOMPARE(model.rowCount(), 7);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QSignalSpy removed(&model, &ComicModel::rowsRemoved);
    model.removeMissingComicsFromFolder({ model.indexOf(PlaceholderId(2)), model.indexOf(ComicId(1)), model.indexOf(PlaceholderId(1)) });

    QCOMPARE(model.rowCount(), 5);
    // the two missing comics are the last rows, so they go in one removal
    QCOMPARE(removed.count(), 1);
    QCOMPARE(removed.at(0).at(1).toInt(), 5);
    QCOMPARE(removed.at(0).at(2).toInt(), 6);
    QVERIFY(!model.indexOf(PlaceholderId(1)).isValid());
    QVERIFY(model.indexOf(ComicId(1)).isValid());
    QCOMPARE(ints("SELECT COUNT(*) FROM folder_placeholder"), QList<int>({ 0 }));
    // the reading list still uses placeholder 2
    QCOMPARE(ints("SELECT id FROM placeholder"), QList<int>({ 2 }));
    QVERIFY(!QFile::exists(coverPath));
    QCOMPARE(ints("SELECT COUNT(*) FROM comic WHERE parentId = 2"), QList<int>({ 5 }));
}

// deleted comics and placeholders are both missing; a placeholder has no reading state or pages
void ComicModelTest::selectionSummaryCountsMissingComics()
{
    QVERIFY(exec("UPDATE comic_info SET read = 1 WHERE id = 3"));
    QVERIFY(exec("INSERT INTO placeholder (id, title) VALUES (1, 'Expected')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 2, 1)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES (1, 3, 4)"));
    QVERIFY(exec("DELETE FROM comic WHERE id = 4"));

    ComicModel model;
    model.setupReadingListModelData(1, dataPath);
    YACReaderComicsSelectionHelper selection;
    selection.setModel(&model);
    selection.selectAll();

    const auto info = selection.selectionInfo();
    QCOMPARE(info.value("count").toInt(), 4);
    QCOMPARE(info.value("readCount").toInt(), 1);
    QCOMPARE(info.value("inProgressCount").toInt(), 2);
    QCOMPARE(info.value("unreadCount").toInt(), 0);
    QCOMPARE(info.value("pageCount").toInt(), 60);
    QCOMPARE(info.value("unknownPageCount").toInt(), 0);
    QCOMPARE(info.value("missingCount").toInt(), 2);
}

QList<qulonglong> ComicModelTest::searchComicInfoIds(const QString &text)
{
    QList<ComicItem> items;
    QString name;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        name = db.connectionName();
        auto query = comicsSearchQuery(db, text);
        items = ComicModel::sortedForSearch(ComicModel::createItems(query));
    }
    QSqlDatabase::removeDatabase(name);

    ComicModel model;
    model.setModelData(new QList<ComicItem>(items), dataPath);
    QList<qulonglong> ids;
    for (int row = 0; row < model.rowCount(); ++row)
        ids.append(recordInfo(model.record(model.index(row, 0)))->id.value());
    return ids;
}

void ComicModelTest::insertOrderedComics()
{
    QVERIFY(exec("INSERT INTO comic_info (id, hash, title, series, volume, number) VALUES "
                 "(10, 'h10', 'Ordered', 'Robin', NULL, '2'), "
                 "(11, 'h11', 'Ordered', 'Batman', '2011', '10'), "
                 "(12, 'h12', 'Ordered', 'Batman', '2011', '2'), "
                 "(13, 'h13', 'Ordered', 'batman', '2016', '1'), "
                 "(14, 'h14', 'Ordered', 'Batman', NULL, '5'), "
                 "(15, 'h15', 'Ordered', NULL, NULL, '1'), "
                 "(16, 'h16', 'Ordered', 'Batman', '2011', NULL)"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES "
                 "(10, 2, 10, 'o10.cbz', '/Series/o10.cbz'), (11, 2, 11, 'o11.cbz', '/Series/o11.cbz'), (12, 2, 12, 'o12.cbz', '/Series/o12.cbz'), "
                 "(13, 2, 13, 'o13.cbz', '/Series/o13.cbz'), (14, 2, 14, 'o14.cbz', '/Series/o14.cbz'), (15, 2, 15, 'o15.cbz', '/Series/o15.cbz'), "
                 "(16, 2, 16, 'o16.cbz', '/Series/o16.cbz')"));
}

void ComicModelTest::searchResultsKeepEachSeriesTogether()
{
    insertOrderedComics();
    QCOMPARE(searchComicInfoIds("Ordered"), QList<qulonglong>({ 14, 12, 11, 16, 13, 10, 15 }));
}

qulonglong ComicModelTest::insertSmartList(const QString &name, const QString &query)
{
    std::optional<SmartListId> id;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        connectionName = db.connectionName();
        id = DBHelper::insertSmartList(name, query, db);
    }
    QSqlDatabase::removeDatabase(connectionName);
    return id ? id->value() : 0;
}

QList<qulonglong> ComicModelTest::comicInfoIds(ComicModel &model)
{
    QList<qulonglong> ids;
    for (int row = 0; row < model.rowCount(); ++row)
        ids.append(recordInfo(model.record(model.index(row, 0)))->id.value());
    return ids;
}

void ComicModelTest::smartListShowsTheMatchingComicsInOrder()
{
    insertOrderedComics();
    const auto id = insertSmartList("Ordered", "Ordered");
    QVERIFY(id != 0);

    ComicModel model;
    model.setupSmartListModelData(id, dataPath);
    QCOMPARE(model.getMode(), ComicModel::SmartList);
    QCOMPARE(model.getSourceId(), id);
    QCOMPARE(comicInfoIds(model), QList<qulonglong>({ 14, 12, 11, 16, 13, 10, 15 }));
}

void ComicModelTest::smartListReloads()
{
    const auto id = insertSmartList("Batman", "series:batman");
    ComicModel model;
    model.setupSmartListModelData(id, dataPath);
    QCOMPARE(model.rowCount(), 0);

    insertOrderedComics();
    model.reload();
    QCOMPARE(comicInfoIds(model), QList<qulonglong>({ 14, 12, 11, 16, 13 }));

    QVERIFY(exec(QString("UPDATE smart_list SET query = 'series:robin' WHERE id = %1").arg(id)));
    model.reload();
    QCOMPARE(comicInfoIds(model), QList<qulonglong>({ 10 }));
}

void ComicModelTest::smartListComicsHaveFolderActions()
{
    insertOrderedComics();
    ComicModel model;
    model.setupSmartListModelData(insertSmartList("Ordered", "Ordered"), dataPath);
    const auto index = model.index(0, 0);

    for (const auto action : { ComicAction::Open, ComicAction::OpenContainingFolder, ComicAction::ChangeFile, ComicAction::SetReadState,
                               ComicAction::EditMetadata, ComicAction::AddToFavorites, ComicAction::AddToList })
        QVERIFY(model.supports(index, action));
    QVERIFY(!model.supports(index, ComicAction::RemoveFromList));
    QVERIFY(!model.supports(index, ComicAction::MatchToLibraryComic));

    std::unique_ptr<QMimeData> data(model.mimeData({ index }));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::MoveAction, 0, 0, QModelIndex()));
}

void ComicModelTest::unreadableOrGoneSmartList()
{
    insertOrderedComics();
    ComicModel model;
    model.setupSmartListModelData(insertSmartList("Broken", "series:(batman"), dataPath);
    QCOMPARE(model.rowCount(), 0);

    model.setupSmartListModelData(999, dataPath);
    QCOMPARE(model.rowCount(), 0);
}

void ComicModelTest::searchLimitKeepsTheFirstSeries()
{
    // the scan order puts "Alpha" last, after more than the limit of "Zeta" comics
    QVERIFY(exec("WITH RECURSIVE n(x) AS (SELECT 1 UNION ALL SELECT x + 1 FROM n WHERE x < 510) "
                 "INSERT INTO comic_info (id, hash, title, series, number) SELECT 1000 + x, 'bulk-' || x, 'Bulk', 'Zeta', x FROM n"));
    QVERIFY(exec("WITH RECURSIVE n(x) AS (SELECT 1 UNION ALL SELECT x + 1 FROM n WHERE x < 510) "
                 "INSERT INTO comic (id, parentId, comicInfoId, fileName, path) SELECT 1000 + x, 2, 1000 + x, 'z' || x || '.cbz', '/Series/z' || x || '.cbz' FROM n"));
    QVERIFY(exec("INSERT INTO comic_info (id, hash, title, series, number) VALUES (2000, 'alpha', 'Bulk', 'Alpha', '1')"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (2000, 2, 2000, 'alpha.cbz', '/Series/alpha.cbz')"));

    const auto ids = searchComicInfoIds("Bulk");
    QCOMPARE(ids.size(), 500);
    QCOMPARE(ids.constFirst(), qulonglong(2000));
    QCOMPARE(ids.at(1), qulonglong(1001));
    QCOMPARE(ids.at(2), qulonglong(1002));
    QCOMPARE(ids.constLast(), qulonglong(1499));
}

void ComicModelTest::rowsAreRemovedInRuns()
{
    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    QCOMPARE(rowIds(model), QList<qulonglong>({ 1, 5, 2, 3, 4 }));
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QSignalSpy removed(&model, &ComicModel::rowsRemoved);

    // in selection order, with a repeated row
    model.deleteComicsFromModel({ model.index(3, 0), model.index(0, 0), model.index(2, 0), model.index(3, 0) });

    QCOMPARE(rowIds(model), QList<qulonglong>({ 5, 4 }));
    QCOMPARE(removed.count(), 2);
    QCOMPARE(removed.at(0).at(1).toInt(), 2);
    QCOMPARE(removed.at(0).at(2).toInt(), 3);
    QCOMPARE(removed.at(1).at(1).toInt(), 0);
    QCOMPARE(removed.at(1).at(2).toInt(), 0);
}

void ComicModelTest::updatesAreSentForRunsOfRows()
{
    ComicModel model;
    model.setupFolderModelData(2, dataPath);
    // comic_info of each row: 1, 1, 2, 3, 4
    QCOMPARE(rowIds(model), QList<qulonglong>({ 1, 5, 2, 3, 4 }));
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QSignalSpy changed(&model, &ComicModel::dataChanged);
    const auto rowsOf = [&changed](int signal) {
        return std::pair { changed.at(signal).at(0).toModelIndex().row(), changed.at(signal).at(1).toModelIndex().row() };
    };

    model.setComicsRead({ model.index(4, 0), model.index(0, 0), model.index(3, 0) }, YACReader::Read);
    QCOMPARE(changed.count(), 2);
    QCOMPARE(rowsOf(0), std::pair(0, 1));
    QCOMPARE(rowsOf(1), std::pair(3, 4));
    for (const auto row : { 0, 1, 3, 4 })
        QCOMPARE(model.data(model.index(row, 0), ComicModel::ReadColumnRole).toBool(), true);
    QCOMPARE(model.data(model.index(2, 0), ComicModel::ReadColumnRole).toBool(), false);

    changed.clear();
    model.notifyCoverChange({ ComicInfoId(2), ComicInfoId(3) });
    QCOMPARE(changed.count(), 1);
    QCOMPARE(rowsOf(0), std::pair(2, 3));
    QCOMPARE(changed.at(0).at(2).value<QList<int>>(), QList<int>({ ComicModel::CoverPathRole }));
}

QTEST_MAIN(ComicModelTest)

#include "main.moc"
