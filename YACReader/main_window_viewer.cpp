#include "main_window_viewer.h"
#include "configuration.h"
#include "viewer.h"
#include "goto_dialog.h"
#include "custom_widgets.h"
#include "options_dialog.h"
#include "check_new_version.h"
#include "comic.h"
#include "bookmarks_dialog.h"
#include "shortcuts_dialog.h"
#include "width_slider.h"
#include "qnaturalsorting.h"
#include "help_about_dialog.h"
#include "yacreader_tool_bar_stretch.h"

#include "comic_db.h"
#include "yacreader_local_client.h"

#include "yacreader_global.h"
#include "edit_shortcuts_dialog.h"
#include "shortcuts_manager.h"

#include "whats_new_controller.h"

#include <ctime>
#include <algorithm>
#include <utility>

#include <QApplication>
#include <QCoreApplication>
#include <QToolButton>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QDate>
#include <QMenuBar>

/* TODO remove, no longer used
#ifdef Q_OS_MAC
class MacToolBarSeparator : public QWidget
{
public:
        MacToolBarSeparator(QWidget * parent =0)
                :QWidget(parent)
        {
                setFixedWidth(2);
        }

        void paintEvent(QPaintEvent *event)
        {
                Q_UNUSED(event);
                QPainter painter(this);

                QLinearGradient lG(0,0,0,height());

                lG.setColorAt(0,QColor(128,128,128,0));
                lG.setColorAt(0.5,QColor(128,128,128,255));
                lG.setColorAt(1,QColor(128,128,128,0));

                painter.fillRect(0,0,1,height(),lG);

                QLinearGradient lG2(1,0,1,height());

                lG2.setColorAt(0,QColor(220,220,220,0));
                lG2.setColorAt(0.5,QColor(220,220,220,255));
                lG2.setColorAt(1,QColor(220,220,220,0));

                painter.fillRect(1,0,1,height(),lG2);
        }
};
#endif*/

MainWindowViewer::MainWindowViewer()
    : QMainWindow(), fullscreen(false), toolbars(true), currentDirectory("."), currentDirectoryImgDest("."), isClient(false)
{
    loadConfiguration();
    setupUI();
    afterLaunchTasks();
}

void MainWindowViewer::afterLaunchTasks()
{
    WhatsNewController whatsNewController;

    whatsNewController.showWhatsNewIfNeeded(this);
}

MainWindowViewer::~MainWindowViewer()
{
    delete settings;
    delete viewer;
    delete had;

    // delete sliderAction;
    delete openAction;
    delete openFolderAction;
    delete openLatestComicAction;
    delete saveImageAction;
    delete openComicOnTheLeftAction;
    delete openComicOnTheRightAction;
    delete goToPageOnTheLeftAction;
    delete goToPageOnTheRightAction;
    delete adjustHeightAction;
    delete adjustWidthAction;
    delete leftRotationAction;
    delete rightRotationAction;
    delete doublePageAction;
    delete doubleMangaPageAction;
    delete increasePageZoomAction;
    delete decreasePageZoomAction;
    delete resetZoomAction;
    delete goToPageAction;
    delete optionsAction;
    delete helpAboutAction;
    delete showMagnifyingGlassAction;
    delete setBookmarkAction;
    delete showBookmarksAction;
    delete showShorcutsAction;
    delete showInfoAction;
    delete closeAction;
    delete showDictionaryAction;
    delete adjustToFullSizeAction;
    delete fitToPageAction;
    delete showFlowAction;
}
void MainWindowViewer::loadConfiguration()
{
    settings = new QSettings(YACReader::getSettingsPath() + "/YACReader.ini", QSettings::IniFormat);

    Configuration &config = Configuration::getConfiguration();
    config.load(settings);
    currentDirectory = config.getDefaultPath();
    fullscreen = config.getFullScreen();
}

void MainWindowViewer::setupUI()
{
    // setUnifiedTitleAndToolBarOnMac(true);

    viewer = new Viewer(this);
    connect(viewer, &Viewer::comicLoaded, this, [this] { setLoadedComicActionsEnabled(true); });
    connect(viewer, &Viewer::reset, this, &MainWindowViewer::processReset);
    // detected end of comic
    connect(viewer, &Viewer::openNextComic, this, &MainWindowViewer::openNextComic);
    // detected start of comic
    connect(viewer, &Viewer::openPreviousComic, this, &MainWindowViewer::openPreviousComic);

    setCentralWidget(viewer);
    QScreen *screen = window()->screen();
    if (screen == nullptr) {
        screen = QApplication::screens().constFirst();
    }

    int heightDesktopResolution = screen != nullptr ? screen->size().height() : 600;
    int widthDesktopResolution = screen != nullptr ? screen->size().height() : 1024;
    int height, width;
    height = static_cast<int>(heightDesktopResolution * 0.84);
    width = static_cast<int>(height * 0.70);
    Configuration &conf = Configuration::getConfiguration();
    if (!restoreGeometry(conf.getGeometry())) {
        move(QPoint((widthDesktopResolution - width) / 2, ((heightDesktopResolution - height) - 40) / 2));
        resize(QSize(width, height));
    }

    had = new HelpAboutDialog(this); // TODO load data

    had->loadAboutInformation(":/files/about.html");
    had->loadHelp(":/files/helpYACReader.html");

    optionsDialog = new OptionsDialog(this);
    connect(optionsDialog, &QDialog::accepted, viewer, &Viewer::updateOptions);
    connect(optionsDialog, &YACReaderOptionsDialog::optionsChanged, this, &MainWindowViewer::reloadOptions);
    connect(optionsDialog, &OptionsDialog::changedFilters, viewer, &Viewer::updateFilters);
    connect(optionsDialog, &OptionsDialog::changedImageOptions, viewer, &Viewer::updatePage);

    optionsDialog->restoreOptions(settings);
    // shortcutsDialog = new ShortcutsDialog(this);
    editShortcutsDialog = new EditShortcutsDialog(this);
    connect(optionsDialog, &YACReaderOptionsDialog::editShortcuts, editShortcutsDialog, &QWidget::show);

    createActions();
    setUpShortcutsManagement();
    disableActions();
    disablePreviousNextComicActions();

    createToolBars();

    setWindowTitle("YACReader");

    checkNewVersion();

    viewer->setFocusPolicy(Qt::StrongFocus);

    previousWindowFlags = windowFlags();
    previousPos = pos();
    previousSize = size();

    if (fullscreen)
        toFullScreen();
    if (conf.getMaximized())
        showMaximized();

    setAcceptDrops(true);

    if (Configuration::getConfiguration().getShowToolbars() && !Configuration::getConfiguration().getFullScreen())
        showToolBars();
    else
        hideToolBars();
}

void MainWindowViewer::createActions()
{
    openAction = new QAction(tr("&Open"), this);
    openAction->setIcon(QIcon(":/images/viewer_toolbar/open.png"));
    openAction->setToolTip(tr("Open a comic"));
    openAction->setData(OPEN_ACTION_Y);
    openAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_ACTION_Y));
    connect(openAction, &QAction::triggered, this, QOverload<>::of(&MainWindowViewer::open));

#ifdef Q_OS_MAC
    newInstanceAction = new QAction(tr("New instance"), this);
    newInstanceAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(NEW_INSTANCE_ACTION_Y));
    connect(newInstanceAction, &QAction::triggered,
            [=]() {
                QStringList possiblePaths { QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../../") };
                possiblePaths += QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

                for (auto &&ypath : possiblePaths) {
                    QString yacreaderPath = QDir::cleanPath(ypath + "/YACReader.app");
                    if (QFileInfo(yacreaderPath).exists()) {
                        QStringList parameters { "-n", "-a", yacreaderPath };
                        QProcess::startDetached("open", parameters);
                        break;
                    }
                }
            });
    newInstanceAction->setData(NEW_INSTANCE_ACTION_Y);
#endif

    openFolderAction = new QAction(tr("Open Folder"), this);
    openFolderAction->setIcon(QIcon(":/images/viewer_toolbar/openFolder.png"));
    openFolderAction->setToolTip(tr("Open image folder"));
    openFolderAction->setData(OPEN_FOLDER_ACTION_Y);
    openFolderAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_FOLDER_ACTION_Y));
    connect(openFolderAction, &QAction::triggered, this, &MainWindowViewer::openFolder);

    openLatestComicAction = new QAction(tr("Open latest comic"), this);
    openLatestComicAction->setToolTip(tr("Open the latest comic opened in the previous reading session"));
    openLatestComicAction->setData(OPEN_LATEST_COMIC_Y);
    openLatestComicAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_LATEST_COMIC_Y));
    connect(openLatestComicAction, &QAction::triggered, this, &MainWindowViewer::openLatestComic);

    QAction *recentFileAction = nullptr;
    // TODO: Replace limit with a configurable value
    for (int i = 0; i < Configuration::getConfiguration().getOpenRecentSize(); i++) {
        recentFileAction = new QAction(this);
        recentFileAction->setVisible(false);
        QObject::connect(recentFileAction, &QAction::triggered, this, &MainWindowViewer::openRecent);
        recentFilesActionList.append(recentFileAction);
    }

    clearRecentFilesAction = new QAction(tr("Clear"), this);
    clearRecentFilesAction->setToolTip(tr("Clear open recent list"));
    connect(clearRecentFilesAction, &QAction::triggered, this, &MainWindowViewer::clearRecentFiles);

    saveImageAction = new QAction(tr("Save"), this);
    saveImageAction->setIcon(QIcon(":/images/viewer_toolbar/save.png"));
    saveImageAction->setToolTip(tr("Save current page"));
    saveImageAction->setData(SAVE_IMAGE_ACTION_Y);
    saveImageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SAVE_IMAGE_ACTION_Y));
    connect(saveImageAction, &QAction::triggered, this, &MainWindowViewer::saveImage);

    openComicOnTheLeftAction = new QAction(tr("Previous Comic"), this);
    openComicOnTheLeftAction->setIcon(QIcon(":/images/viewer_toolbar/openPrevious.png"));
    openComicOnTheLeftAction->setToolTip(tr("Open previous comic"));
    openComicOnTheLeftAction->setData(OPEN_PREVIOUS_COMIC_ACTION_Y);
    openComicOnTheLeftAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_PREVIOUS_COMIC_ACTION_Y));
    connect(openComicOnTheLeftAction, &QAction::triggered, this, &MainWindowViewer::openLeftComic);

    openComicOnTheRightAction = new QAction(tr("Next Comic"), this);
    openComicOnTheRightAction->setIcon(QIcon(":/images/viewer_toolbar/openNext.png"));
    openComicOnTheRightAction->setToolTip(tr("Open next comic"));
    openComicOnTheRightAction->setData(OPEN_NEXT_COMIC_ACTION_Y);
    openComicOnTheRightAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_NEXT_COMIC_ACTION_Y));
    connect(openComicOnTheRightAction, &QAction::triggered, this, &MainWindowViewer::openRightComic);

    goToPageOnTheLeftAction = new QAction(tr("&Previous"), this);
    goToPageOnTheLeftAction->setIcon(QIcon(":/images/viewer_toolbar/previous.png"));
    goToPageOnTheLeftAction->setShortcutContext(Qt::WidgetShortcut);
    goToPageOnTheLeftAction->setToolTip(tr("Go to previous page"));
    goToPageOnTheLeftAction->setData(PREV_ACTION_Y);
    goToPageOnTheLeftAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(PREV_ACTION_Y));
    connect(goToPageOnTheLeftAction, &QAction::triggered, viewer, &Viewer::left);

    goToPageOnTheRightAction = new QAction(tr("&Next"), this);
    goToPageOnTheRightAction->setIcon(QIcon(":/images/viewer_toolbar/next.png"));
    goToPageOnTheRightAction->setShortcutContext(Qt::WidgetShortcut);
    goToPageOnTheRightAction->setToolTip(tr("Go to next page"));
    goToPageOnTheRightAction->setData(NEXT_ACTION_Y);
    goToPageOnTheRightAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(NEXT_ACTION_Y));
    connect(goToPageOnTheRightAction, &QAction::triggered, viewer, &Viewer::right);

    adjustHeightAction = new QAction(tr("Fit Height"), this);
    adjustHeightAction->setIcon(QIcon(":/images/viewer_toolbar/toHeight.png"));
    // adjustWidth->setCheckable(true);
    adjustHeightAction->setToolTip(tr("Fit image to height"));
    // adjustWidth->setIcon(QIcon(":/images/fitWidth.png"));
    adjustHeightAction->setData(ADJUST_HEIGHT_ACTION_Y);
    adjustHeightAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ADJUST_HEIGHT_ACTION_Y));
    adjustHeightAction->setCheckable(true);
    connect(adjustHeightAction, &QAction::triggered, this, &MainWindowViewer::fitToHeight);

    adjustWidthAction = new QAction(tr("Fit Width"), this);
    adjustWidthAction->setIcon(QIcon(":/images/viewer_toolbar/toWidth.png"));
    // adjustWidth->setCheckable(true);
    adjustWidthAction->setToolTip(tr("Fit image to width"));
    // adjustWidth->setIcon(QIcon(":/images/fitWidth.png"));
    adjustWidthAction->setData(ADJUST_WIDTH_ACTION_Y);
    adjustWidthAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ADJUST_WIDTH_ACTION_Y));
    adjustWidthAction->setCheckable(true);
    connect(adjustWidthAction, &QAction::triggered, this, &MainWindowViewer::fitToWidth);

    adjustToFullSizeAction = new QAction(tr("Show full size"), this);
    adjustToFullSizeAction->setIcon(QIcon(":/images/viewer_toolbar/full.png"));
    adjustToFullSizeAction->setCheckable(false);
    adjustToFullSizeAction->setData(ADJUST_TO_FULL_SIZE_ACTION_Y);
    adjustToFullSizeAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ADJUST_TO_FULL_SIZE_ACTION_Y));
    adjustToFullSizeAction->setCheckable(true);
    connect(adjustToFullSizeAction, &QAction::triggered, this, &MainWindowViewer::adjustToFullSizeSwitch);

    fitToPageAction = new QAction(tr("Fit to page"), this);
    fitToPageAction->setIcon(QIcon(":/images/viewer_toolbar/fitToPage.png"));
    fitToPageAction->setData(FIT_TO_PAGE_ACTION_Y);
    fitToPageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(FIT_TO_PAGE_ACTION_Y));
    fitToPageAction->setCheckable(true);
    connect(fitToPageAction, &QAction::triggered, this, &MainWindowViewer::fitToPageSwitch);

    // fit modes have to be exclusive and checkable
    auto fitModes = new QActionGroup(this);
    fitModes->addAction(adjustHeightAction);
    fitModes->addAction(adjustWidthAction);
    fitModes->addAction(adjustToFullSizeAction);
    fitModes->addAction(fitToPageAction);

    switch (Configuration::getConfiguration().getFitMode()) {
    case YACReader::FitMode::ToWidth:
        adjustWidthAction->setChecked(true);
        break;
    case YACReader::FitMode::ToHeight:
        adjustHeightAction->setChecked(true);
        break;
    case YACReader::FitMode::FullRes:
        adjustToFullSizeAction->setChecked(true);
        break;
    case YACReader::FitMode::FullPage:
        fitToPageAction->setChecked(true);
        break;
    default:
        fitToPageAction->setChecked(true);
    }

    resetZoomAction = new QAction(tr("Reset zoom"), this);
    resetZoomAction->setData(RESET_ZOOM_ACTION_Y);
    resetZoomAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(RESET_ZOOM_ACTION_Y));
    connect(resetZoomAction, &QAction::triggered, this, &MainWindowViewer::resetZoomLevel);

    showZoomSliderlAction = new QAction(tr("Show zoom slider"), this);
    showZoomSliderlAction->setIcon(QIcon(":/images/viewer_toolbar/zoom.png"));

    increasePageZoomAction = new QAction(tr("Zoom+"), this);
    increasePageZoomAction->setData(ZOOM_PLUS_ACTION_Y);
    increasePageZoomAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ZOOM_PLUS_ACTION_Y));
    connect(increasePageZoomAction, &QAction::triggered, this, &MainWindowViewer::increasePageZoomLevel);

    decreasePageZoomAction = new QAction(tr("Zoom-"), this);
    decreasePageZoomAction->setData(ZOOM_MINUS_ACTION_Y);
    decreasePageZoomAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ZOOM_MINUS_ACTION_Y));
    connect(decreasePageZoomAction, &QAction::triggered, this, &MainWindowViewer::decreasePageZoomLevel);

    leftRotationAction = new QAction(tr("Rotate image to the left"), this);
    leftRotationAction->setIcon(QIcon(":/images/viewer_toolbar/rotateL.png"));
    leftRotationAction->setData(LEFT_ROTATION_ACTION_Y);
    leftRotationAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(LEFT_ROTATION_ACTION_Y));
    connect(leftRotationAction, &QAction::triggered, viewer, &Viewer::rotateLeft);

    rightRotationAction = new QAction(tr("Rotate image to the right"), this);
    rightRotationAction->setIcon(QIcon(":/images/viewer_toolbar/rotateR.png"));
    rightRotationAction->setData(RIGHT_ROTATION_ACTION_Y);
    rightRotationAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(RIGHT_ROTATION_ACTION_Y));
    connect(rightRotationAction, &QAction::triggered, viewer, &Viewer::rotateRight);

    doublePageAction = new QAction(tr("Double page mode"), this);
    doublePageAction->setToolTip(tr("Switch to double page mode"));
    doublePageAction->setIcon(QIcon(":/images/viewer_toolbar/doublePage.png"));
    doublePageAction->setCheckable(true);
    doublePageAction->setChecked(Configuration::getConfiguration().getDoublePage());
    doublePageAction->setData(DOUBLE_PAGE_ACTION_Y);
    doublePageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(DOUBLE_PAGE_ACTION_Y));
    connect(doublePageAction, &QAction::triggered, viewer, &Viewer::doublePageSwitch);

    // inversed pictures mode
    doubleMangaPageAction = new QAction(tr("Double page manga mode"), this);
    doubleMangaPageAction->setToolTip(tr("Reverse reading order in double page mode"));
    doubleMangaPageAction->setIcon(QIcon(":/images/viewer_toolbar/doubleMangaPage.png"));
    doubleMangaPageAction->setCheckable(true);
    doubleMangaPageAction->setChecked(Configuration::getConfiguration().getDoubleMangaPage());
    doubleMangaPageAction->setData(DOUBLE_MANGA_PAGE_ACTION_Y);
    doubleMangaPageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(DOUBLE_MANGA_PAGE_ACTION_Y));
    connect(doubleMangaPageAction, &QAction::triggered, viewer, &Viewer::doubleMangaPageSwitch);
    connect(doubleMangaPageAction, &QAction::triggered, this, &MainWindowViewer::doubleMangaPageSwitch);

    goToPageAction = new QAction(tr("Go To"), this);
    goToPageAction->setIcon(QIcon(":/images/viewer_toolbar/goto.png"));
    goToPageAction->setToolTip(tr("Go to page ..."));
    goToPageAction->setData(GO_TO_PAGE_ACTION_Y);
    goToPageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(GO_TO_PAGE_ACTION_Y));
    connect(goToPageAction, &QAction::triggered, viewer, &Viewer::showGoToDialog);

    optionsAction = new QAction(tr("Options"), this);
    optionsAction->setToolTip(tr("YACReader options"));
    optionsAction->setData(OPTIONS_ACTION_Y);
    optionsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPTIONS_ACTION_Y));
    optionsAction->setIcon(QIcon(":/images/viewer_toolbar/options.png"));

    connect(optionsAction, &QAction::triggered, optionsDialog, &OptionsDialog::show);

    helpAboutAction = new QAction(tr("Help"), this);
    helpAboutAction->setToolTip(tr("Help, About YACReader"));
    helpAboutAction->setIcon(QIcon(":/images/viewer_toolbar/help.png"));
    helpAboutAction->setData(HELP_ABOUT_ACTION_Y);
    helpAboutAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(HELP_ABOUT_ACTION_Y));
    connect(helpAboutAction, &QAction::triggered, had, &QWidget::show);

    showMagnifyingGlassAction = new QAction(tr("Magnifying glass"), this);
    showMagnifyingGlassAction->setToolTip(tr("Switch Magnifying glass"));
    showMagnifyingGlassAction->setIcon(QIcon(":/images/viewer_toolbar/magnifyingGlass.png"));
    showMagnifyingGlassAction->setCheckable(true);
    showMagnifyingGlassAction->setData(SHOW_MAGNIFYING_GLASS_ACTION_Y);
    showMagnifyingGlassAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_MAGNIFYING_GLASS_ACTION_Y));
    connect(showMagnifyingGlassAction, &QAction::triggered, viewer, &Viewer::magnifyingGlassSwitch);

    setBookmarkAction = new QAction(tr("Set bookmark"), this);
    setBookmarkAction->setToolTip(tr("Set a bookmark on the current page"));
    setBookmarkAction->setIcon(QIcon(":/images/viewer_toolbar/bookmark.png"));
    setBookmarkAction->setCheckable(true);
    setBookmarkAction->setData(SET_BOOKMARK_ACTION_Y);
    setBookmarkAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_BOOKMARK_ACTION_Y));
    connect(setBookmarkAction, &QAction::triggered, viewer, &Viewer::setBookmark);
    connect(viewer, &Viewer::pageAvailable, setBookmarkAction, &QAction::setEnabled);
    connect(viewer, &Viewer::pageIsBookmark, setBookmarkAction, &QAction::setChecked);

    showBookmarksAction = new QAction(tr("Show bookmarks"), this);
    showBookmarksAction->setToolTip(tr("Show the bookmarks of the current comic"));
    showBookmarksAction->setIcon(QIcon(":/images/viewer_toolbar/showBookmarks.png"));
    showBookmarksAction->setData(SHOW_BOOKMARKS_ACTION_Y);
    showBookmarksAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_BOOKMARKS_ACTION_Y));
    connect(showBookmarksAction, &QAction::triggered, viewer->getBookmarksDialog(), &QWidget::show);

    showShorcutsAction = new QAction(tr("Show keyboard shortcuts"), this);
    showShorcutsAction->setIcon(QIcon(":/images/viewer_toolbar/shortcuts.png"));
    showShorcutsAction->setData(SHOW_SHORCUTS_ACTION_Y);
    showShorcutsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_SHORCUTS_ACTION_Y));
    // connect(showShorcutsAction, SIGNAL(triggered()),shortcutsDialog,SLOT(show()));
    connect(showShorcutsAction, &QAction::triggered, editShortcutsDialog, &QWidget::show);

    showInfoAction = new QAction(tr("Show Info"), this);
    showInfoAction->setIcon(QIcon(":/images/viewer_toolbar/info.png"));
    showInfoAction->setData(SHOW_INFO_ACTION_Y);
    showInfoAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_INFO_ACTION_Y));
    connect(showInfoAction, &QAction::triggered, viewer, &Viewer::informationSwitch);

    closeAction = new QAction(tr("Close"), this);
    closeAction->setIcon(QIcon(":/images/viewer_toolbar/close.png"));
    closeAction->setData(CLOSE_ACTION_Y);
    closeAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(CLOSE_ACTION_Y));
    connect(closeAction, &QAction::triggered, this, &QWidget::close);

    showDictionaryAction = new QAction(tr("Show Dictionary"), this);
    showDictionaryAction->setIcon(QIcon(":/images/viewer_toolbar/translator.png"));
    // showDictionaryAction->setCheckable(true);
    showDictionaryAction->setData(SHOW_DICTIONARY_ACTION_Y);
    showDictionaryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_DICTIONARY_ACTION_Y));
    connect(showDictionaryAction, &QAction::triggered, viewer, &Viewer::translatorSwitch);

    showFlowAction = new QAction(tr("Show go to flow"), this);
    showFlowAction->setIcon(QIcon(":/images/viewer_toolbar/flow.png"));
    showFlowAction->setData(SHOW_FLOW_ACTION_Y);
    showFlowAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_FLOW_ACTION_Y));
    connect(showFlowAction, &QAction::triggered, viewer, &Viewer::goToFlowSwitch);

    showEditShortcutsAction = new QAction(tr("Edit shortcuts"), this);
    showEditShortcutsAction->setData(SHOW_EDIT_SHORTCUTS_ACTION_Y);
    showEditShortcutsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_EDIT_SHORTCUTS_ACTION_Y));
    connect(showEditShortcutsAction, &QAction::triggered, editShortcutsDialog, &QWidget::show);
}

void MainWindowViewer::createToolBars()
{
#ifdef Q_OS_MAC
    comicToolBar = new YACReaderMacOSXToolbar(this);
#else
    comicToolBar = addToolBar(tr("&File"));
#endif

#ifdef Q_OS_MAC
    // comicToolBar->setIconSize(QSize(16,16));
#else
    comicToolBar->setIconSize(QSize(18, 18));
    comicToolBar->setStyleSheet("QToolBar{border:none;}");
#endif

#ifdef Q_OS_MAC
    comicToolBar->addAction(openAction);
    comicToolBar->addAction(openFolderAction);
#else
    auto recentmenu = new QMenu(tr("Open recent"));
    recentmenu->addActions(recentFilesActionList);
    recentmenu->addSeparator();
    recentmenu->addAction(clearRecentFilesAction);
    refreshRecentFilesActionList();

    auto tb = new QToolButton();
    tb->addAction(openAction);
    tb->addAction(openLatestComicAction);
    tb->addAction(openFolderAction);
    tb->addAction(recentmenu->menuAction());
    tb->setPopupMode(QToolButton::MenuButtonPopup);
    tb->setDefaultAction(openAction);

    comicToolBar->addWidget(tb);
#endif

    comicToolBar->addAction(saveImageAction);
    comicToolBar->addAction(openComicOnTheLeftAction);
    comicToolBar->addAction(openComicOnTheRightAction);

    comicToolBar->addSeparator();

    comicToolBar->addAction(goToPageOnTheLeftAction);
    comicToolBar->addAction(goToPageOnTheRightAction);
    comicToolBar->addAction(goToPageAction);

    comicToolBar->addSeparator();

    comicToolBar->addAction(adjustWidthAction);
    comicToolBar->addAction(adjustHeightAction);
    comicToolBar->addAction(adjustToFullSizeAction);
    comicToolBar->addAction(fitToPageAction);

    zoomSliderAction = new YACReaderSlider(this);
    zoomSliderAction->hide();

    comicToolBar->addAction(showZoomSliderlAction);

    connect(showZoomSliderlAction, &QAction::triggered, this, &MainWindowViewer::toggleFitToWidthSlider);
    connect(zoomSliderAction, &YACReaderSlider::zoomRatioChanged, viewer, &Viewer::updateZoomRatio);
    connect(viewer, &Viewer::zoomUpdated, zoomSliderAction, &YACReaderSlider::updateZoomRatio);

    comicToolBar->addAction(leftRotationAction);
    comicToolBar->addAction(rightRotationAction);
    comicToolBar->addAction(doublePageAction);
    comicToolBar->addAction(doubleMangaPageAction);

    comicToolBar->addSeparator();

    comicToolBar->addAction(showMagnifyingGlassAction);

    comicToolBar->addSeparator();

    comicToolBar->addAction(setBookmarkAction);
    comicToolBar->addAction(showBookmarksAction);

    comicToolBar->addSeparator();

    comicToolBar->addAction(showDictionaryAction);
    comicToolBar->addAction(showFlowAction);
    comicToolBar->addAction(showInfoAction);

#ifdef Q_OS_MAC
    comicToolBar->addStretch();
#else
    comicToolBar->addWidget(new YACReaderToolBarStretch());
#endif

    comicToolBar->addAction(showShorcutsAction);
    comicToolBar->addAction(optionsAction);
    comicToolBar->addAction(helpAboutAction);
    // comicToolBar->addAction(closeAction);

#ifndef Q_OS_MAC
    comicToolBar->setMovable(false);
#endif

    viewer->addAction(openAction);
    viewer->addAction(openFolderAction);
    viewer->addAction(saveImageAction);
    viewer->addAction(openComicOnTheLeftAction);
    viewer->addAction(openComicOnTheRightAction);
    YACReader::addSperator(viewer);

    viewer->addAction(goToPageOnTheLeftAction);
    viewer->addAction(goToPageOnTheRightAction);
    viewer->addAction(goToPageAction);
    viewer->addAction(adjustHeightAction);
    viewer->addAction(adjustWidthAction);
    viewer->addAction(adjustToFullSizeAction);
    viewer->addAction(fitToPageAction);
    viewer->addAction(leftRotationAction);
    viewer->addAction(rightRotationAction);
    viewer->addAction(doublePageAction);
    viewer->addAction(doubleMangaPageAction);
    YACReader::addSperator(viewer);

    viewer->addAction(showMagnifyingGlassAction);
    viewer->addAction(increasePageZoomAction);
    viewer->addAction(decreasePageZoomAction);
    viewer->addAction(resetZoomAction);
    YACReader::addSperator(viewer);

    viewer->addAction(setBookmarkAction);
    viewer->addAction(showBookmarksAction);
    YACReader::addSperator(viewer);

    viewer->addAction(showDictionaryAction);
    viewer->addAction(showFlowAction);
    viewer->addAction(showInfoAction);
    YACReader::addSperator(viewer);

    viewer->addAction(showShorcutsAction);
    viewer->addAction(showEditShortcutsAction);
    viewer->addAction(optionsAction);
    viewer->addAction(helpAboutAction);
    YACReader::addSperator(viewer);

    viewer->addAction(closeAction);

    viewer->setContextMenuPolicy(Qt::ActionsContextMenu);

    // MacOSX app menus
#ifdef Q_OS_MAC
    auto menuBar = this->menuBar();
    // about / preferences
    // TODO

    // file
    auto fileMenu = new QMenu(tr("File"));

    fileMenu->addAction(newInstanceAction);
    fileMenu->addSeparator();
    fileMenu->addAction(openAction);
    fileMenu->addAction(openLatestComicAction);
    fileMenu->addAction(openFolderAction);
    fileMenu->addSeparator();
    fileMenu->addAction(saveImageAction);
    fileMenu->addSeparator();

    auto recentmenu = new QMenu(tr("Open recent"));
    recentmenu->addActions(recentFilesActionList);
    recentmenu->addSeparator();
    recentmenu->addAction(clearRecentFilesAction);
    refreshRecentFilesActionList();
    fileMenu->addMenu(recentmenu);

    fileMenu->addSeparator();
    fileMenu->addAction(closeAction);

    auto editMenu = new QMenu(tr("Edit"));
    editMenu->addAction(leftRotationAction);
    editMenu->addAction(rightRotationAction);

    auto viewMenu = new QMenu(tr("View"));
    viewMenu->addAction(adjustHeightAction);
    viewMenu->addAction(adjustWidthAction);
    viewMenu->addAction(fitToPageAction);
    viewMenu->addAction(adjustToFullSizeAction);
    viewMenu->addSeparator();
    viewMenu->addAction(increasePageZoomAction);
    viewMenu->addAction(decreasePageZoomAction);
    viewMenu->addAction(resetZoomAction);
    viewMenu->addAction(showZoomSliderlAction);
    viewMenu->addSeparator();
    viewMenu->addAction(doublePageAction);
    viewMenu->addAction(doubleMangaPageAction);
    viewMenu->addSeparator();
    viewMenu->addAction(showMagnifyingGlassAction);

    auto goMenu = new QMenu(tr("Go"));
    goMenu->addAction(goToPageOnTheLeftAction);
    goMenu->addAction(goToPageOnTheRightAction);
    goMenu->addAction(goToPageAction);
    goMenu->addSeparator();
    goMenu->addAction(setBookmarkAction);
    goMenu->addAction(showBookmarksAction);

    auto windowMenu = new QMenu(tr("Window"));
    windowMenu->addAction(optionsAction); // this action goes to MacOS's Preference menu by Qt
    windowMenu->addAction(showShorcutsAction);
    windowMenu->addAction(showFlowAction);
    windowMenu->addAction(showInfoAction);
    windowMenu->addAction(showDictionaryAction);

    auto helpMenu = new QMenu(tr("Help"));
    helpMenu->addAction(helpAboutAction);

    menuBar->addMenu(fileMenu);
    menuBar->addMenu(editMenu);
    menuBar->addMenu(viewMenu);
    menuBar->addMenu(goMenu);
    menuBar->addMenu(windowMenu);
    menuBar->addMenu(helpMenu);

    // tool bar
    // QMenu * toolbarMenu = new QMenu(tr("Toolbar"));
    // toolbarMenu->addAction();
    // TODO

    // menu->addMenu(toolbarMenu);

    // attach toolbar

    comicToolBar->attachToWindow(this->windowHandle());

#endif
}

void MainWindowViewer::refreshRecentFilesActionList()
{
    QStringList recentFilePaths = Configuration::getConfiguration().openRecentList();

    // TODO: Replace limit with something configurable
    int iteration = (recentFilePaths.size() < Configuration::getConfiguration().getOpenRecentSize())
            ? recentFilePaths.size()
            : Configuration::getConfiguration().getOpenRecentSize();
    for (int i = 0; i < iteration; i++) {
        QString strippedName = QFileInfo(recentFilePaths.at(i)).fileName();
        recentFilesActionList.at(i)->setText(strippedName);
        recentFilesActionList.at(i)->setData(recentFilePaths.at(i));
        recentFilesActionList.at(i)->setVisible(true);
    }

    for (int i = iteration; i < Configuration::getConfiguration().getOpenRecentSize(); i++) {
        recentFilesActionList.at(i)->setVisible(false);
    }
}

void MainWindowViewer::clearRecentFiles()
{
    Configuration::getConfiguration().clearOpenRecentList();
    refreshRecentFilesActionList();
}

void MainWindowViewer::openRecent()
{
    auto action = qobject_cast<QAction *>(sender());

    openComicFromRecentAction(action);
}

void MainWindowViewer::openLatestComic()
{
    if (recentFilesActionList.isEmpty()) {
        return;
    }

    openComicFromRecentAction(recentFilesActionList[0]);
}

void MainWindowViewer::openComicFromRecentAction(QAction *action)
{
    if (action == nullptr) {
        return;
    }

    QFileInfo info1(action->data().toString());
    if (info1.exists()) {
        if (info1.isFile()) {
            openComicFromPath(action->data().toString());
        } else if (info1.isDir()) {
            openFolderFromPath(action->data().toString());
        }
    } else {
        viewer->resetContent();
        viewer->showMessageErrorOpening();
    }
}

void MainWindowViewer::reloadOptions()
{
    viewer->updateConfig(settings);
}

void MainWindowViewer::open()
{
    QFileDialog openDialog;
#ifndef use_unarr
    QString pathFile = openDialog.getOpenFileName(this, tr("Open Comic"), currentDirectory, tr("Comic files") + "(*.cbr *.cbz *.rar *.zip *.tar *.pdf *.7z *.cb7 *.arj *.cbt)");
#else
    QString pathFile = openDialog.getOpenFileName(this, tr("Open Comic"), currentDirectory, tr("Comic files") + "(*.cbr *.cbz *.rar *.zip *.tar *.pdf *.cbt)");
#endif
    if (!pathFile.isEmpty()) {
        openComicFromPath(pathFile);
    }
}

void MainWindowViewer::open(QString path, ComicDB &comic, QList<ComicDB> &siblings)
{
    QFileInfo fi(path);

    if (!comic.info.title.isNull() && !comic.info.title.toString().isEmpty())
        setWindowTitle("YACReader - " + comic.info.title.toString());
    else
        setWindowTitle("YACReader - " + fi.fileName());

    viewer->setMangaWithoutStoringSetting(comic.info.manga.toBool());
    doubleMangaPageAction->setChecked(comic.info.manga.toBool());

    viewer->open(path, comic);
    enableActions();
    int index = siblings.indexOf(comic);
    updatePrevNextActions(index > 0, index + 1 < siblings.count());

    optionsDialog->setFilters(currentComicDB.info.brightness, currentComicDB.info.contrast, currentComicDB.info.gamma);
}

void MainWindowViewer::open(QString path, qint64 comicId, qint64 libraryId, YACReader::OpenComicSource source)
{
    currentDirectory = path;

    this->libraryId = libraryId;
    this->source = source;

    enableActions();

    currentComicDB.id = comicId;
    YACReaderLocalClient client;
    int tries = 1;
    bool success = false;
    while (!(success = client.requestComicInfo(libraryId, currentComicDB, siblingComics, source)) && tries != 0)
        tries--;

    if (success) {
        isClient = true;
        open(path + currentComicDB.path, currentComicDB, siblingComics);
    } else {
        isClient = false;
        QMessageBox::information(this, "Connection Error", "Unable to connect to YACReaderLibrary");
        // error
    }

    optionsDialog->setFilters(currentComicDB.info.brightness, currentComicDB.info.contrast, currentComicDB.info.gamma);
}

void MainWindowViewer::openComicFromPath(QString pathFile)
{
    doubleMangaPageAction->setChecked(Configuration::getConfiguration().getDoubleMangaPage());
    openComic(pathFile);
    isClient = false; // this method is used for direct openings
    updatePrevNextActions(!previousComicPath.isEmpty(), !nextComicPath.isEmpty());
}

// isClient shouldn't be modified when a siblinig comic is opened
void MainWindowViewer::openSiblingComic(QString pathFile)
{
    openComic(pathFile);
}

void MainWindowViewer::openComic(QString pathFile)
{
    QFileInfo fi(pathFile);
    currentDirectory = fi.dir().absolutePath();
    getSiblingComics(fi.absolutePath(), fi.fileName());

    setWindowTitle("YACReader - " + fi.fileName());

    enableActions();

    viewer->open(pathFile);
    Configuration::getConfiguration().updateOpenRecentList(fi.absoluteFilePath());
    refreshRecentFilesActionList();
}

void MainWindowViewer::openFolder()
{
    QFileDialog openDialog;
    QString pathDir = openDialog.getExistingDirectory(this, tr("Open folder"), currentDirectory);
    if (!pathDir.isEmpty()) {
        openFolderFromPath(pathDir);
        isClient = false;
    }
}

void MainWindowViewer::openFolderFromPath(QString pathDir)
{
    currentDirectory = pathDir; // TODO ??
    QFileInfo fi(pathDir);
    getSiblingComics(fi.absolutePath(), fi.fileName());

    setWindowTitle("YACReader - " + fi.fileName());

    enableActions();

    viewer->open(pathDir);
    Configuration::getConfiguration().updateOpenRecentList(fi.absoluteFilePath());
    refreshRecentFilesActionList();
}

void MainWindowViewer::openFolderFromPath(QString pathDir, QString atFileName)
{
    currentDirectory = pathDir; // TODO ??
    QFileInfo fi(pathDir);
    getSiblingComics(fi.absolutePath(), fi.fileName());

    setWindowTitle("YACReader - " + fi.fileName());

    enableActions();

    QDir d(pathDir);
    d.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    d.setNameFilters(Comic::getSupportedImageFormats());
    d.setSorting(QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);
    QStringList list = d.entryList();

    std::sort(list.begin(), list.end(), naturalSortLessThanCI);
    int i = 0;
    foreach (QString path, list) {
        if (path.endsWith(atFileName))
            break;
        i++;
    }

    int index = 0;
    if (i < list.count())
        index = i;

    viewer->open(pathDir, index);
}

void MainWindowViewer::saveImage()
{
    QFileDialog saveDialog;
    QString pathFile = saveDialog.getSaveFileName(this, tr("Save current page"), currentDirectoryImgDest + "/" + tr("page_%1.jpg").arg(viewer->getIndex()), tr("Image files (*.jpg)"));
    if (!pathFile.isEmpty()) {
        QFileInfo fi(pathFile);
        currentDirectoryImgDest = fi.absolutePath();
        const QPixmap p = viewer->pixmap();
        if (!p.isNull()) {
            p.save(pathFile);
        }
    }
}

void MainWindowViewer::enableActions()
{
    setActionsEnabled(true);
#ifdef Q_OS_MAC
    activateWindow();
    raise();
#endif
}
void MainWindowViewer::disableActions()
{
    setActionsEnabled(false);
    setLoadedComicActionsEnabled(false);
    setBookmarkAction->setEnabled(false);
}

void MainWindowViewer::disablePreviousNextComicActions()
{
    for (auto *a : { openComicOnTheLeftAction, openComicOnTheRightAction })
        a->setEnabled(false);
}

void MainWindowViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        toggleFullScreen();
        event->accept();
    }
}

void MainWindowViewer::toggleFullScreen()
{
    fullscreen ? toNormal() : toFullScreen();
    Configuration::getConfiguration().setFullScreen(fullscreen = !fullscreen);
}

#ifdef Q_OS_WIN // fullscreen mode in Windows for preventing this bug: QTBUG-41309 https://bugreports.qt.io/browse/QTBUG-41309

void MainWindowViewer::toFullScreen()
{
    fromMaximized = this->isMaximized();

    hideToolBars();
    viewer->hide();
    viewer->fullscreen = true; // TODO, change by the right use of windowState();

    previousWindowFlags = windowFlags();
    previousPos = pos();
    previousSize = size();

    showNormal();
    setWindowFlags(previousWindowFlags | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    QRect r = windowHandle()->screen()->geometry();

    r.setHeight(r.height() + 1);

    setGeometry(r);
    show();

    viewer->show();
    if (viewer->magnifyingGlassIsVisible())
        viewer->showMagnifyingGlass();
}

void MainWindowViewer::toNormal()
{
    // show all
    viewer->hide();
    viewer->fullscreen = false; // TODO, change by the right use of windowState();
    // viewer->hideMagnifyingGlass();

    setWindowFlags(previousWindowFlags);
    move(previousPos);
    resize(previousSize);
    show();

    if (fromMaximized)
        showMaximized();

    if (Configuration::getConfiguration().getShowToolbars())
        showToolBars();
    viewer->show();
    if (viewer->magnifyingGlassIsVisible())
        viewer->showMagnifyingGlass();
}

#else
void MainWindowViewer::toFullScreen()
{
    fromMaximized = this->isMaximized();

    hideToolBars();
    viewer->hide();
    viewer->fullscreen = true; // TODO, change by the right use of windowState();
    setWindowState(Qt::WindowFullScreen);
    viewer->show();
    if (viewer->magnifyingGlassIsVisible())
        viewer->showMagnifyingGlass();
}

void MainWindowViewer::toNormal()
{
    // show all
    viewer->hide();
    viewer->fullscreen = false; // TODO, change by the right use of windowState();
    // viewer->hideMagnifyingGlass();
    if (fromMaximized)
        showMaximized();
    else
        showNormal();

    if (Configuration::getConfiguration().getShowToolbars())
        showToolBars();
    viewer->show();
    if (viewer->magnifyingGlassIsVisible())
        viewer->showMagnifyingGlass();
}
#endif

void MainWindowViewer::toggleToolBars()
{
    toolbars ? hideToolBars() : showToolBars();

    Configuration::getConfiguration().setShowToolbars(toolbars);
#ifndef Q_OS_MAC
    comicToolBar->setMovable(false);
#endif
}
void MainWindowViewer::hideToolBars()
{
    // hide all
    this->comicToolBar->hide();
    toolbars = false;
}

void MainWindowViewer::showToolBars()
{
    this->comicToolBar->show();
    toolbars = true;
}
void MainWindowViewer::fitToWidth()
{
    Configuration::getConfiguration().setFitMode(YACReader::FitMode::ToWidth);
    viewer->setZoomFactor(100);
    viewer->updatePage();
}
void MainWindowViewer::fitToHeight()
{
    Configuration::getConfiguration().setFitMode(YACReader::FitMode::ToHeight);
    viewer->setZoomFactor(100);
    viewer->updatePage();
}

void MainWindowViewer::toggleWidthHeight()
{
    // Only switch to "Fit to height" when we're in "Fit to width"
    if (Configuration::getConfiguration().getFitMode() == YACReader::FitMode::ToWidth) {
        adjustHeightAction->trigger();
    }
    // Default to "Fit to width" in all other cases
    else {
        adjustWidthAction->trigger();
    }
}
void MainWindowViewer::checkNewVersion()
{
    Configuration &conf = Configuration::getConfiguration();
    QDate lastCheck = conf.getLastVersionCheck();
    QDate current = QDate::currentDate();
    if (lastCheck.isNull() || lastCheck.daysTo(current) >= conf.getNumDaysBetweenVersionChecks()) {
        versionChecker = new HttpVersionChecker();

        connect(versionChecker, &HttpVersionChecker::newVersionDetected,
                this, &MainWindowViewer::newVersion);

        QTimer::singleShot(100, versionChecker, &HttpVersionChecker::get);

        conf.setLastVersionCheck(current);
    }
}

void MainWindowViewer::processReset()
{
    disableActions();
    if (!isClient || siblingComics.size() <= 1)
        disablePreviousNextComicActions();
}

void MainWindowViewer::setUpShortcutsManagement()
{
    // actions holder
    auto orphanActions = new QObject;

    QList<QAction *> allActions;
    QList<QAction *> tmpList;

    editShortcutsDialog->addActionsGroup(tr("Comics"), QIcon(":/images/shortcuts_group_comics.png"),
                                         tmpList = { openAction,
                                                     openLatestComicAction,
                                                     openFolderAction,
                                                     saveImageAction,
                                                     openComicOnTheLeftAction,
                                                     openComicOnTheRightAction });

    allActions << tmpList;

    QAction *toggleFullScreenAction = new QAction(tr("Toggle fullscreen mode"), orphanActions);
    toggleFullScreenAction->setData(TOGGLE_FULL_SCREEN_ACTION_Y);
    toggleFullScreenAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_FULL_SCREEN_ACTION_Y));
    addAction(toggleFullScreenAction);
    connect(toggleFullScreenAction, &QAction::triggered, this, &MainWindowViewer::toggleFullScreen);

    QAction *toggleToolbarsAction = new QAction(tr("Hide/show toolbar"), orphanActions);
    toggleToolbarsAction->setData(TOGGLE_TOOL_BARS_ACTION_Y);
    toggleToolbarsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_TOOL_BARS_ACTION_Y));
    addAction(toggleToolbarsAction);
    connect(toggleToolbarsAction, &QAction::triggered, this, &MainWindowViewer::toggleToolBars);

    editShortcutsDialog->addActionsGroup(tr("General"), QIcon(":/images/shortcuts_group_general.png"),
                                         tmpList = QList<QAction *>()
                                                 << optionsAction
                                                 << helpAboutAction
                                                 << showShorcutsAction
                                                 << showInfoAction
                                                 << closeAction
                                                 << showDictionaryAction
                                                 << showFlowAction
                                                 << toggleFullScreenAction
                                                 << toggleToolbarsAction
                                                 << showEditShortcutsAction
#ifdef Q_OS_MAC
                                                 << newInstanceAction
#endif
    );

    allActions << tmpList;

    // keys without actions (MGlass)
    auto sizeUpMglassAction = new QAction(tr("Size up magnifying glass"), orphanActions);
    sizeUpMglassAction->setData(SIZE_UP_MGLASS_ACTION_Y);
    sizeUpMglassAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SIZE_UP_MGLASS_ACTION_Y));

    auto sizeDownMglassAction = new QAction(tr("Size down magnifying glass"), orphanActions);
    sizeDownMglassAction->setData(SIZE_DOWN_MGLASS_ACTION_Y);
    sizeDownMglassAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SIZE_DOWN_MGLASS_ACTION_Y));

    auto zoomInMglassAction = new QAction(tr("Zoom in magnifying glass"), orphanActions);
    zoomInMglassAction->setData(ZOOM_IN_MGLASS_ACTION_Y);
    zoomInMglassAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ZOOM_IN_MGLASS_ACTION_Y));

    auto zoomOutMglassAction = new QAction(tr("Zoom out magnifying glass"), orphanActions);
    zoomOutMglassAction->setData(ZOOM_OUT_MGLASS_ACTION_Y);
    zoomOutMglassAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ZOOM_OUT_MGLASS_ACTION_Y));

    editShortcutsDialog->addActionsGroup(tr("Magnifiying glass"), QIcon(":/images/shortcuts_group_mglass.png"),
                                         tmpList = QList<QAction *>()
                                                 << showMagnifyingGlassAction
                                                 << sizeUpMglassAction
                                                 << sizeDownMglassAction
                                                 << zoomInMglassAction
                                                 << zoomOutMglassAction);

    allActions << tmpList;

    auto toggleFitToScreenAction = new QAction(tr("Toggle between fit to width and fit to height"), orphanActions);
    toggleFitToScreenAction->setData(CHANGE_FIT_ACTION_Y);
    toggleFitToScreenAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(CHANGE_FIT_ACTION_Y));
    addAction(toggleFitToScreenAction);
    connect(toggleFitToScreenAction, &QAction::triggered, this, &MainWindowViewer::toggleWidthHeight);

    editShortcutsDialog->addActionsGroup(tr("Page adjustement"), QIcon(":/images/shortcuts_group_page.png"),
                                         tmpList = QList<QAction *>()
                                                 << adjustHeightAction
                                                 << adjustWidthAction
                                                 << toggleFitToScreenAction
                                                 << leftRotationAction
                                                 << rightRotationAction
                                                 << doublePageAction
                                                 << doubleMangaPageAction
                                                 << adjustToFullSizeAction
                                                 << fitToPageAction
                                                 << increasePageZoomAction
                                                 << decreasePageZoomAction
                                                 << resetZoomAction);

    allActions << tmpList;

    auto autoScrollForwardAction = new QAction(tr("Autoscroll down"), orphanActions);
    autoScrollForwardAction->setData(AUTO_SCROLL_FORWARD_ACTION_Y);
    autoScrollForwardAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(AUTO_SCROLL_FORWARD_ACTION_Y));
    connect(autoScrollForwardAction, &QAction::triggered, viewer, &Viewer::scrollForward);

    auto autoScrollBackwardAction = new QAction(tr("Autoscroll up"), orphanActions);
    autoScrollBackwardAction->setData(AUTO_SCROLL_BACKWARD_ACTION_Y);
    autoScrollBackwardAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(AUTO_SCROLL_BACKWARD_ACTION_Y));
    connect(autoScrollBackwardAction, &QAction::triggered, viewer, &Viewer::scrollBackward);

    auto autoScrollForwardHorizontalFirstAction = new QAction(tr("Autoscroll forward, horizontal first"), orphanActions);
    autoScrollForwardHorizontalFirstAction->setData(AUTO_SCROLL_FORWARD_HORIZONTAL_FIRST_ACTION_Y);
    autoScrollForwardHorizontalFirstAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(AUTO_SCROLL_FORWARD_HORIZONTAL_FIRST_ACTION_Y));
    connect(autoScrollForwardHorizontalFirstAction, &QAction::triggered, viewer, &Viewer::scrollForwardHorizontalFirst);

    auto autoScrollBackwardHorizontalFirstAction = new QAction(tr("Autoscroll backward, horizontal first"), orphanActions);
    autoScrollBackwardHorizontalFirstAction->setData(AUTO_SCROLL_BACKWARD_HORIZONTAL_FIRST_ACTION_Y);
    autoScrollBackwardHorizontalFirstAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(AUTO_SCROLL_BACKWARD_HORIZONTAL_FIRST_ACTION_Y));
    connect(autoScrollBackwardHorizontalFirstAction, &QAction::triggered, viewer, &Viewer::scrollBackwardHorizontalFirst);

    auto autoScrollForwardVerticalFirstAction = new QAction(tr("Autoscroll forward, vertical first"), orphanActions);
    autoScrollForwardVerticalFirstAction->setData(AUTO_SCROLL_FORWARD_VERTICAL_FIRST_ACTION_Y);
    autoScrollForwardVerticalFirstAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(AUTO_SCROLL_FORWARD_VERTICAL_FIRST_ACTION_Y));
    connect(autoScrollForwardVerticalFirstAction, &QAction::triggered, viewer, &Viewer::scrollForwardVerticalFirst);

    auto autoScrollBackwardVerticalFirstAction = new QAction(tr("Autoscroll backward, vertical first"), orphanActions);
    autoScrollBackwardVerticalFirstAction->setData(AUTO_SCROLL_BACKWARD_VERTICAL_FIRST_ACTION_Y);
    autoScrollBackwardVerticalFirstAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(AUTO_SCROLL_BACKWARD_VERTICAL_FIRST_ACTION_Y));
    connect(autoScrollBackwardVerticalFirstAction, &QAction::triggered, viewer, &Viewer::scrollBackwardVerticalFirst);

    auto moveDownAction = new QAction(tr("Move down"), orphanActions);
    moveDownAction->setData(MOVE_DOWN_ACTION_Y);
    moveDownAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(MOVE_DOWN_ACTION_Y));
    connect(moveDownAction, &QAction::triggered, viewer, [this] { viewer->moveView(Qt::Key_Down); });

    auto moveUpAction = new QAction(tr("Move up"), orphanActions);
    moveUpAction->setData(MOVE_UP_ACTION_Y);
    moveUpAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(MOVE_UP_ACTION_Y));
    connect(moveUpAction, &QAction::triggered, viewer, [this] { viewer->moveView(Qt::Key_Up); });

    auto moveLeftAction = new QAction(tr("Move left"), orphanActions);
    moveLeftAction->setData(MOVE_LEFT_ACTION_Y);
    moveLeftAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(MOVE_LEFT_ACTION_Y));
    connect(moveLeftAction, &QAction::triggered, viewer, [this] { viewer->moveView(Qt::Key_Left); });

    auto moveRightAction = new QAction(tr("Move right"), orphanActions);
    moveRightAction->setData(MOVE_RIGHT_ACTION_Y);
    moveRightAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(MOVE_RIGHT_ACTION_Y));
    connect(moveRightAction, &QAction::triggered, viewer, [this] { viewer->moveView(Qt::Key_Right); });

    auto goToFirstPageAction = new QAction(tr("Go to the first page"), orphanActions);
    goToFirstPageAction->setData(GO_TO_FIRST_PAGE_ACTION_Y);
    goToFirstPageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(GO_TO_FIRST_PAGE_ACTION_Y));
    connect(goToFirstPageAction, &QAction::triggered, viewer, &Viewer::goToFirstPage);

    auto goToLastPageAction = new QAction(tr("Go to the last page"), orphanActions);
    goToLastPageAction->setData(GO_TO_LAST_PAGE_ACTION_Y);
    goToLastPageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(GO_TO_LAST_PAGE_ACTION_Y));
    connect(goToLastPageAction, &QAction::triggered, viewer, &Viewer::goToLastPage);

    loadedComicActions = { autoScrollForwardAction,
                           autoScrollBackwardAction,
                           autoScrollForwardHorizontalFirstAction,
                           autoScrollBackwardHorizontalFirstAction,
                           autoScrollForwardVerticalFirstAction,
                           autoScrollBackwardVerticalFirstAction,
                           moveDownAction,
                           moveUpAction,
                           moveLeftAction,
                           moveRightAction,
                           goToFirstPageAction,
                           goToLastPageAction };
    addActions(loadedComicActions);

    editShortcutsDialog->addActionsGroup(tr("Reading"), QIcon(":/images/shortcuts_group_reading.png"),
                                         tmpList = QList<QAction *>()
                                                 << goToPageOnTheRightAction
                                                 << goToPageOnTheLeftAction
                                                 << setBookmarkAction
                                                 << showBookmarksAction
                                                 << loadedComicActions
                                                 << goToPageAction);

    allActions << tmpList;

    ShortcutsManager::getShortcutsManager().registerActions(allActions);
}

void MainWindowViewer::doubleMangaPageSwitch()
{
    if (isClient) {
        int index = siblingComics.indexOf(currentComicDB);
        updatePrevNextActions(index > 0, index + 1 < siblingComics.size());
    } else {
        updatePrevNextActions(!previousComicPath.isEmpty(), !nextComicPath.isEmpty());
    }
}

void MainWindowViewer::toggleFitToWidthSlider()
{
    int y;

#ifdef Q_OS_MAC
    y = 0;
#else
    y = this->comicToolBar->frameSize().height();
#endif

    if (zoomSliderAction->isVisible()) {
        zoomSliderAction->hide();
    } else {
        zoomSliderAction->move(250, y);
        zoomSliderAction->show();
    }
}

void MainWindowViewer::newVersion()
{
    QMessageBox msgBox;
    msgBox.setText(tr("There is a new version available"));
    msgBox.setInformativeText(tr("Do you want to download the new version?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Ignore | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.button(QMessageBox::Ignore)->setText(tr("Remind me in 14 days"));
    msgBox.button(QMessageBox::No)->setText(tr("Not now"));
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox.setModal(true);
    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::Yes:
        QDesktopServices::openUrl(QUrl("http://www.yacreader.com"));
        break;
    case QMessageBox::No:
        Configuration::getConfiguration().setNumDaysBetweenVersionChecks(1);
        break;
    case QMessageBox::Ignore:
        Configuration::getConfiguration().setNumDaysBetweenVersionChecks(14);
        break;
    }
}

void MainWindowViewer::closeEvent(QCloseEvent *event)
{
    if (isClient)
        sendComic();

    viewer->save();
    Configuration &conf = Configuration::getConfiguration();
    if (!fullscreen && !isMaximized())
        conf.setGeometry(saveGeometry());
    conf.setMaximized(isMaximized());

    event->accept();
}

void MainWindowViewer::openPreviousComic()
{
    if (!siblingComics.isEmpty() && isClient) {
        sendComic();

        int currentIndex = siblingComics.indexOf(currentComicDB);
        if (currentIndex == -1)
            return;
        if (currentIndex - 1 >= 0 && currentIndex - 1 < siblingComics.count()) {
            siblingComics[currentIndex] = currentComicDB; // updated
            currentComicDB = siblingComics.at(currentIndex - 1);
            open(currentDirectory + currentComicDB.path, currentComicDB, siblingComics);
        }
        return;
    }
    if (!previousComicPath.isEmpty()) {
        openSiblingComic(previousComicPath);
    }
}

void MainWindowViewer::openNextComic()
{
    if (!siblingComics.isEmpty() && isClient) {
        sendComic();

        int currentIndex = siblingComics.indexOf(currentComicDB);
        if (currentIndex == -1)
            return;
        if (currentIndex + 1 > 0 && currentIndex + 1 < siblingComics.count()) {
            siblingComics[currentIndex] = currentComicDB; // updated
            currentComicDB = siblingComics.at(currentIndex + 1);
            open(currentDirectory + currentComicDB.path, currentComicDB, siblingComics);
        }

        return;
    }
    if (!nextComicPath.isEmpty()) {
        openSiblingComic(nextComicPath);
    }
}

void MainWindowViewer::openLeftComic()
{
    if (viewer->getIsMangaMode()) {
        openNextComic();
    } else {
        openPreviousComic();
    }
}

void MainWindowViewer::openRightComic()
{
    if (viewer->getIsMangaMode()) {
        openPreviousComic();
    } else {
        openNextComic();
    }
}

void MainWindowViewer::getSiblingComics(QString path, QString currentComic)
{
    QDir d(path);
    d.setFilter(QDir::Files | QDir::NoDotAndDotDot);
#ifndef use_unarr
    d.setNameFilters(QStringList() << "*.cbr"
                                   << "*.cbz"
                                   << "*.rar"
                                   << "*.zip"
                                   << "*.tar"
                                   << "*.pdf"
                                   << "*.7z"
                                   << "*.cb7"
                                   << "*.arj"
                                   << "*.cbt");
#else
    d.setNameFilters(QStringList() << "*.cbr"
                                   << "*.cbz"
                                   << "*.rar"
                                   << "*.zip"
                                   << "*.tar"
                                   << "*.pdf"
                                   << "*.cbt");
#endif
    d.setSorting(QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);
    QStringList list = d.entryList();
    std::sort(list.begin(), list.end(), naturalSortLessThanCI);
    int index = list.indexOf(currentComic);
    if (index == -1) // comic not found
    {
        /*QFile f(QCoreApplication::applicationDirPath()+"/errorLog.txt");
                        if(!f.open(QIODevice::WriteOnly))
                        {
                                QMessageBox::critical(NULL,tr("Saving error log file...."),tr("There was a problem saving YACReader error log file. Please, check if you have enough permissions in the YACReader root folder."));
                        }
                        else
                        {
                        QTextStream txtS(&f);
                        txtS << "METHOD : MainWindowViewer::getSiblingComics" << '\n';
                        txtS << "ERROR : current comic not found in its own path" << '\n';
                        txtS << path << '\n';
                        txtS << currentComic << '\n';
                        txtS << "Comic list count : " + list.count() << '\n';
                        foreach(QString s, list){
                                txtS << s << '\n';
                        }
                        f.close();
                }*/
    }

    previousComicPath = nextComicPath = "";
    if (index > 0) {
        previousComicPath = path + "/" + list.at(index - 1);
    }

    if (index + 1 < list.count()) {
        nextComicPath = path + "/" + list.at(index + 1);
    }

    updatePrevNextActions(index > 0, index + 1 < list.count());
}

void MainWindowViewer::setActionsEnabled(bool enabled)
{
    // TODO enable goTo and showInfo (or update) when numPages emited
    const auto actions = { saveImageAction,
                           goToPageOnTheLeftAction,
                           goToPageOnTheRightAction,
                           adjustHeightAction,
                           adjustWidthAction,
                           goToPageAction,
                           leftRotationAction,
                           rightRotationAction,
                           showMagnifyingGlassAction,
                           doublePageAction,
                           doubleMangaPageAction,
                           adjustToFullSizeAction,
                           fitToPageAction,
                           showZoomSliderlAction,
                           increasePageZoomAction,
                           decreasePageZoomAction,
                           resetZoomAction,
                           showBookmarksAction,
                           showInfoAction,
                           showDictionaryAction,
                           showFlowAction };
    for (auto *a : actions)
        a->setEnabled(enabled);
}

void MainWindowViewer::setLoadedComicActionsEnabled(bool enabled)
{
    for (auto *a : std::as_const(loadedComicActions))
        a->setEnabled(enabled);
}

void MainWindowViewer::dropEvent(QDropEvent *event)
{
    QList<QUrl> urlList;
    QString fName;
    QFileInfo info;

    if (event->mimeData()->hasUrls()) {
        urlList = event->mimeData()->urls();

        if (urlList.size() > 0) {
            fName = urlList[0].toLocalFile(); // convert first QUrl to local path
            info.setFile(fName); // information about file
            if (info.isFile()) {
                QStringList imageSuffixs = Comic::getSupportedImageLiteralFormats();
                if (imageSuffixs.contains(info.suffix())) // image dropped
                    openFolderFromPath(info.absoluteDir().absolutePath(), info.fileName());
                else
                    openComicFromPath(fName); // if is file, setText
            } else if (info.isDir())
                openFolderFromPath(fName);

            isClient = false;
        }
    }

    event->acceptProposedAction();
}
void MainWindowViewer::dragEnterEvent(QDragEnterEvent *event)
{
    // accept just text/uri-list mime format
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
        isClient = false;
    }
}

void MainWindowViewer::adjustToFullSizeSwitch()
{
    Configuration::getConfiguration().setFitMode(YACReader::FitMode::FullRes);
    viewer->setZoomFactor(100);
    viewer->updatePage();
}

void MainWindowViewer::fitToPageSwitch()
{
    Configuration::getConfiguration().setFitMode(YACReader::FitMode::FullPage);
    viewer->setZoomFactor(100);
    viewer->updatePage();
}

void MainWindowViewer::resetZoomLevel()
{
    viewer->setZoomFactor(100);
    viewer->updatePage();
}

void MainWindowViewer::increasePageZoomLevel()
{
    viewer->increaseZoomFactor();
}

void MainWindowViewer::decreasePageZoomLevel()
{
    viewer->decreaseZoomFactor();
}

void MainWindowViewer::sendComic()
{
    auto client = new YACReaderLocalClient;

    connect(client, &YACReaderLocalClient::finished, client, &YACReaderLocalClient::deleteLater);
    currentComicDB.info.lastTimeOpened = QDateTime::currentMSecsSinceEpoch() / 1000;

    viewer->updateComic(currentComicDB);

    int currentIndex = siblingComics.indexOf(currentComicDB);
    bool sendNextComicInfo = (currentComicDB.info.currentPage == currentComicDB.info.numPages) && (currentIndex + 1 > 0 && currentIndex + 1 < siblingComics.count());

    if (sendNextComicInfo) {
        ComicDB &nextComic = siblingComics[currentIndex + 1];
        nextComic.info.hasBeenOpened = true;
        int retries = 1;
        while (!client->sendComicInfo(libraryId, currentComicDB, nextComic.id) && retries != 0)
            retries--;
    } else {
        int retries = 1;
        while (!client->sendComicInfo(libraryId, currentComicDB) && retries != 0)
            retries--;
    }
}

void MainWindowViewer::updatePrevNextActions(bool thereIsPrevious, bool thereIsNext)
{
    if (thereIsPrevious) {
        if (viewer->getIsMangaMode()) {
            openComicOnTheRightAction->setDisabled(false);
        } else {
            openComicOnTheLeftAction->setDisabled(false);
        }
    } else {
        if (viewer->getIsMangaMode()) {
            openComicOnTheRightAction->setDisabled(true);
        } else {
            openComicOnTheLeftAction->setDisabled(true);
        }
    }

    if (thereIsNext) {
        if (viewer->getIsMangaMode()) {
            openComicOnTheLeftAction->setDisabled(false);
        } else {
            openComicOnTheRightAction->setDisabled(false);
        }
    } else {
        if (viewer->getIsMangaMode()) {
            openComicOnTheLeftAction->setDisabled(true);
        } else {
            openComicOnTheRightAction->setDisabled(true);
        }
    }

    if (viewer->getIsMangaMode()) {
        openComicOnTheLeftAction->setToolTip(tr("Open next comic"));
        openComicOnTheRightAction->setToolTip(tr("Open previous comic"));
        goToPageOnTheLeftAction->setToolTip(tr("Go to next page"));
        goToPageOnTheRightAction->setToolTip(tr("Go to previous page"));
    } else {
        openComicOnTheLeftAction->setToolTip(tr("Open previous comic"));
        openComicOnTheRightAction->setToolTip(tr("Open next comic"));
        goToPageOnTheLeftAction->setToolTip(tr("Go to previous page"));
        goToPageOnTheRightAction->setToolTip(tr("Go to next page"));
    }
}
