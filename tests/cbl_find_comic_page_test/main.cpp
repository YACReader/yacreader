#include "cbl_find_comic_page.h"
#include "data_base_management.h"
#include "yacreader_global.h"

#include <QApplication>
#include <QDir>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>
#include <QTreeView>

#include <memory>

using namespace YACReader;

class CblFindComicPageTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void aFolderLimitShowsAndSearchesOnlyThatFolder();

private:
    bool exec(const QString &sql);

    std::unique_ptr<QTemporaryDir> root;
    QString library;
};

void CblFindComicPageTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    library = QDir::cleanPath(root->filePath("library"));
    QVERIFY(QDir().mkpath(LibraryPaths::libraryDataPath(library)));

    QString name;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(library));
        name = db.connectionName();
        db.close();
    }
    QSqlDatabase::removeDatabase(name);

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (2, 1, 'Batman', '/Batman'), (3, 1, 'Robin', '/Robin')"));
    QVERIFY(exec("INSERT INTO comic_info (id, hash, series, number) VALUES (1, 'h1', 'Batman', '11'), (2, 'h2', 'Batman', '12'), (3, 'h3', 'Batman', '13')"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES "
                 "(1, 2, 1, 'Batman 011.cbz', '/Batman/Batman 011.cbz'), (2, 2, 2, 'Batman 012.cbz', '/Batman/Batman 012.cbz'), "
                 "(3, 3, 3, 'Batman 013.cbz', '/Robin/Batman 013.cbz')"));
}

void CblFindComicPageTest::cleanup()
{
    root.reset();
}

bool CblFindComicPageTest::exec(const QString &sql)
{
    bool ok = false;
    QString name;
    {
        auto db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(library));
        name = db.connectionName();
        QSqlQuery query(db);
        ok = query.exec(sql);
    }
    QSqlDatabase::removeDatabase(name);
    return ok;
}

void CblFindComicPageTest::aFolderLimitShowsAndSearchesOnlyThatFolder()
{
    std::optional<QList<CblLibraryFolder>> folders;
    QString name;
    {
        auto db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(library));
        name = db.connectionName();
        folders = CblFindComicPage::loadFolders(db);
    }
    QSqlDatabase::removeDatabase(name);
    QVERIFY(folders);

    CblFindComicPage page(library, CblFindComicPage::Mode::OneEntry);
    page.setFolders(*folders);
    page.limitToFolder(2);
    CblImportEntry entry;
    entry.book.series = "Batman";
    entry.book.number = "12";
    page.showEntry(entry, 0, 1, CblImportStatus::Missing, "/Robin", -1, 0, false, false);
    page.show();

    const auto views = page.findChildren<QTreeView *>();
    QTreeView *comicsView = nullptr;
    for (auto *view : views) {
        if (view->isVisible())
            comicsView = view;
        else
            QCOMPARE(view->model()->headerData(0, Qt::Horizontal).toString(), QString("Folders"));
    }
    QVERIFY(comicsView != nullptr);
    const auto fileNames = [comicsView] {
        QStringList names;
        for (int row = 0; row < comicsView->model()->rowCount(); row++)
            names.append(comicsView->model()->index(row, 0).data().toString());
        return names;
    };
    QCOMPARE(fileNames(), QStringList({ "Batman 011.cbz", "Batman 012.cbz" }));

    auto *search = page.findChild<QLineEdit *>();
    QCOMPARE(search->placeholderText(), QString("Search in this folder"));
    QTest::keyClicks(search, "Batman");
    QTRY_COMPARE(comicsView->model()->rowCount(), 2);
    QTest::keyClicks(search, " 013");
    QTRY_COMPARE(comicsView->model()->rowCount(), 0);
    search->clear();
    QCOMPARE(fileNames(), QStringList({ "Batman 011.cbz", "Batman 012.cbz" }));

    QList<ComicInfoId> chosen;
    connect(&page, &CblFindComicPage::comicChosen, this, [&chosen](const CblLibraryComic &comic) { chosen.append(comic.id); });
    comicsView->setCurrentIndex(comicsView->model()->index(1, 0));
    for (auto *button : page.findChildren<QPushButton *>()) {
        if (button->text() == "Use this comic")
            button->click();
    }
    QCOMPARE(chosen, QList<ComicInfoId>({ ComicInfoId(2) }));
}

QTEST_MAIN(CblFindComicPageTest)
#include "main.moc"
