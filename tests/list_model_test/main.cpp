#include "data_base_management.h"
#include "db_helper.h"
#include "list_folder.h"
#include "qnaturalsorting.h"
#include "reading_list.h"
#include "reading_list_item.h"
#include "reading_list_model.h"
#include "smart_list.h"
#include "table_projection.h"
#include "yacreader_global.h"
#include "yacreader_global_gui.h"

#include <QAbstractItemModelTester>
#include <QApplication>
#include <QDataStream>
#include <QDir>
#include <QMimeData>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
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
    void loadedLabelsHaveTheOrderOfAddingByColorAndName();
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
    void foldersAndListsAreSortedByName();
    void loadedFoldersHaveTheOrderOfAddingByName();
    void renamingMovesTheRowToItsName();
    void foldersAndListsMoveToFolders();
    void movesStayInTheirSectionAndOutOfThemselves();
    void droppedRowsGoIntoFoldersNextToListsAndToTheTopLevel();
    void childListsAreReorderedByDrops();
    void rowsDroppedBetweenRowsGoNextToThem();

private:
    std::unique_ptr<QTemporaryDir> root;
    QString dataPath;

    bool exec(const QString &sql);
    QString value(const QString &sql);
    QStringList names(ReadingListModel &model, const QModelIndex &parent = QModelIndex());
    QModelIndex find(ReadingListModel &model, const QString &name, const QModelIndex &parent = QModelIndex());
    std::unique_ptr<QMimeData> rowData(ReadingListModel &model, const QModelIndex &index);
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

std::unique_ptr<QMimeData> ListModelTest::rowData(ReadingListModel &model, const QModelIndex &index)
{
    return std::unique_ptr<QMimeData>(model.mimeData({ index }));
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

void ListModelTest::loadedLabelsHaveTheOrderOfAddingByColorAndName()
{
    QVERIFY(exec("DELETE FROM label"));
    // equal names without case keep the order of the first load
    const QStringList names = { "Batman", "batman", "BATMAN", "List 2", "List 10", "list 2", "Été", "ete", "X", "x-men", "X-Men", "Zeta" };
    QRandomGenerator random(20261025);
    QList<int> ids;
    for (int id = 1; id <= 80; id++)
        ids.insert(random.bounded(int(ids.size()) + 1), id);
    for (const auto id : std::as_const(ids)) {
        const auto color = random.bounded(4) + 1;
        QVERIFY(exec(QString("INSERT INTO label (id, name, color, ordering) VALUES (%1, '%2', 'red', %3)").arg(id).arg(names.at(random.bounded(int(names.size()))), QString::number(color))));
    }

    // the reference: the old load query, then the insert by color and name of each label
    QList<std::tuple<int, QString, QString>> reference; // color, name, id
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        connectionName = db.connectionName();
        QSqlQuery query("SELECT id, name, ordering FROM label ORDER BY ordering, name", db);
        while (query.next()) {
            const auto color = query.value(2).toInt();
            const auto name = query.value(1).toString();
            int row = 0;
            while (row < reference.size() && std::get<0>(reference.at(row)) < color)
                row++;
            while (row < reference.size() && std::get<0>(reference.at(row)) == color && naturalSortLessThanCI(std::get<1>(reference.at(row)), name))
                row++;
            reference.insert(row, { color, name, query.value(0).toString() });
        }
    }
    QSqlDatabase::removeDatabase(connectionName);

    QStringList expected;
    for (const auto &label : std::as_const(reference))
        expected.append(std::get<2>(label));

    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QStringList loaded;
    for (const auto *label : model.getLabels())
        loaded.append(QString::number(label->getId()));
    QCOMPARE(loaded, expected);
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
    YACReader::setRowsMimeData(&data, { ComicId(9), ComicId(9) });

    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, find(model, "Alpha")));
    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, find(model, "Events")));
    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, model.index(0, 0)));

    QCOMPARE(labels.size(), 1);
    QVERIFY(labels.first().first == QList<ComicRowId>({ ComicId(9), ComicId(9) }));
    QCOMPARE(labels.first().second, 2ULL);
    QCOMPARE(lists.size(), 1);
    QCOMPARE(lists.first().second, 1ULL);

    // a drag names its rows, so a placeholder or a deleted comic can be added too
    YACReader::setRowsMimeData(&data, { ListItemId { ListKind::ReadingList, 7 }, PlaceholderId(4), ListItemId { ListKind::Label, 3 } });
    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, find(model, "Events")));
    QCOMPARE(lists.size(), 2);
    QVERIFY(lists.last().first == QList<ComicRowId>({ ListItemId { ListKind::ReadingList, 7 }, PlaceholderId(4), ListItemId { ListKind::Label, 3 } }));
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
    QCOMPARE(model.flags(first), Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
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
    QCOMPARE(names(model).mid(rows - 1, 2), QStringList({ "Rated", "Unread" }));
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

    // a new model sorts them by name too
    model.addSmartList("B", "read:true");
    model.addSmartList("A", "read:false");
    QCOMPARE(names(model).mid(rows - 1, 2), QStringList({ "A", "B" }));
    ReadingListModel reloaded;
    reloaded.setupReadingListsData(dataPath);
    QCOMPARE(names(reloaded).mid(rows - 1, 2), QStringList({ "A", "B" }));
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
    QCOMPARE(model.flags(folder), Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled);
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

void ListModelTest::foldersAndListsAreSortedByName()
{
    QVERIFY(exec("INSERT INTO reading_list_folder (id, parent_id, name, ordering) VALUES (1, NULL, 'Marvel', 0), (2, NULL, 'DC', 1), (3, 1, 'X-Men', 0), (4, 1, 'Avengers', 1)"));
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name, folder_id) VALUES (3, NULL, 0, 'Secret Wars', 1), (4, NULL, 1, 'Civil War', 1), (5, 1, 1, 'Another', NULL)"));
    QVERIFY(exec("INSERT INTO smart_list_folder (id, parent_id, name, ordering) VALUES (1, NULL, 'Unread', 0), (2, NULL, 'Rated', 1)"));
    QVERIFY(exec("INSERT INTO smart_list (id, name, query, ordering, folder_id) VALUES (1, 'Zed', 'read:false', 0, NULL), (2, 'Alpha', 'read:true', 1, NULL), "
                 "(3, 'Later', 'read:false', 0, 1), (4, 'Batman', 'read:false', 1, 1)"));

    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    // folders first, then lists, each by name, whatever their stored order
    QCOMPARE(names(model).mid(8), QStringList({ "Rated", "Unread", "Alpha", "Zed", "", "DC", "Marvel", "Events" }));
    QCOMPARE(names(model, find(model, "Marvel")), QStringList({ "Avengers", "X-Men", "Civil War", "Secret Wars" }));
    QCOMPARE(names(model, find(model, "Unread")), QStringList({ "Batman", "Later" }));
    // child lists keep their order: it is the reading order of their parent
    QCOMPARE(names(model, find(model, "Events")), QStringList({ "First", "Another" }));

    // new rows go to their place by name
    model.addListFolder(ListFolderKind::ReadingList, find(model, "Marvel"), "Daredevil");
    model.addSmartList("Big", "rating>=4", find(model, "Unread"));
    QCOMPARE(names(model, find(model, "Marvel")), QStringList({ "Avengers", "Daredevil", "X-Men", "Civil War", "Secret Wars" }));
    QCOMPARE(names(model, find(model, "Unread")), QStringList({ "Batman", "Big", "Later" }));
}

namespace {

// a folder of the sidebar as the first load built it: each child inserted by name, one by one, in the load order
struct ReferenceFolder {
    QList<std::pair<QString, QString>> folders; // key, name
    QList<std::pair<QString, QString>> lists;

    static void insert(QList<std::pair<QString, QString>> &children, const QString &key, const QString &name)
    {
        const auto row = std::count_if(children.cbegin(), children.cend(), [&name](const auto &child) { return naturalSortLessThanCI(child.second, name); });
        children.insert(row, { key, name });
    }
    QStringList keys() const
    {
        QStringList result;
        for (const auto &child : folders)
            result.append(child.first);
        for (const auto &child : lists)
            result.append(child.first);
        return result;
    }
};

void readModelFolders(ReadingListModel &model, const QModelIndex &parent, const QString &key, QHash<QString, QStringList> &result)
{
    for (int row = 0; row < model.rowCount(parent); row++) {
        const auto index = model.index(row, 0, parent);
        const auto type = model.data(index, ReadingListModel::TypeListsRole).toInt();
        const auto id = model.data(index, ReadingListModel::IDRole).toString();
        switch (type) {
        case ReadingListModel::ReadingListFolder:
            result[key + "R"].append("F" + id);
            readModelFolders(model, index, "R" + id, result);
            break;
        case ReadingListModel::SmartListFolder:
            result[key + "S"].append("F" + id);
            readModelFolders(model, index, "S" + id, result);
            break;
        case ReadingListModel::ReadingList:
            result[key + "R"].append("L" + id);
            break;
        case ReadingListModel::SmartList:
            result[key + "S"].append("L" + id);
            break;
        default:
            break;
        }
    }
}

}

void ListModelTest::loadedFoldersHaveTheOrderOfAddingByName()
{
    QVERIFY(exec("DELETE FROM reading_list"));
    // equal names without case keep the order of the first load
    const QStringList names = { "Batman", "batman", "BATMAN", "List 2", "List 10", "list 2", "Été", "ete", "X", "x-men", "X-Men" };
    QRandomGenerator random(20261023);
    const auto pick = [&] { return names.at(random.bounded(int(names.size()))); };

    for (const auto &kind : { QString("reading_list_folder"), QString("smart_list_folder") }) {
        for (int id = 1; id <= 30; id++) {
            const auto parent = id > 3 && random.bounded(2) == 0 ? QString::number(random.bounded(id - 1) + 1) : QString("NULL");
            QVERIFY(exec(QString("INSERT INTO %1 (id, parent_id, name, ordering) VALUES (%2, %3, '%4', %5)").arg(kind).arg(id).arg(parent, pick()).arg(random.bounded(3))));
        }
    }
    // ids in a random order, so the load order is not the id order
    QList<int> ids;
    for (int id = 1; id <= 150; id++)
        ids.insert(random.bounded(int(ids.size()) + 1), id);
    for (const auto id : std::as_const(ids)) {
        const auto folder = random.bounded(3) == 0 ? QString("NULL") : QString::number(random.bounded(30) + 1);
        QVERIFY(exec(QString("INSERT INTO reading_list (id, parentId, ordering, name, folder_id) VALUES (%1, NULL, %2, '%3', %4)").arg(id).arg(random.bounded(3)).arg(pick(), folder)));
        QVERIFY(exec(QString("INSERT INTO smart_list (id, name, query, ordering, folder_id) VALUES (%1, '%2', 'read:true', %3, %4)").arg(id).arg(pick()).arg(random.bounded(3)).arg(folder)));
    }
    // child lists stay in their parent
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name) VALUES (500, 7, 0, 'Child'), (501, 7, 1, 'batman')"));

    // the reference: the same load queries, then the insert by name of each one
    QHash<QString, ReferenceFolder> reference;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        connectionName = db.connectionName();
        for (const auto &[kind, prefix] : { std::pair(ListFolderKind::ReadingList, QString("R")), std::pair(ListFolderKind::SmartList, QString("S")) }) {
            const auto folders = DBHelper::loadListFolders(kind, db);
            QVERIFY(folders);
            for (const auto &folder : *folders)
                ReferenceFolder::insert(reference[(folder.parentId ? prefix + QString::number(*folder.parentId) : QString()) + prefix].folders, "F" + QString::number(folder.id.id), folder.name);
        }
        const auto smartLists = DBHelper::loadSmartLists(db);
        QVERIFY(smartLists);
        for (const auto &list : *smartLists)
            ReferenceFolder::insert(reference[(list.folderId ? "S" + QString::number(*list.folderId) : QString()) + "S"].lists, "L" + QString::number(list.id.value()), list.name);
        QSqlQuery query("SELECT " + TableProjection::selectList("rl", ::ReadingList::columns()) + " FROM reading_list rl ORDER BY rl.parentId IS NULL DESC", db);
        const auto indexes = ::ReadingList::columnIndexes(query.record(), "rl");
        while (query.next()) {
            const auto list = ::ReadingList::fromQuery(query, indexes);
            if (!list.parentId)
                ReferenceFolder::insert(reference[(list.folderId ? "R" + QString::number(*list.folderId) : QString()) + "R"].lists, "L" + QString::number(list.id.value()), list.name);
        }
    }
    QSqlDatabase::removeDatabase(connectionName);

    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QHash<QString, QStringList> loaded;
    readModelFolders(model, QModelIndex(), QString(), loaded);

    QHash<QString, QStringList> expected;
    for (auto folder = reference.cbegin(); folder != reference.cend(); ++folder)
        expected.insert(folder.key(), folder.value().keys());
    QVERIFY(expected.size() > 20);
    QCOMPARE(loaded, expected);
}

void ListModelTest::renamingMovesTheRowToItsName()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    model.addListFolder(ListFolderKind::ReadingList, QModelIndex(), "DC");
    model.addListFolder(ListFolderKind::ReadingList, QModelIndex(), "Marvel");
    model.addReadingList("Zero Hour");
    QCOMPARE(names(model).mid(8), QStringList({ "DC", "Marvel", "Events", "Zero Hour" }));

    model.rename(find(model, "DC"), "Vertigo");
    QCOMPARE(names(model).mid(8), QStringList({ "Marvel", "Vertigo", "Events", "Zero Hour" }));
    model.rename(find(model, "Zero Hour"), "Armageddon");
    QCOMPARE(names(model).mid(8), QStringList({ "Marvel", "Vertigo", "Armageddon", "Events" }));
    QCOMPARE(names(model, find(model, "Events")), QStringList({ "First" }));

    model.addSmartList("Unread", "read:false");
    model.addSmartList("Rated", "rating>=4");
    QVERIFY(model.updateSmartList(find(model, "Rated"), "Zed", "rating>=4"));
    QCOMPARE(names(model).mid(8, 2), QStringList({ "Unread", "Zed" }));

    // child lists keep their place
    model.addReadingListAt("Second", find(model, "Events"));
    model.rename(find(model, "First", find(model, "Events")), "Zzz");
    model.rename(find(model, "Second", find(model, "Events")), "Aaa");
    QCOMPARE(names(model, find(model, "Events")), QStringList({ "Zzz", "Aaa" }));
}

void ListModelTest::foldersAndListsMoveToFolders()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    const auto dc = model.addListFolder(ListFolderKind::ReadingList, QModelIndex(), "DC");
    model.addListFolder(ListFolderKind::ReadingList, dc, "Crisis");
    const auto crisis = [&] { return find(model, "Crisis", find(model, "DC")); };

    QVERIFY(model.canMoveToFolder(find(model, "Events"), crisis()));
    QVERIFY(model.moveToFolder(find(model, "Events"), crisis()));
    QCOMPARE(names(model).mid(8), QStringList({ "DC" }));
    QCOMPARE(names(model, crisis()), QStringList({ "Events" }));
    QCOMPARE(value("SELECT folder_id FROM reading_list WHERE id = 1"), value("SELECT id FROM reading_list_folder WHERE name = 'Crisis'"));
    // its child lists go with it
    QCOMPARE(names(model, find(model, "Events", crisis())), QStringList({ "First" }));

    // a folder goes with what it holds
    QVERIFY(model.moveToFolder(crisis(), QModelIndex()));
    QCOMPARE(names(model).mid(8), QStringList({ "Crisis", "DC" }));
    QCOMPARE(names(model, find(model, "Crisis")), QStringList({ "Events" }));
    QCOMPARE(value("SELECT parent_id IS NULL FROM reading_list_folder WHERE name = 'Crisis'"), QString("1"));

    const auto unread = model.addListFolder(ListFolderKind::SmartList, QModelIndex(), "Unread");
    QVERIFY(unread.isValid());
    model.addSmartList("Batman", "read:false");
    QVERIFY(model.moveToFolder(find(model, "Batman"), find(model, "Unread")));
    QCOMPARE(typesOfRows(model).mid(8, 2), QStringList({ "smart folder", "-" }));
    QCOMPARE(names(model, find(model, "Unread")), QStringList({ "Batman" }));
    QCOMPARE(value("SELECT folder_id FROM smart_list WHERE name = 'Batman'"), value("SELECT id FROM smart_list_folder WHERE name = 'Unread'"));
    QVERIFY(model.moveToFolder(find(model, "Batman", find(model, "Unread")), QModelIndex()));
    QCOMPARE(names(model).mid(8, 2), QStringList({ "Unread", "Batman" }));
    QVERIFY(model.moveToFolder(find(model, "Batman"), find(model, "Unread")));

    // a new model reads the same tree
    ReadingListModel reloaded;
    reloaded.setupReadingListsData(dataPath);
    QCOMPARE(names(reloaded, find(reloaded, "Crisis")), QStringList({ "Events" }));
    QCOMPARE(names(reloaded, find(reloaded, "Unread")), QStringList({ "Batman" }));
}

void ListModelTest::movesStayInTheirSectionAndOutOfThemselves()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    const auto dc = model.addListFolder(ListFolderKind::ReadingList, QModelIndex(), "DC");
    model.addListFolder(ListFolderKind::ReadingList, dc, "Crisis");
    model.addListFolder(ListFolderKind::SmartList, QModelIndex(), "Unread");

    // the other section
    QVERIFY(!model.canMoveToFolder(find(model, "Events"), find(model, "Unread")));
    QVERIFY(!model.moveToFolder(find(model, "Events"), find(model, "Unread")));
    // a folder into itself or a folder below it
    QVERIFY(!model.canMoveToFolder(find(model, "DC"), find(model, "DC")));
    QVERIFY(!model.canMoveToFolder(find(model, "DC"), find(model, "Crisis", find(model, "DC"))));
    // the place it is in
    QVERIFY(!model.canMoveToFolder(find(model, "Events"), QModelIndex()));
    QVERIFY(!model.canMoveToFolder(find(model, "Crisis", find(model, "DC")), find(model, "DC")));
    // child lists, labels, and rows that are not folders
    QVERIFY(!model.canMoveToFolder(find(model, "First", find(model, "Events")), find(model, "DC")));
    QVERIFY(!model.canMoveToFolder(find(model, "Alpha"), find(model, "DC")));
    QVERIFY(!model.canMoveToFolder(find(model, "Events"), find(model, "Alpha")));

    QCOMPARE(value("SELECT folder_id IS NULL FROM reading_list WHERE id = 1"), QString("1"));
    QCOMPARE(value("SELECT parent_id IS NULL FROM reading_list_folder WHERE name = 'DC'"), QString("1"));
}

void ListModelTest::droppedRowsGoIntoFoldersNextToListsAndToTheTopLevel()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    model.addListFolder(ListFolderKind::ReadingList, QModelIndex(), "DC");
    model.addReadingList("Zero Hour", find(model, "DC"));
    model.addListFolder(ListFolderKind::ReadingList, QModelIndex(), "Marvel");
    model.addListFolder(ListFolderKind::SmartList, QModelIndex(), "Unread");

    // onto a folder: inside it
    auto data = rowData(model, find(model, "Events"));
    QVERIFY(model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "DC")));
    QVERIFY(model.dropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "DC")));
    QCOMPARE(names(model, find(model, "DC")), QStringList({ "Events", "Zero Hour" }));

    // onto a list: next to it, in its folder
    data = rowData(model, find(model, "Marvel"));
    QVERIFY(model.dropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Zero Hour", find(model, "DC"))));
    QCOMPARE(names(model, find(model, "DC")), QStringList({ "Marvel", "Events", "Zero Hour" }));

    // on empty space: the top level of its section
    data = rowData(model, find(model, "Events", find(model, "DC")));
    QVERIFY(model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, QModelIndex()));
    QVERIFY(model.dropMimeData(data.get(), Qt::CopyAction, -1, -1, QModelIndex()));
    QCOMPARE(names(model).mid(10), QStringList({ "DC", "Events" }));
    QCOMPARE(value("SELECT folder_id IS NULL FROM reading_list WHERE id = 1"), QString("1"));

    // refused: a label, a separator, its own place, the other section
    data = rowData(model, find(model, "Events"));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Alpha")));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, model.index(3, 0)));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Events")));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Unread")));
    QVERIFY(!model.dropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Unread")));
    // a folder into a folder below it
    data = rowData(model, find(model, "DC"));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Marvel", find(model, "DC"))));
    QVERIFY(!model.dropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Marvel", find(model, "DC"))));
    // a child list stays in its parent
    data = rowData(model, find(model, "First", find(model, "Events")));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "DC")));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, QModelIndex()));
    QCOMPARE(names(model).mid(8), QStringList({ "Unread", "", "DC", "Events" }));
}

void ListModelTest::childListsAreReorderedByDrops()
{
    QVERIFY(exec("INSERT INTO reading_list_folder (id, parent_id, name, ordering) VALUES (1, NULL, 'DC', 0)"));
    QVERIFY(exec("UPDATE reading_list SET folder_id = 1 WHERE id = 1"));

    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    const auto events = [&] { return find(model, "Events", find(model, "DC")); };

    model.addReadingListAt("Second", events());
    model.addReadingListAt("Third", events());
    QCOMPARE(names(model, events()), QStringList({ "First", "Second", "Third" }));

    auto data = rowData(model, find(model, "First", events()));
    QVERIFY(model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Third", events())));
    QVERIFY(model.dropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Third", events())));
    QCOMPARE(names(model, events()), QStringList({ "Second", "Third", "First" }));
    QCOMPARE(value("SELECT ordering FROM reading_list WHERE name = 'First'"), QString("2"));

    QVERIFY(model.dropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Second", events())));
    QCOMPARE(names(model, events()), QStringList({ "First", "Second", "Third" }));
    QCOMPARE(value("SELECT ordering FROM reading_list WHERE name = 'Third'"), QString("2"));

    // not next to a list of another parent, nor out of its parent
    model.addReadingList("Other");
    model.addReadingListAt("Child", find(model, "Other"));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Child", find(model, "Other"))));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Other")));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "First", events())));
    QVERIFY(!model.dropMimeData(data.get(), Qt::CopyAction, -1, -1, find(model, "Other")));
    QCOMPARE(names(model, events()), QStringList({ "First", "Second", "Third" }));
}

void ListModelTest::rowsDroppedBetweenRowsGoNextToThem()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    // the base view drops a dragged row only when the model names its format
    QCOMPARE(model.mimeTypes(), QStringList({ YACReader::YACReaderLibraryListRowMimeDataFormat }));
    // and only between top level rows and on empty space when the root takes drops
    QCOMPARE(model.flags(QModelIndex()), Qt::ItemIsDropEnabled);

    model.addListFolder(ListFolderKind::ReadingList, QModelIndex(), "DC");
    QVERIFY(model.moveToFolder(find(model, "Events"), find(model, "DC")));
    // the top level of the section has only a folder
    QCOMPARE(names(model).mid(8), QStringList({ "DC" }));

    // above or below the folder row: next to it, at the top level
    auto data = rowData(model, find(model, "Events", find(model, "DC")));
    const auto dcRow = find(model, "DC").row();
    QVERIFY(model.canDropMimeData(data.get(), Qt::CopyAction, dcRow, 0, QModelIndex()));
    QVERIFY(model.dropMimeData(data.get(), Qt::CopyAction, dcRow + 1, 0, QModelIndex()));
    QCOMPARE(names(model).mid(8), QStringList({ "DC", "Events" }));
    QCOMPARE(value("SELECT folder_id IS NULL FROM reading_list WHERE id = 1"), QString("1"));

    // between the rows of a folder: in that folder
    model.addReadingList("Zero Hour", find(model, "DC"));
    data = rowData(model, find(model, "Events"));
    QVERIFY(model.dropMimeData(data.get(), Qt::CopyAction, 1, 0, find(model, "DC")));
    QCOMPARE(names(model, find(model, "DC")), QStringList({ "Events", "Zero Hour" }));
    // between the rows of its own folder: it is there already
    data = rowData(model, find(model, "Zero Hour", find(model, "DC")));
    QVERIFY(!model.canDropMimeData(data.get(), Qt::CopyAction, 0, 0, find(model, "DC")));

    // a child list dropped between rows of its parent takes the row after the gap, else the one before it
    const auto events = [&] { return find(model, "Events", find(model, "DC")); };
    model.addReadingListAt("Second", events());
    data = rowData(model, find(model, "First", events()));
    QVERIFY(model.dropMimeData(data.get(), Qt::CopyAction, 2, 0, events()));
    QCOMPARE(names(model, events()), QStringList({ "Second", "First" }));
}

QTEST_MAIN(ListModelTest)

#include "main.moc"
