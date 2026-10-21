#include "data_base_management.h"
#include "db_helper.h"
#include "reading_list_item.h"
#include "reading_list_model.h"
#include "yacreader_global.h"
#include "yacreader_global_gui.h"

#include <QAbstractItemModelTester>
#include <QApplication>
#include <QDataStream>
#include <QDir>
#include <QMimeData>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

using namespace YACReader;

class ListModelTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void labelsAreSortedByColorAndName();
    void addedLabelHasItsColorAndId();
    void addedListsHaveTheirOrder();
    void renameChangesTheRowAndTheDatabase();
    void deleteRemovesTheSublist();
    void droppedComicsGiveComicInfoToListsAndComicsToFavorites();
    void smartListsComeAfterTheLabels();
    void comicsCannotBeDroppedOnASmartList();
    void smartListsCanBeAddedEditedAndDeleted();
    void childListsHaveTheListIcon();
    void foldersFormATreeInEachSection();
    void listsAndFoldersAreAddedInsideFolders();
    void deletingAFolderRemovesItsRows();
    void foldersAreRenamedAndTakeNoComics();
    void rowsAreFoundByTypeAndId();

private:
    std::unique_ptr<QTemporaryDir> root;
    QString dataPath;

    bool exec(const QString &sql);
    QString value(const QString &sql);
    QStringList names(ReadingListModel &model, const QModelIndex &parent = QModelIndex());
    QModelIndex find(ReadingListModel &model, const QString &name, const QModelIndex &parent = QModelIndex());
};

void ListModelTest::init()
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

    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Zed', 'green', 4), (2, 'Alpha', 'red', 1), (3, 'Beta', 'green', 4)"));
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name) VALUES (1, NULL, 0, 'Events'), (2, 1, 0, 'First')"));
}

void ListModelTest::cleanup()
{
    root.reset();
}

bool ListModelTest::exec(const QString &sql)
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

QString ListModelTest::value(const QString &sql)
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

QStringList ListModelTest::names(ReadingListModel &model, const QModelIndex &parent)
{
    QStringList result;
    for (int row = 0; row < model.rowCount(parent); row++)
        result.append(model.data(model.index(row, 0, parent), Qt::DisplayRole).toString());
    return result;
}

QModelIndex ListModelTest::find(ReadingListModel &model, const QString &name, const QModelIndex &parent)
{
    for (int row = 0; row < model.rowCount(parent); row++) {
        const auto index = model.index(row, 0, parent);
        if (model.data(index, Qt::DisplayRole).toString() == name)
            return index;
    }
    return { };
}

void ListModelTest::labelsAreSortedByColorAndName()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QStringList labels;
    for (const auto *label : model.getLabels())
        labels.append(label->name());
    QCOMPARE(labels, QStringList({ "Alpha", "Beta", "Zed" }));
}

void ListModelTest::addedLabelHasItsColorAndId()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    model.addNewLabel("Cyan one", YACReader::YCyan);

    const auto index = find(model, "Cyan one");
    QVERIFY(index.isValid());
    QCOMPARE(model.data(index, ReadingListModel::TypeListsRole).toInt(), int(ReadingListModel::Label));
    QCOMPARE(model.data(index, ReadingListModel::LabelColorRole).toInt(), int(YACReader::YCyan));
    QCOMPARE(model.data(index, ReadingListModel::IDRole).toULongLong(), value("SELECT id FROM label WHERE name = 'Cyan one'").toULongLong());

    QStringList labels;
    for (const auto *label : model.getLabels())
        labels.append(label->name());
    QCOMPARE(labels, QStringList({ "Alpha", "Beta", "Zed", "Cyan one" }));
}

void ListModelTest::addedListsHaveTheirOrder()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);

    model.addReadingList("Arcs");
    QVERIFY(find(model, "Arcs").isValid());
    QCOMPARE(value("SELECT parentId IS NULL FROM reading_list WHERE name = 'Arcs'"), QString("1"));

    const auto events = find(model, "Events");
    QVERIFY(events.isValid());
    model.addReadingListAt("Second", events);
    QCOMPARE(names(model, events), QStringList({ "First", "Second" }));
    QCOMPARE(value("SELECT ordering FROM reading_list WHERE name = 'Second'"), QString("1"));
    QCOMPARE(value("SELECT parentId FROM reading_list WHERE name = 'Second'"), QString("1"));
}

void ListModelTest::renameChangesTheRowAndTheDatabase()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);

    model.rename(find(model, "Zed"), "Zeta");
    QVERIFY(find(model, "Zeta").isValid());
    QCOMPARE(value("SELECT name FROM label WHERE id = 1"), QString("Zeta"));

    const auto events = find(model, "Events");
    model.rename(find(model, "First", events), "Renamed");
    QCOMPARE(names(model, find(model, "Events")), QStringList({ "Renamed" }));
    QCOMPARE(value("SELECT name FROM reading_list WHERE id = 2"), QString("Renamed"));
}

void ListModelTest::deleteRemovesTheSublist()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);

    const auto events = find(model, "Events");
    model.deleteItem(find(model, "First", events));
    QCOMPARE(model.rowCount(find(model, "Events")), 0);
    QCOMPARE(value("SELECT count(*) FROM reading_list WHERE id = 2"), QString("0"));
}

void ListModelTest::droppedComicsGiveComicInfoToListsAndComicsToFavorites()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);

    QList<qulonglong> favorites;
    QList<QPair<QList<ComicRowId>, qulonglong>> labels;
    QList<QPair<QList<ComicRowId>, qulonglong>> lists;
    connect(&model, &ReadingListModel::addComicsToFavorites, this, [&](const QList<qulonglong> &comicIds) { favorites = comicIds; });
    connect(&model, &ReadingListModel::addComicsToLabel, this, [&](const QList<ComicRowId> &rows, qulonglong id) { labels.append({ rows, id }); });
    connect(&model, &ReadingListModel::addComicsToReadingList, this, [&](const QList<ComicRowId> &rows, qulonglong id) { lists.append({ rows, id }); });

    QMimeData data;
    QByteArray comicIds;
    QDataStream out(&comicIds, QIODevice::WriteOnly);
    out << QList<qulonglong> { 9, 9 };
    data.setData(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat, comicIds);

    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, find(model, "Alpha")));
    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, find(model, "Events")));
    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, model.index(0, 0)));

    QCOMPARE(labels.size(), 1);
    QVERIFY(labels.first().first == QList<ComicRowId>({ ComicId(9), ComicId(9) }));
    QCOMPARE(labels.first().second, 2ULL);
    QCOMPARE(lists.size(), 1);
    QCOMPARE(lists.first().second, 1ULL);

    // a drag from a list names its entries, so a placeholder or a deleted comic can be added too
    YACReader::setListEntriesMimeData(&data, { { ListKind::ReadingList, 7 }, { ListKind::Label, 3 } });
    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, find(model, "Events")));
    QCOMPARE(lists.size(), 2);
    QVERIFY(lists.last().first == QList<ComicRowId>({ ListItemId { ListKind::ReadingList, 7 }, ListItemId { ListKind::Label, 3 } }));
    QCOMPARE(favorites, QList<qulonglong>({ 9, 9 }));
}

QStringList typesOfRows(ReadingListModel &model)
{
    QStringList types;
    for (int row = 0; row < model.rowCount(); row++) {
        switch (model.data(model.index(row, 0), ReadingListModel::TypeListsRole).toInt()) {
        case ReadingListModel::SpecialList:
            types.append("special");
            break;
        case ReadingListModel::Label:
            types.append("label");
            break;
        case ReadingListModel::ReadingList:
            types.append("list");
            break;
        case ReadingListModel::Separator:
            types.append("-");
            break;
        case ReadingListModel::SmartList:
            types.append("smart");
            break;
        case ReadingListModel::ReadingListFolder:
            types.append("reading folder");
            break;
        case ReadingListModel::SmartListFolder:
            types.append("smart folder");
            break;
        }
    }
    return types;
}

void ListModelTest::smartListsComeAfterTheLabels()
{
    {
        ReadingListModel model;
        model.setupReadingListsData(dataPath);
        // without smart lists there is no third separator
        QCOMPARE(typesOfRows(model), QStringList({ "special", "special", "special", "-", "label", "label", "label", "-", "list" }));
    }

    QVERIFY(exec("INSERT INTO smart_list (id, name, query, ordering) VALUES (1, 'Second', 'read:true', 1), (2, 'First', 'read:false', 0)"));
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QCOMPARE(typesOfRows(model), QStringList({ "special", "special", "special", "-", "label", "label", "label", "-", "smart", "smart", "-", "list" }));
    QCOMPARE(names(model).mid(8, 2), QStringList({ "First", "Second" }));

    const auto first = find(model, "First");
    QCOMPARE(model.data(first, ReadingListModel::IDRole).toULongLong(), 2ULL);
    QVERIFY(!model.data(first, Qt::DecorationRole).isNull());
    QCOMPARE(model.flags(first), Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    QCOMPARE(model.rowCount(first), 0);
    QVERIFY(model.isEditable(first));
    QVERIFY(model.isEditable(find(model, "Events")));
    QVERIFY(model.isEditable(find(model, "Alpha")));

    // the sublists of a reading list still find their parent row
    const auto events = find(model, "Events");
    QCOMPARE(events.row(), 11);
    const auto sublist = find(model, "First", events);
    QVERIFY(sublist.isValid());
    QCOMPARE(model.parent(sublist), events);
}

void ListModelTest::comicsCannotBeDroppedOnASmartList()
{
    QVERIFY(exec("INSERT INTO smart_list (id, name, query, ordering) VALUES (1, 'Unread', 'read:false', 0)"));
    ReadingListModel model;
    model.setupReadingListsData(dataPath);

    QMimeData data;
    QByteArray comicIds;
    QDataStream out(&comicIds, QIODevice::WriteOnly);
    out << QList<qulonglong> { 9 };
    data.setData(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat, comicIds);

    const auto smartList = find(model, "Unread");
    QVERIFY(!model.canDropMimeData(&data, Qt::CopyAction, smartList.row(), 0, QModelIndex()));
    QVERIFY(model.canDropMimeData(&data, Qt::CopyAction, find(model, "Events").row(), 0, QModelIndex()));
}

void ListModelTest::smartListsCanBeAddedEditedAndDeleted()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    const auto rows = model.rowCount();

    // the first one also brings its separator
    const auto unread = model.addSmartList("Unread", "read:false");
    QVERIFY(unread.isValid());
    QCOMPARE(model.rowCount(), rows + 2);
    QCOMPARE(model.data(unread, Qt::DisplayRole).toString(), QString("Unread"));
    QCOMPARE(model.smartList(unread)->query, QString("read:false"));
    QCOMPARE(value("SELECT query FROM smart_list WHERE name = 'Unread'"), QString("read:false"));

    const auto rated = model.addSmartList("Rated", "rating>=4");
    QCOMPARE(model.rowCount(), rows + 3);
    QCOMPARE(rated.row(), unread.row() + 1);
    QCOMPARE(value("SELECT ordering FROM smart_list WHERE name = 'Rated'"), QString("1"));
    QVERIFY(!model.smartList(find(model, "Events")));

    QVERIFY(model.updateSmartList(rated, "Highly rated", "rating>=5"));
    QCOMPARE(model.data(rated, Qt::DisplayRole).toString(), QString("Highly rated"));
    QCOMPARE(value("SELECT query FROM smart_list WHERE name = 'Highly rated'"), QString("rating>=5"));
    QVERIFY(!model.updateSmartList(find(model, "Events"), "Not smart", "read:true"));

    model.deleteItem(find(model, "Unread"));
    QCOMPARE(model.rowCount(), rows + 2);
    // the last one also takes its separator
    model.deleteItem(find(model, "Highly rated"));
    QCOMPARE(model.rowCount(), rows);
    QCOMPARE(value("SELECT count(*) FROM smart_list"), QString("0"));
    QVERIFY(find(model, "Events").isValid());

    // a new model reads the same order
    model.addSmartList("B", "read:true");
    model.addSmartList("A", "read:false");
    ReadingListModel reloaded;
    reloaded.setupReadingListsData(dataPath);
    QCOMPARE(names(reloaded).mid(rows - 1, 2), QStringList({ "B", "A" }));
}

void ListModelTest::childListsHaveTheListIcon()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);

    const auto events = find(model, "Events");
    const auto first = find(model, "First", events);
    QVERIFY(first.isValid());
    const auto listIcon = model.data(events, Qt::DecorationRole).value<QIcon>();
    const auto childIcon = model.data(first, Qt::DecorationRole).value<QIcon>();
    QCOMPARE(childIcon.cacheKey(), listIcon.cacheKey());
}

void ListModelTest::foldersFormATreeInEachSection()
{
    // DC > Crisis > (Final Crisis); DC > (Events); Unread (smart folder) > (Unread Batman)
    QVERIFY(exec("INSERT INTO reading_list_folder (id, parent_id, name, ordering) VALUES (1, NULL, 'DC', 0), (2, 1, 'Crisis', 0)"));
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name, folder_id) VALUES (3, NULL, 0, 'Final Crisis', 2)"));
    QVERIFY(exec("UPDATE reading_list SET folder_id = 1 WHERE id = 1"));
    QVERIFY(exec("INSERT INTO smart_list_folder (id, parent_id, name, ordering) VALUES (1, NULL, 'Unread', 0)"));
    QVERIFY(exec("INSERT INTO smart_list (id, name, query, ordering, folder_id) VALUES (1, 'Unread Batman', 'read:false', 0, 1), (2, 'Rated', 'rating>=4', 1, NULL)"));

    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    QCOMPARE(typesOfRows(model), QStringList({ "special", "special", "special", "-", "label", "label", "label", "-", "smart folder", "smart", "-", "reading folder" }));

    const auto unread = find(model, "Unread");
    QCOMPARE(model.listFolder(unread), std::optional<ListFolderId>(ListFolderId { ListFolderKind::SmartList, 1 }));
    QCOMPARE(model.section(unread), std::optional<ListFolderKind>(ListFolderKind::SmartList));
    QCOMPARE(names(model, unread), QStringList({ "Unread Batman" }));
    QCOMPARE(model.parent(find(model, "Unread Batman", unread)), unread);

    // in a folder, its folders first, then its lists
    const auto dc = find(model, "DC");
    QCOMPARE(model.data(dc, ReadingListModel::TypeListsRole).toInt(), int(ReadingListModel::ReadingListFolder));
    QCOMPARE(names(model, dc), QStringList({ "Crisis", "Events" }));
    const auto crisis = find(model, "Crisis", dc);
    QCOMPARE(model.parent(crisis), dc);
    const auto finalCrisis = find(model, "Final Crisis", crisis);
    QCOMPARE(model.parent(finalCrisis), crisis);
    QCOMPARE(model.section(finalCrisis), std::optional<ListFolderKind>(ListFolderKind::ReadingList));

    // a child list of a list in a folder
    const auto events = find(model, "Events", dc);
    const auto first = find(model, "First", events);
    QVERIFY(first.isValid());
    QCOMPARE(model.parent(first), events);
    QVERIFY(model.isReadingSubList(first));
    QVERIFY(!model.isReadingSubList(events));
    QCOMPARE(model.section(find(model, "Alpha")), std::nullopt);
}

void ListModelTest::listsAndFoldersAreAddedInsideFolders()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    const auto rows = model.rowCount();

    const auto dc = model.addListFolder(ListFolderKind::ReadingList, QModelIndex(), "DC");
    QVERIFY(dc.isValid());
    QCOMPARE(model.rowCount(), rows + 1);
    // the folders of a section come before its lists
    QCOMPARE(names(model).mid(rows - 1), QStringList({ "DC", "Events" }));

    const auto crisis = model.addListFolder(ListFolderKind::ReadingList, dc, "Crisis");
    QCOMPARE(model.parent(crisis), find(model, "DC"));
    QCOMPARE(value("SELECT parent_id FROM reading_list_folder WHERE name = 'Crisis'"), value("SELECT id FROM reading_list_folder WHERE name = 'DC'"));

    // reading lists in a folder are sorted by name, after its folders
    model.addReadingList("Zero Hour", find(model, "DC"));
    model.addReadingList("Blackest Night", find(model, "DC"));
    QCOMPARE(names(model, find(model, "DC")), QStringList({ "Crisis", "Blackest Night", "Zero Hour" }));
    QCOMPARE(value("SELECT folder_id FROM reading_list WHERE name = 'Zero Hour'"), value("SELECT id FROM reading_list_folder WHERE name = 'DC'"));

    // a folder of the other section is ignored
    const auto smartFolder = model.addListFolder(ListFolderKind::SmartList, QModelIndex(), "Unread");
    QVERIFY(smartFolder.isValid());
    QCOMPARE(typesOfRows(model).mid(8, 2), QStringList({ "smart folder", "-" }));
    model.addReadingList("Top", find(model, "Unread"));
    QCOMPARE(value("SELECT folder_id IS NULL FROM reading_list WHERE name = 'Top'"), QString("1"));

    const auto batman = model.addSmartList("Unread Batman", "read:false", find(model, "Unread"));
    QVERIFY(batman.isValid());
    QCOMPARE(model.parent(batman), find(model, "Unread"));
    QCOMPARE(model.smartList(batman)->folderId, std::optional<qulonglong>(model.listFolder(find(model, "Unread"))->id));
    model.addSmartList("Rated", "rating>=4");
    QCOMPARE(names(model).mid(8, 2), QStringList({ "Unread", "Rated" }));
}

void ListModelTest::deletingAFolderRemovesItsRows()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    const auto rows = model.rowCount();

    const auto unread = model.addListFolder(ListFolderKind::SmartList, QModelIndex(), "Unread");
    model.addSmartList("Unread Batman", "read:false", unread);
    const auto dc = model.addListFolder(ListFolderKind::ReadingList, QModelIndex(), "DC");
    model.addListFolder(ListFolderKind::ReadingList, dc, "Crisis");
    model.addReadingList("Infinite Crisis", find(model, "DC"));
    QCOMPARE(model.rowCount(), rows + 3);

    model.deleteItem(find(model, "DC"));
    QCOMPARE(model.rowCount(), rows + 2);
    QCOMPARE(value("SELECT count(*) FROM reading_list_folder"), QString("0"));
    QCOMPARE(value("SELECT count(*) FROM reading_list WHERE name = 'Infinite Crisis'"), QString("0"));

    // the last row of the smart lists section takes its separator
    model.deleteItem(find(model, "Unread"));
    QCOMPARE(model.rowCount(), rows);
    QCOMPARE(value("SELECT count(*) FROM smart_list"), QString("0"));
    QVERIFY(find(model, "Events").isValid());
}

void ListModelTest::foldersAreRenamedAndTakeNoComics()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    const auto dc = model.addListFolder(ListFolderKind::ReadingList, QModelIndex(), "DC");
    QVERIFY(model.isEditable(dc));
    model.rename(dc, "DC Comics");
    QCOMPARE(model.data(find(model, "DC Comics"), Qt::DisplayRole).toString(), QString("DC Comics"));
    QCOMPARE(value("SELECT name FROM reading_list_folder"), QString("DC Comics"));

    QMimeData data;
    QByteArray comicIds;
    QDataStream out(&comicIds, QIODevice::WriteOnly);
    out << QList<qulonglong> { 9 };
    data.setData(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat, comicIds);
    const auto folder = find(model, "DC Comics");
    QVERIFY(!model.canDropMimeData(&data, Qt::CopyAction, folder.row(), 0, QModelIndex()));
    QVERIFY(!model.dropMimeData(&data, Qt::CopyAction, -1, 0, folder));
    QCOMPARE(model.flags(folder), Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

void ListModelTest::rowsAreFoundByTypeAndId()
{
    QVERIFY(exec("INSERT INTO reading_list_folder (id, parent_id, name, ordering) VALUES (1, NULL, 'DC', 0), (2, 1, 'Crisis', 0)"));
    QVERIFY(exec("UPDATE reading_list SET folder_id = 2 WHERE id = 1"));
    QVERIFY(exec("INSERT INTO smart_list_folder (id, parent_id, name, ordering) VALUES (2, NULL, 'Unread', 0)"));

    ReadingListModel model;
    model.setupReadingListsData(dataPath);

    const auto crisis = model.indexOf(ReadingListModel::ReadingListFolder, 2);
    QCOMPARE(model.data(crisis, Qt::DisplayRole).toString(), QString("Crisis"));
    QCOMPARE(model.data(model.indexOf(ReadingListModel::ReadingList, 1), Qt::DisplayRole).toString(), QString("Events"));
    QCOMPARE(model.data(model.indexOf(ReadingListModel::ReadingList, 2), Qt::DisplayRole).toString(), QString("First"));
    // the same id in the other section is another row
    QCOMPARE(model.data(model.indexOf(ReadingListModel::SmartListFolder, 2), Qt::DisplayRole).toString(), QString("Unread"));
    QVERIFY(!model.indexOf(ReadingListModel::SmartListFolder, 1).isValid());
}

QTEST_MAIN(ListModelTest)

#include "main.moc"
