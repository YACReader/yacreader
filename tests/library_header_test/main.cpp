#include "header_model.h"
#include "library_header.h"
#include "view_selector.h"
#include "window_frame.h"
#include "yacreader_search_line_edit.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QToolButton>

#include <memory>

// The frame used where the system draws the title bar.
class ConventionalFrame : public WindowFrame
{
public:
    ConventionalFrame(QMainWindow *window, LibraryHeader *header)
        : WindowFrame(window, header) { }
};

class LibraryHeaderTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void searchDoesNotMoveWithTheFolderName();
    void folderNameStopsBeforeTheSearch();
    void searchTextFromTheWindowIsNotReportedBack();
    void sameSearchTextDoesNotRunTheSearchAgain();
    void searchTextFromTheUserRunsTheSearch();
    void searchStateFollowsTheModel();
    void headerFollowsTheModelEnabledState();
    void selectorFollowsTheCheckedAction();
    void selectorSegmentTriggersItsAction();
    void systemTitleBarHidesTheAppIdentity();
    void hitTestSeparatesControlsFromCaption();
    void trailingInsetKeepsControlsOutOfTheCaptionButtons();

private:
    std::unique_ptr<QObject> owner;
    QActionGroup *viewGroup = nullptr;
    HeaderActions actions;
    HeaderModel *model = nullptr;
    LibraryHeader *header = nullptr;
    ContentHeader *content = nullptr;

    void showContent(int width);
};

void LibraryHeaderTest::init()
{
    owner = std::make_unique<QObject>();
    const auto action = [this](const QString &text) { return new QAction(text, owner.get()); };
    actions.back = action("Back");
    actions.forward = action("Forward");
    actions.settings = action("Settings");
    actions.server = action("Server");
    actions.help = action("Help");
    actions.fullscreen = action("Full screen");
    viewGroup = new QActionGroup(owner.get());
    actions.gridView = new QAction("Grid", viewGroup);
    actions.flowView = new QAction("Flow", viewGroup);
    actions.infoView = new QAction("Info", viewGroup);
    for (QAction *view : { actions.gridView, actions.flowView, actions.infoView })
        view->setCheckable(true);
    actions.gridView->setChecked(true);

    model = new HeaderModel(actions, owner.get());
    header = new LibraryHeader(model, owner.get());
    content = header->contentPart();
}

void LibraryHeaderTest::cleanup()
{
    delete header->sidebarPart();
    delete header->contentPart();
    owner.reset();
}

void LibraryHeaderTest::showContent(int width)
{
    content->resize(width, LibraryHeader::Height);
    content->show();
    QVERIFY(QTest::qWaitForWindowExposed(content));
}

void LibraryHeaderTest::searchDoesNotMoveWithTheFolderName()
{
    showContent(900);
    model->setFolderName("A");
    const QRect searchGeometry = content->searchField()->geometry();

    model->setFolderName("A much longer folder name that does not fit before the search field at all");
    QCOMPARE(content->searchField()->geometry(), searchGeometry);

    model->setFolderName(QString());
    QCOMPARE(content->searchField()->geometry(), searchGeometry);
}

void LibraryHeaderTest::folderNameStopsBeforeTheSearch()
{
    showContent(700);
    const QString name = "A much longer folder name that does not fit before the search field at all";
    model->setFolderName(name);

    const QLabel *label = content->folderNameLabel();
    QVERIFY(label->geometry().right() < content->searchField()->geometry().left());
    QVERIFY(label->text() != name);
    QVERIFY(!label->text().isEmpty());
}

void LibraryHeaderTest::searchTextFromTheWindowIsNotReportedBack()
{
    QSignalSpy searches(model, &HeaderModel::searchChanged);

    model->setSearchText("batman", false);
    QCOMPARE(content->searchField()->text(), QString("batman"));
    QCOMPARE(model->searchText(), QString("batman"));
    QCOMPARE(searches.count(), 0);

    model->setSearchText("robin");
    QCOMPARE(content->searchField()->text(), QString("robin"));
    QCOMPARE(searches.count(), 1);
    QCOMPARE(searches.takeFirst().at(0).toString(), QString("robin"));
}

void LibraryHeaderTest::sameSearchTextDoesNotRunTheSearchAgain()
{
    QSignalSpy searches(model, &HeaderModel::searchChanged);

    model->setSearchText(QString());
    QCOMPARE(searches.count(), 0);

    model->setSearchText("batman");
    model->setSearchText("batman");
    QCOMPARE(searches.count(), 1);
}

void LibraryHeaderTest::searchTextFromTheUserRunsTheSearch()
{
    showContent(900);
    QSignalSpy searches(model, &HeaderModel::searchChanged);

    content->searchField()->setFocus();
    QTest::keyClicks(content->searchField(), "ab");

    QCOMPARE(searches.count(), 2);
    QCOMPARE(searches.last().at(0).toString(), QString("ab"));
    QCOMPARE(model->searchText(), QString("ab"));
}

void LibraryHeaderTest::searchStateFollowsTheModel()
{
    model->setSearchEnabled(false);
    QVERIFY(!content->searchField()->isEnabled());
    model->setSearchEnabled(true);
    QVERIFY(content->searchField()->isEnabled());
}

void LibraryHeaderTest::headerFollowsTheModelEnabledState()
{
    model->setEnabled(false);
    QVERIFY(!content->isEnabled());
    QVERIFY(!header->sidebarPart()->isEnabled());
    model->setEnabled(true);
    QVERIFY(content->isEnabled());
    QVERIFY(header->sidebarPart()->isEnabled());
}

void LibraryHeaderTest::selectorFollowsTheCheckedAction()
{
    const auto selector = content->findChild<ViewSelector *>();
    QVERIFY(selector != nullptr);
    const auto segments = selector->segments();
    QCOMPARE(segments.size(), 3);

    QVERIFY(segments[0]->isChecked());
    actions.infoView->setChecked(true);
    QVERIFY(!segments[0]->isChecked());
    QVERIFY(!segments[1]->isChecked());
    QVERIFY(segments[2]->isChecked());
}

void LibraryHeaderTest::selectorSegmentTriggersItsAction()
{
    showContent(900);
    const auto segments = content->findChild<ViewSelector *>()->segments();
    QSignalSpy flowTriggered(actions.flowView, &QAction::triggered);

    QTest::mouseClick(segments[1], Qt::LeftButton);

    QCOMPARE(flowTriggered.count(), 1);
    QVERIFY(actions.flowView->isChecked());
    QVERIFY(!actions.gridView->isChecked());
}

void LibraryHeaderTest::systemTitleBarHidesTheAppIdentity()
{
#ifdef Q_OS_MACOS
    QSKIP("The macOS header has no app identity");
#endif
    QMainWindow window;
    auto *sidebar = header->sidebarPart();
    sidebar->setParent(&window);
    window.setCentralWidget(sidebar);
    // The window must not delete the header part when a check fails.
    const auto restore = qScopeGuard([&window, sidebar] {
        window.takeCentralWidget();
        sidebar->setParent(nullptr);
    });
    const auto labels = [sidebar] { return sidebar->findChildren<QLabel *>(); };
    QCOMPARE(labels().size(), 2);

    ConventionalFrame frame(&window, header);
    window.resize(300, 300);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    for (QLabel *label : labels())
        QVERIFY(!label->isVisible());
    // The buttons take the place of the identity.
    const auto buttons = sidebar->findChildren<QToolButton *>();
    QVERIFY(!buttons.isEmpty());
    for (QToolButton *button : buttons)
        QVERIFY(button->geometry().right() < sidebar->width() / 2);
}

void LibraryHeaderTest::hitTestSeparatesControlsFromCaption()
{
    QWidget window;
    content->setParent(&window);
    const auto restore = qScopeGuard([this] { content->setParent(nullptr); });
    content->setGeometry(0, 0, 900, LibraryHeader::Height);
    window.resize(900, 300);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QRect search = content->searchField()->geometry();
    QCOMPARE(header->hitTest(search.center()), LibraryHeader::Hit::Client);

    const auto selector = content->findChild<ViewSelector *>();
    QCOMPARE(header->hitTest(selector->geometry().center()), LibraryHeader::Hit::Client);

    // Empty space between the forward button and the search drags the window.
    QCOMPARE(header->hitTest(QPoint(search.left() - 4, LibraryHeader::Height / 2)), LibraryHeader::Hit::Caption);
    // The folder name is not interactive.
    model->setFolderName("Comics");
    QCOMPARE(header->hitTest(content->folderNameLabel()->geometry().center()), LibraryHeader::Hit::Caption);

    QCOMPARE(header->hitTest(QPoint(450, 200)), LibraryHeader::Hit::Outside);
}

void LibraryHeaderTest::trailingInsetKeepsControlsOutOfTheCaptionButtons()
{
    showContent(900);
    header->setTrailingInset(138);
    for (QWidget *control : content->findChildren<QToolButton *>(Qt::FindDirectChildrenOnly))
        QVERIFY(control->geometry().right() < 900 - 138);
    QVERIFY(content->findChild<ViewSelector *>()->geometry().right() < 900 - 138);
}

QTEST_MAIN(LibraryHeaderTest)
#include "main.moc"
