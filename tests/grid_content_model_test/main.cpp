#include "comic_model.h"
#include "data_base_management.h"
#include "folder_model.h"
#include "grid_content_model.h"
#include "yacreader_global.h"

#include <QAbstractItemModelTester>
#include <QApplication>
#include <QDir>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

using namespace YACReader;

class GridContentModelTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void comicRowsForwardTheComicRoles();
    void folderRowsHaveOnlyTheirTitle();
    void roleNames();
    void listCardsReplaceTheOtherRows();
    void removedRowsUpdateTheSourceIndexOfTheNextRows();

private:
    std::unique_ptr<QTemporaryDir> root;
    QString dataPath;

    bool exec(const QString &sql);
};

// grid role -> comic model role
static const QList<QPair<int, int>> forwardedRoles = {
    { GridContentModel::TitleRole, ComicModel::TitleRole },
    { GridContentModel::IdRole, ComicModel::IdRole },
    { GridContentModel::ReadableTitleRole, ComicModel::ReadableTitle },
    { GridContentModel::KindRole, ComicModel::KindRole },
    { GridContentModel::AvailableRole, ComicModel::AvailableRole },
    { GridContentModel::LastKnownNameRole, ComicModel::LastKnownNameRole },
    { GridContentModel::ListItemIdRole, ComicModel::ListItemIdRole },
    { GridContentModel::ListIdRole, ComicModel::ListIdRole },
};

void GridContentModelTest::init()
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

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path, numChildren) VALUES (2, 1, 'Series', '/Series', 3), (3, 2, 'Child', '/Series/Child', 0)"));
    for (int i = 1; i <= 2; i++) {
        QVERIFY(exec(QString("INSERT INTO comic_info (id, hash, title, number, numPages) VALUES (%1, 'hash-%1', 'Title %1', '%1', 20)").arg(i)));
        QVERIFY(exec(QString("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (%1, 2, %1, 'c%1.cbz', '/Series/c%1.cbz')").arg(i)));
    }
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'List')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, comic_info_id) VALUES (7, 1, 0, 2), (8, 1, 1, 1)"));
}

void GridContentModelTest::cleanup()
{
    root.reset();
}

bool GridContentModelTest::exec(const QString &sql)
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

void GridContentModelTest::comicRowsForwardTheComicRoles()
{
    ComicModel comics;
    GridContentModel grid;
    grid.setComicModel(&comics);

    for (const bool list : { false, true }) {
        if (list)
            comics.setupReadingListModelData(1, dataPath);
        else
            comics.setupFolderModelData(2, dataPath);

        QCOMPARE(grid.rowCount(), 2);
        for (int row = 0; row < grid.rowCount(); row++) {
            const auto viewIndex = grid.index(row, 0);
            const auto sourceIndex = comics.index(grid.sourceComicRow(row), 0);
            QCOMPARE(viewIndex.data(GridContentModel::ItemKindRole).toInt(), int(GridContentModel::ComicItem));
            for (const auto &[gridRole, comicRole] : forwardedRoles)
                QCOMPARE(viewIndex.data(gridRole), sourceIndex.data(comicRole));
        }
    }

    QCOMPARE(grid.index(0, 0).data(GridContentModel::KindRole).toString(), QString("ComicInList"));
    QCOMPARE(grid.index(0, 0).data(GridContentModel::ListItemIdRole).toULongLong(), 7ULL);
    QCOMPARE(grid.index(1, 0).data(GridContentModel::ListIdRole).toULongLong(), 1ULL);
    QCOMPARE(grid.index(0, 0).data(GridContentModel::ReadableTitleRole).toString(), QString("#2 - Title 2"));
}

void GridContentModelTest::folderRowsHaveOnlyTheirTitle()
{
    FolderModel folders;
    folders.setupModelData(dataPath);
    ComicModel comics;
    comics.setupFolderModelData(2, dataPath);

    GridContentModel grid;
    grid.setComicModel(&comics);
    grid.setFolderModel(&folders, folders.index(qulonglong(2)));

    QCOMPARE(grid.rowCount(), 3);
    const auto folderRow = grid.index(0, 0);
    QCOMPARE(folderRow.data(GridContentModel::ItemKindRole).toInt(), int(GridContentModel::FolderItem));
    QCOMPARE(folderRow.data(GridContentModel::ReadableTitleRole).toString(), QString("Child"));
    for (const auto role : { GridContentModel::KindRole, GridContentModel::AvailableRole, GridContentModel::LastKnownNameRole, GridContentModel::ListItemIdRole, GridContentModel::ListIdRole })
        QVERIFY(!folderRow.data(role).isValid());
    QCOMPARE(grid.index(1, 0).data(GridContentModel::KindRole).toString(), QString("Comic"));
}

void GridContentModelTest::roleNames()
{
    GridContentModel grid;
    const auto names = grid.roleNames();
    QCOMPARE(names.value(GridContentModel::ReadableTitleRole), QByteArray("readable_title"));
    QCOMPARE(names.value(GridContentModel::KindRole), QByteArray("kind"));
    QCOMPARE(names.value(GridContentModel::AvailableRole), QByteArray("available"));
    QCOMPARE(names.value(GridContentModel::LastKnownNameRole), QByteArray("last_known_name"));
    QCOMPARE(names.value(GridContentModel::ListItemIdRole), QByteArray("list_item_id"));
    QCOMPARE(names.value(GridContentModel::ListIdRole), QByteArray("list_id"));
    QCOMPARE(names.value(GridContentModel::ItemKindRole), QByteArray("item_kind"));
}

void GridContentModelTest::listCardsReplaceTheOtherRows()
{
    FolderModel folders;
    folders.setupModelData(dataPath);
    ComicModel comics;
    comics.setupFolderModelData(2, dataPath);
    GridContentModel grid;
    grid.setComicModel(&comics);
    grid.setFolderModel(&folders, folders.index(qulonglong(2)));
    QAbstractItemModelTester tester(&grid, QAbstractItemModelTester::FailureReportingMode::QtTest);

    GridListCard folder;
    folder.kind = GridListCard::Folder;
    folder.listType = 5;
    folder.id = 3;
    folder.name = "Crisis";
    GridListCard list;
    list.kind = GridListCard::ReadingList;
    list.listType = 2;
    list.id = 1;
    list.name = "Infinite Crisis";
    list.cover = QUrl::fromLocalFile("/covers/hash-1.jpg");
    GridListCard smart;
    smart.kind = GridListCard::SmartList;
    smart.listType = 4;
    smart.id = 9;
    smart.name = "Unread";
    smart.icon = QUrl::fromLocalFile("/icons/smart.svg");
    grid.setListCards({ folder, list, smart });

    QCOMPARE(grid.rowCount(), 3);
    QVERIFY(grid.isListCardRow(0));
    QVERIFY(!grid.isFolderRow(0));
    QVERIFY(!grid.isSpacerRow(0));
    QCOMPARE(grid.index(0, 0).data(GridContentModel::ItemKindRole).toInt(), int(GridContentModel::ListCardItem));
    QCOMPARE(grid.index(0, 0).data(GridContentModel::CardKindRole).toString(), QString("folder"));
    QCOMPARE(grid.index(1, 0).data(GridContentModel::CardKindRole).toString(), QString("readingList"));
    QCOMPARE(grid.index(1, 0).data(GridContentModel::TitleRole).toString(), QString("Infinite Crisis"));
    QCOMPARE(grid.index(1, 0).data(GridContentModel::CoverPathRole).toUrl(), list.cover);
    QCOMPARE(grid.index(2, 0).data(GridContentModel::CardKindRole).toString(), QString("smartList"));
    QCOMPARE(grid.index(2, 0).data(GridContentModel::IconPathRole).toUrl(), smart.icon);
    QCOMPARE(grid.listCardAt(2).listType, 4);
    QCOMPARE(grid.listCardAt(2).id, 9ULL);

    // the comic model does not reach the cards
    comics.setupFolderModelData(3, dataPath);
    comics.setupFolderModelData(2, dataPath);
    QCOMPARE(grid.rowCount(), 3);

    // a folder model or a comic model ends the cards
    grid.setFolderModel(&folders, folders.index(qulonglong(2)));
    QVERIFY(!grid.isListCardRow(0));
    QCOMPARE(grid.rowCount(), 3);
    grid.setListCards({ folder });
    grid.setComicModel(&comics);
    QCOMPARE(grid.rowCount(), 2);
    QCOMPARE(grid.index(0, 0).data(GridContentModel::ItemKindRole).toInt(), int(GridContentModel::ComicItem));
}

void GridContentModelTest::removedRowsUpdateTheSourceIndexOfTheNextRows()
{
    QVERIFY(exec("INSERT INTO placeholder (id, number) VALUES (1, '0')"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 1)"));
    FolderModel folders;
    folders.setupModelData(dataPath);
    ComicModel comics;
    comics.setupFolderModelData(2, dataPath);
    GridContentModel grid;
    grid.setComicModel(&comics);
    grid.setFolderModel(&folders, folders.index(qulonglong(2)));
    QAbstractItemModelTester tester(&grid, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QCOMPARE(grid.rowCount(), 4);

    QSignalSpy changed(&grid, &QAbstractItemModel::dataChanged);
    comics.removeMissingComicsFromFolder({ comics.indexOf(PlaceholderId(1)) });
    QCOMPARE(grid.rowCount(), 3);
    QCOMPARE(changed.count(), 1);
    QCOMPARE(changed.at(0).at(0).toModelIndex().row(), 1);
    QCOMPARE(changed.at(0).at(1).toModelIndex().row(), 2);
    QCOMPARE(changed.at(0).at(2).value<QList<int>>(), QList<int>({ GridContentModel::SourceIndexRole }));
    for (int row = 1; row < grid.rowCount(); row++)
        QCOMPARE(grid.index(row, 0).data(GridContentModel::SourceIndexRole).toInt(), row - 1);
}

QTEST_MAIN(GridContentModelTest)

#include "main.moc"
