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

#include <ctime>
#include <algorithm>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopWidget>
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
:QMainWindow(),fullscreen(false),toolbars(true),alwaysOnTop(false),currentDirectory("."),currentDirectoryImgDest("."),isClient(false)
{
	loadConfiguration();
	setupUI();
}

MainWindowViewer::~MainWindowViewer()
{
	delete settings;
	delete viewer;
	delete had;

	delete sliderAction;
	delete openAction;
	delete openFolderAction;
	delete saveImageAction;
	delete openPreviousComicAction; 
	delete openNextComicAction;
	delete prevAction;
	delete nextAction;
    delete adjustHeightAction;
    delete adjustWidthAction;
	delete leftRotationAction;
	delete rightRotationAction;
	delete doublePageAction;
	delete doubleMangaPageAction;
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
	delete alwaysOnTopAction;
	delete adjustToFullSizeAction;
	delete showFlowAction;

}
void MainWindowViewer::loadConfiguration()
{
	settings = new QSettings(YACReader::getSettingsPath()+"/YACReader.ini",QSettings::IniFormat);

	Configuration & config = Configuration::getConfiguration();
	config.load(settings);
	currentDirectory = config.getDefaultPath();
	fullscreen = config.getFullScreen();
}

void MainWindowViewer::setupUI()
{
	setWindowIcon(QIcon(":/images/icon.png"));

    //setUnifiedTitleAndToolBarOnMac(true);

	viewer = new Viewer(this);
    connect(viewer,SIGNAL(reset()),this,SLOT(processReset()));
	//detected end of comic
	connect(viewer,SIGNAL(openNextComic()),this,SLOT(openNextComic()));
	//detected start of comic
	connect(viewer,SIGNAL(openPreviousComic()),this,SLOT(openPreviousComic()));

	setCentralWidget(viewer);
	int heightDesktopResolution = QApplication::desktop()->screenGeometry().height();
	int widthDesktopResolution = QApplication::desktop()->screenGeometry().width();
	int height,width;
	height = static_cast<int>(heightDesktopResolution*0.84);
	width = static_cast<int>(height*0.70);
	Configuration & conf = Configuration::getConfiguration();
	QPoint p = conf.getPos();
	QSize s = conf.getSize();
	if(s.width()!=0)
	{
		move(p);
		resize(s);
	}
	else
	{
		move(QPoint((widthDesktopResolution-width)/2,((heightDesktopResolution-height)-40)/2));
		resize(QSize(width,height));
	}

	had = new HelpAboutDialog(this); //TODO load data

	had->loadAboutInformation(":/files/about.html");
	had->loadHelp(":/files/helpYACReader.html");

	optionsDialog = new OptionsDialog(this);
	connect(optionsDialog,SIGNAL(accepted()),viewer,SLOT(updateOptions()));
	connect(optionsDialog,SIGNAL(fitToWidthRatioChanged(float)),viewer,SLOT(updateFitToWidthRatio(float)));
	connect(optionsDialog, SIGNAL(optionsChanged()),this,SLOT(reloadOptions()));
	connect(optionsDialog,SIGNAL(changedFilters(int,int,int)),viewer,SLOT(updateFilters(int,int,int)));

	optionsDialog->restoreOptions(settings);
    //shortcutsDialog = new ShortcutsDialog(this);
    editShortcutsDialog = new EditShortcutsDialog(this);
    connect(optionsDialog,SIGNAL(editShortcuts()),editShortcutsDialog,SLOT(show()));

	createActions();
    setUpShortcutsManagement();

	createToolBars();

	setWindowTitle("YACReader");

	checkNewVersion();
	
	viewer->setFocusPolicy(Qt::StrongFocus);
	

	//if(Configuration::getConfiguration().getAlwaysOnTop())
	//{
	//	setWindowFlags(this->windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
	//}

	if(fullscreen)
		toFullScreen();
	if(conf.getMaximized())
		showMaximized();

	setAcceptDrops(true);

	if(Configuration::getConfiguration().getShowToolbars() && !Configuration::getConfiguration().getFullScreen())
		showToolBars();
	else
		hideToolBars();
}

void MainWindowViewer::createActions()
{
    openAction = new QAction(tr("&Open"),this);
    openAction->setIcon(QIcon(":/images/viewer_toolbar/open.png"));
    openAction->setToolTip(tr("Open a comic"));
    openAction->setData(OPEN_ACTION_Y);
    openAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_ACTION_Y));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

    openFolderAction = new QAction(tr("Open Folder"),this);
    openFolderAction->setIcon(QIcon(":/images/viewer_toolbar/openFolder.png"));
    openFolderAction->setToolTip(tr("Open image folder"));
    openFolderAction->setData(OPEN_FOLDER_ACTION_Y);
    openFolderAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_FOLDER_ACTION_Y));
    connect(openFolderAction, SIGNAL(triggered()), this, SLOT(openFolder()));

    saveImageAction = new QAction(tr("Save"),this);
    saveImageAction->setIcon(QIcon(":/images/viewer_toolbar/save.png"));
    saveImageAction->setToolTip(tr("Save current page"));
    saveImageAction->setDisabled(true);
    saveImageAction->setData(SAVE_IMAGE_ACTION_Y);
    saveImageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SAVE_IMAGE_ACTION_Y));
    connect(saveImageAction,SIGNAL(triggered()),this,SLOT(saveImage()));

    openPreviousComicAction = new QAction(tr("Previous Comic"),this);
    openPreviousComicAction->setIcon(QIcon(":/images/viewer_toolbar/openPrevious.png"));
    openPreviousComicAction->setToolTip(tr("Open previous comic"));
    openPreviousComicAction->setDisabled(true);
    openPreviousComicAction->setData(OPEN_PREVIOUS_COMIC_ACTION_Y);
    openPreviousComicAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_PREVIOUS_COMIC_ACTION_Y));
    connect(openPreviousComicAction,SIGNAL(triggered()),this,SLOT(openPreviousComic()));

    openNextComicAction = new QAction(tr("Next Comic"),this);
    openNextComicAction->setIcon(QIcon(":/images/viewer_toolbar/openNext.png"));
    openNextComicAction->setToolTip(tr("Open next comic"));
    openNextComicAction->setDisabled(true);
    openNextComicAction->setData(OPEN_NEXT_COMIC_ACTION_Y);
    openNextComicAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_NEXT_COMIC_ACTION_Y));
    connect(openNextComicAction,SIGNAL(triggered()),this,SLOT(openNextComic()));

    prevAction = new QAction(tr("&Previous"),this);
    prevAction->setIcon(QIcon(":/images/viewer_toolbar/previous.png"));
    prevAction->setShortcutContext(Qt::WidgetShortcut);
    prevAction->setToolTip(tr("Go to previous page"));
    prevAction->setDisabled(true);
    prevAction->setData(PREV_ACTION_Y);
    prevAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(PREV_ACTION_Y));
    connect(prevAction, SIGNAL(triggered()),viewer,SLOT(prev()));

    nextAction = new QAction(tr("&Next"),this);
    nextAction->setIcon(QIcon(":/images/viewer_toolbar/next.png"));
    nextAction->setShortcutContext(Qt::WidgetShortcut);
    nextAction->setToolTip(tr("Go to next page"));
	nextAction->setDisabled(true);
    nextAction->setData(NEXT_ACTION_Y);
    nextAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(NEXT_ACTION_Y));
	connect(nextAction, SIGNAL(triggered()),viewer,SLOT(next()));

    adjustHeightAction = new QAction(tr("Fit Height"),this);
    adjustHeightAction->setIcon(QIcon(":/images/viewer_toolbar/toHeight.png"));
	//adjustWidth->setCheckable(true);
    adjustHeightAction->setDisabled(true);
    adjustHeightAction->setChecked(Configuration::getConfiguration().getAdjustToWidth());
    adjustHeightAction->setToolTip(tr("Fit image to height"));
	//adjustWidth->setIcon(QIcon(":/images/fitWidth.png"));
    adjustHeightAction->setData(ADJUST_HEIGHT_ACTION_Y);
    adjustHeightAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ADJUST_HEIGHT_ACTION_Y));
    connect(adjustHeightAction, SIGNAL(triggered()),this,SLOT(fitToHeight()));

    adjustWidthAction = new QAction(tr("Fit Width"),this);
    adjustWidthAction->setIcon(QIcon(":/images/viewer_toolbar/toWidth.png"));
	//adjustWidth->setCheckable(true);
    adjustWidthAction->setDisabled(true);
    adjustWidthAction->setChecked(Configuration::getConfiguration().getAdjustToWidth());
    adjustWidthAction->setToolTip(tr("Fit image to width"));
	//adjustWidth->setIcon(QIcon(":/images/fitWidth.png"));
    adjustWidthAction->setData(ADJUST_WIDTH_ACTION_Y);
    adjustWidthAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ADJUST_WIDTH_ACTION_Y));
    connect(adjustWidthAction, SIGNAL(triggered()),this,SLOT(fitToWidth()));

	leftRotationAction = new QAction(tr("Rotate image to the left"),this);
	leftRotationAction->setIcon(QIcon(":/images/viewer_toolbar/rotateL.png"));
	leftRotationAction->setDisabled(true);
    leftRotationAction->setData(LEFT_ROTATION_ACTION_Y);
    leftRotationAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(LEFT_ROTATION_ACTION_Y));
	connect(leftRotationAction, SIGNAL(triggered()),viewer,SLOT(rotateLeft()));

	rightRotationAction = new QAction(tr("Rotate image to the right"),this);
	rightRotationAction->setIcon(QIcon(":/images/viewer_toolbar/rotateR.png"));
	rightRotationAction->setDisabled(true);
    rightRotationAction->setData(RIGHT_ROTATION_ACTION_Y);
    rightRotationAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(RIGHT_ROTATION_ACTION_Y));
	connect(rightRotationAction, SIGNAL(triggered()),viewer,SLOT(rotateRight()));

	doublePageAction = new QAction(tr("Double page mode"),this);
	doublePageAction->setToolTip(tr("Switch to double page mode"));
	doublePageAction->setIcon(QIcon(":/images/viewer_toolbar/doublePage.png"));
	doublePageAction->setDisabled(true);
	doublePageAction->setCheckable(true);
	doublePageAction->setChecked(Configuration::getConfiguration().getDoublePage());
    doublePageAction->setData(DOUBLE_PAGE_ACTION_Y);
    doublePageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(DOUBLE_PAGE_ACTION_Y));
	connect(doublePageAction, SIGNAL(triggered()),viewer,SLOT(doublePageSwitch()));
	
	//inversed pictures mode
	doubleMangaPageAction = new QAction(tr("Double page manga mode"),this);
	doubleMangaPageAction->setToolTip(tr("Reverse reading order in double page mode"));
	doubleMangaPageAction->setIcon(QIcon(":/images/viewer_toolbar/doubleMangaPage.png"));
	doubleMangaPageAction->setDisabled(true);
	doubleMangaPageAction->setCheckable(true);
	doubleMangaPageAction->setChecked(Configuration::getConfiguration().getDoubleMangaPage());
   doubleMangaPageAction->setData(DOUBLE_MANGA_PAGE_ACTION_Y);
   doubleMangaPageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(DOUBLE_MANGA_PAGE_ACTION_Y));
	connect(doubleMangaPageAction, SIGNAL(triggered()),viewer,SLOT(doubleMangaPageSwitch()));
	
    goToPageAction = new QAction(tr("Go To"),this);
    goToPageAction->setIcon(QIcon(":/images/viewer_toolbar/goto.png"));
    goToPageAction->setDisabled(true);
    goToPageAction->setToolTip(tr("Go to page ..."));
    goToPageAction->setData(GO_TO_PAGE_ACTION_Y);
    goToPageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(GO_TO_PAGE_ACTION_Y));
    connect(goToPageAction, SIGNAL(triggered()),viewer,SLOT(showGoToDialog()));

	optionsAction = new QAction(tr("Options"),this);
	optionsAction->setToolTip(tr("YACReader options"));
    optionsAction->setData(OPTIONS_ACTION_Y);
    optionsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPTIONS_ACTION_Y));
	optionsAction->setIcon(QIcon(":/images/viewer_toolbar/options.png"));

	connect(optionsAction, SIGNAL(triggered()),optionsDialog,SLOT(show()));

	helpAboutAction = new QAction(tr("Help"),this);
	helpAboutAction->setToolTip(tr("Help, About YACReader"));
	helpAboutAction->setIcon(QIcon(":/images/viewer_toolbar/help.png"));
    helpAboutAction->setData(HELP_ABOUT_ACTION_Y);
    helpAboutAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(HELP_ABOUT_ACTION_Y));
	connect(helpAboutAction, SIGNAL(triggered()),had,SLOT(show()));

    showMagnifyingGlassAction = new QAction(tr("Magnifying glass"),this);
    showMagnifyingGlassAction->setToolTip(tr("Switch Magnifying glass"));
    showMagnifyingGlassAction->setIcon(QIcon(":/images/viewer_toolbar/magnifyingGlass.png"));
    showMagnifyingGlassAction->setDisabled(true);
    showMagnifyingGlassAction->setCheckable(true);
    showMagnifyingGlassAction->setData(SHOW_MAGNIFYING_GLASS_ACTION_Y);
    showMagnifyingGlassAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_MAGNIFYING_GLASS_ACTION_Y));
    connect(showMagnifyingGlassAction, SIGNAL(triggered()),viewer,SLOT(magnifyingGlassSwitch()));

    setBookmarkAction = new QAction(tr("Set bookmark"),this);
    setBookmarkAction->setToolTip(tr("Set a bookmark on the current page"));
    setBookmarkAction->setIcon(QIcon(":/images/viewer_toolbar/bookmark.png"));
    setBookmarkAction->setDisabled(true);
    setBookmarkAction->setCheckable(true);
    setBookmarkAction->setData(SET_BOOKMARK_ACTION_Y);
    setBookmarkAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_BOOKMARK_ACTION_Y));
    connect(setBookmarkAction,SIGNAL(triggered (bool)),viewer,SLOT(setBookmarkAction(bool)));
    connect(viewer,SIGNAL(pageAvailable(bool)),setBookmarkAction,SLOT(setEnabled(bool)));
    connect(viewer,SIGNAL(pageIsBookmark(bool)),setBookmarkAction,SLOT(setChecked(bool)));

    showBookmarksAction = new QAction(tr("Show bookmarks"),this);
    showBookmarksAction->setToolTip(tr("Show the bookmarks of the current comic"));
    showBookmarksAction->setIcon(QIcon(":/images/viewer_toolbar/showBookmarks.png"));
    showBookmarksAction->setDisabled(true);
    showBookmarksAction->setData(SHOW_BOOKMARKS_ACTION_Y);
    showBookmarksAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_BOOKMARKS_ACTION_Y));
    connect(showBookmarksAction, SIGNAL(triggered()),viewer->getBookmarksDialog(),SLOT(show()));

	showShorcutsAction = new QAction(tr("Show keyboard shortcuts"), this );
	showShorcutsAction->setIcon(QIcon(":/images/viewer_toolbar/shortcuts.png"));
    showShorcutsAction->setData(SHOW_SHORCUTS_ACTION_Y);
    showShorcutsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_SHORCUTS_ACTION_Y));
    //connect(showShorcutsAction, SIGNAL(triggered()),shortcutsDialog,SLOT(show()));
    connect(showShorcutsAction, SIGNAL(triggered()), editShortcutsDialog, SLOT(show()));

    showInfoAction = new QAction(tr("Show Info"),this);
    showInfoAction->setIcon(QIcon(":/images/viewer_toolbar/info.png"));
    showInfoAction->setDisabled(true);
    showInfoAction->setData(SHOW_INFO_ACTION_Y);
    showInfoAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_INFO_ACTION_Y));
    connect(showInfoAction, SIGNAL(triggered()),viewer,SLOT(informationSwitch()));

	closeAction = new QAction(tr("Close"),this);
	closeAction->setIcon(QIcon(":/images/viewer_toolbar/close.png"));
    closeAction->setData(CLOSE_ACTION_Y);
    closeAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(CLOSE_ACTION_Y));
	connect(closeAction,SIGNAL(triggered()),this,SLOT(close()));

	showDictionaryAction = new QAction(tr("Show Dictionary"),this);
	showDictionaryAction->setIcon(QIcon(":/images/viewer_toolbar/translator.png"));
	//showDictionaryAction->setCheckable(true);
	showDictionaryAction->setDisabled(true);
    showDictionaryAction->setData(SHOW_DICTIONARY_ACTION_Y);
    showDictionaryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_DICTIONARY_ACTION_Y));
	connect(showDictionaryAction,SIGNAL(triggered()),viewer,SLOT(translatorSwitch()));

    //deprecated
	alwaysOnTopAction = new QAction(tr("Always on top"),this);
	alwaysOnTopAction->setIcon(QIcon(":/images/alwaysOnTop.png"));
	alwaysOnTopAction->setCheckable(true);
	alwaysOnTopAction->setDisabled(true);
	alwaysOnTopAction->setChecked(Configuration::getConfiguration().getAlwaysOnTop());
    alwaysOnTopAction->setData(ALWAYS_ON_TOP_ACTION_Y);
    alwaysOnTopAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ALWAYS_ON_TOP_ACTION_Y));
	connect(alwaysOnTopAction,SIGNAL(triggered()),this,SLOT(alwaysOnTopSwitch()));

	adjustToFullSizeAction = new QAction(tr("Show full size"),this);
	adjustToFullSizeAction->setIcon(QIcon(":/images/viewer_toolbar/full.png"));
	adjustToFullSizeAction->setCheckable(true);
	adjustToFullSizeAction->setDisabled(true);
	adjustToFullSizeAction->setChecked(Configuration::getConfiguration().getAdjustToFullSize());
    adjustToFullSizeAction->setData(ADJUST_TO_FULL_SIZE_ACTION_Y);
    adjustToFullSizeAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ADJUST_TO_FULL_SIZE_ACTION_Y));
	connect(adjustToFullSizeAction,SIGNAL(triggered()),this,SLOT(adjustToFullSizeSwitch()));

	showFlowAction = new QAction(tr("Show go to flow"),this);
	showFlowAction->setIcon(QIcon(":/images/viewer_toolbar/flow.png"));
	showFlowAction->setDisabled(true);
    showFlowAction->setData(SHOW_FLOW_ACTION_Y);
    showFlowAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_FLOW_ACTION_Y));
	connect(showFlowAction,SIGNAL(triggered()),viewer,SLOT(goToFlowSwitch()));

    showEditShortcutsAction = new QAction(tr("Edit shortcuts"),this);
    showEditShortcutsAction->setData(SHOW_EDIT_SHORTCUTS_ACTION_Y);
    showEditShortcutsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_EDIT_SHORTCUTS_ACTION_Y));
    connect(showEditShortcutsAction,SIGNAL(triggered()),editShortcutsDialog,SLOT(show()));
}

void MainWindowViewer::createToolBars()
{
#ifdef Q_OS_MAC
    comicToolBar = new YACReaderMacOSXToolbar(this);
#else
	comicToolBar = addToolBar(tr("&File"));
#endif

#ifdef Q_OS_MAC
    //comicToolBar->setIconSize(QSize(16,16));
#else
	comicToolBar->setIconSize(QSize(18,18));
    comicToolBar->setStyleSheet("QToolBar{border:none;}");
#endif

#ifdef Q_OS_MAC
    comicToolBar->addAction(openAction);
    comicToolBar->addAction(openFolderAction);
#else
	QToolButton * tb = new QToolButton();
	tb->addAction(openAction);
	tb->addAction(openFolderAction);
	tb->setPopupMode(QToolButton::MenuButtonPopup);
	tb->setDefaultAction(openAction);

	comicToolBar->addWidget(tb);
#endif
	comicToolBar->addAction(saveImageAction);
	comicToolBar->addAction(openPreviousComicAction);
	comicToolBar->addAction(openNextComicAction);

	comicToolBar->addSeparator();

	comicToolBar->addAction(prevAction);
	comicToolBar->addAction(nextAction);
    comicToolBar->addAction(goToPageAction);

//#ifndef Q_OS_MAC
//	comicToolBar->addSeparator();
//	comicToolBar->addAction(alwaysOnTopAction);
//#else
//	alwaysOnTopAction->setEnabled(false);
//#endif


	comicToolBar->addSeparator();

	//QWidget * widget = new QWidget();

	//QToolButton * tbW = new QToolButton(widget);
	//tbW->addAction(adjustWidth);
	//tbW->setPopupMode(QToolButton::MenuButtonPopup);
	//tbW->setDefaultAction(adjustWidth);

	//QHBoxLayout *layout = new QHBoxLayout;
	//layout->addWidget(tbW);
	//layout->setContentsMargins(0,0,0,0);
	//widget->setLayout(layout);
	//widget->setContentsMargins(0,0,0,0);

	//comicToolBar->addWidget(widget);

	//comicToolBar->addAction(adjustWidth);


#ifdef Q_OS_MAC

    sliderAction = new YACReaderSlider(this);
    sliderAction->hide();

    comicToolBar->addAction(adjustWidthAction);

    QAction * action = comicToolBar->addFitToWidthSlider(adjustWidthAction);

    connect(action,SIGNAL(triggered()),this,SLOT(toggleFitToWidthSlider()));

#else
	QMenu * menu = new QMenu();

    sliderAction = new YACReaderSliderAction(this);

	menu->setAutoFillBackground(false);
	menu->setStyleSheet(" QMenu {background:transparent; border: 0px;padding: 0px; }"
		);
	menu->addAction(sliderAction);
		QToolButton * tb2 = new QToolButton();
    tb2->addAction(adjustWidthAction);
	tb2->setMenu(menu);

	//tb2->addAction();
	tb2->setPopupMode(QToolButton::MenuButtonPopup);
    tb2->setDefaultAction(adjustWidthAction);
    comicToolBar->addWidget(tb2);
#endif

    connect(sliderAction,SIGNAL(fitToWidthRatioChanged(float)),viewer,SLOT(updateFitToWidthRatio(float)));
    connect(optionsDialog,SIGNAL(fitToWidthRatioChanged(float)),sliderAction,SLOT(updateFitToWidthRatio(float)));

    comicToolBar->addAction(adjustHeightAction);
	comicToolBar->addAction(adjustToFullSizeAction);
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
	comicToolBar->addWidget(new QToolBarStretch());
#endif

	
	comicToolBar->addAction(showShorcutsAction);
	comicToolBar->addAction(optionsAction);
	comicToolBar->addAction(helpAboutAction);
	//comicToolBar->addAction(closeAction);

#ifndef Q_OS_MAC
	comicToolBar->setMovable(false);
#endif

	viewer->addAction(openAction);
	viewer->addAction(openFolderAction);
	viewer->addAction(saveImageAction);
	viewer->addAction(openPreviousComicAction);
	viewer->addAction(openNextComicAction);
    YACReader::addSperator(viewer);

	viewer->addAction(prevAction);
	viewer->addAction(nextAction);
    viewer->addAction(goToPageAction);
    viewer->addAction(adjustHeightAction);
    viewer->addAction(adjustWidthAction);
	viewer->addAction(adjustToFullSizeAction);
	viewer->addAction(leftRotationAction);
	viewer->addAction(rightRotationAction);
    viewer->addAction(doublePageAction);
    viewer->addAction(doubleMangaPageAction);
    YACReader::addSperator(viewer);

    viewer->addAction(showMagnifyingGlassAction);
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

    //MacOSX app menus
#ifdef Q_OS_MAC
    QMenuBar * menuBar = this->menuBar();
    //about / preferences
    //TODO

    //file
    QMenu * fileMenu = new QMenu(tr("File"));

    fileMenu->addAction(openAction);
    fileMenu->addAction(openFolderAction);
    fileMenu->addSeparator();
    fileMenu->addAction(saveImageAction);

    //tool bar
    //QMenu * toolbarMenu = new QMenu(tr("Toolbar"));
    //toolbarMenu->addAction();
    //TODO

    menuBar->addMenu(fileMenu);
    //menu->addMenu(toolbarMenu);

    //attach toolbar

    comicToolBar->attachToWindow(this->windowHandle());

#endif

}

void MainWindowViewer::reloadOptions()
{
	viewer->updateConfig(settings);
}

void MainWindowViewer::open()
{
	QFileDialog openDialog;
#ifndef use_unarr
	QString pathFile = openDialog.getOpenFileName(this,tr("Open Comic"),currentDirectory,tr("Comic files") + "(*.cbr *.cbz *.rar *.zip *.tar *.pdf *.7z *.cb7 *.arj *.cbt)");
#else
	QString pathFile = openDialog.getOpenFileName(this,tr("Open Comic"),currentDirectory,tr("Comic files") + "(*.cbr *.cbz *.rar *.zip *.tar *.pdf *.cbt)");
#endif
	if (!pathFile.isEmpty())
	{
		openComicFromPath(pathFile);
	}
}

void MainWindowViewer::open(QString path, ComicDB & comic, QList<ComicDB> & siblings)
{
	//currentComicDB = comic;
	//siblingComics = siblings;

	QFileInfo fi(path);

    if(!comic.info.title.isNull() && !comic.info.title.toString().isEmpty())
        setWindowTitle("YACReader - " + comic.info.title.toString());
	else
		setWindowTitle("YACReader - " + fi.fileName());

	viewer->open(path,comic);
	enableActions();
	int index = siblings.indexOf(comic);

	optionsDialog->setFilters(currentComicDB.info.brightness, currentComicDB.info.contrast, currentComicDB.info.gamma);

	if(index>0)
		openPreviousComicAction->setDisabled(false);
	else
		openPreviousComicAction->setDisabled(true);

	if(index+1<siblings.count())
		openNextComicAction->setDisabled(false);
	else
		openNextComicAction->setDisabled(true);
}

void MainWindowViewer::open(QString path, qint64 comicId, qint64 libraryId)
{
	//QString pathFile = QCoreApplication::arguments().at(1);
	currentDirectory = path;
	//quint64 comicId = QCoreApplication::arguments().at(2).split("=").at(1).toULongLong();
	//libraryId = QCoreApplication::arguments().at(3).split("=").at(1).toULongLong();
	this->libraryId=libraryId;
//	this->path=path;
	
	enableActions();
	
	currentComicDB.id = comicId;
	YACReaderLocalClient client;
        int tries = 1;
		bool success = false;
        while(!(success = client.requestComicInfo(libraryId,currentComicDB,siblingComics)) && tries != 0)
            tries--;

        if(success)
	{
		isClient = true;
		open(path+currentComicDB.path,currentComicDB,siblingComics);
	}
	else
	{
		isClient = false; 
		QMessageBox::information(this,"Connection Error", "Unable to connect to YACReaderLibrary");
		//error
	}

	optionsDialog->setFilters(currentComicDB.info.brightness, currentComicDB.info.contrast, currentComicDB.info.gamma);
}

void MainWindowViewer::openComicFromPath(QString pathFile)
{
    openComic(pathFile);
    isClient = false; //this method is used for direct openings
}

//isClient shouldn't be modified when a siblinig comic is opened
void MainWindowViewer::openSiblingComic(QString pathFile)
{
    openComic(pathFile);
}

void MainWindowViewer::openComic(QString pathFile)
{
    QFileInfo fi(pathFile);
    currentDirectory = fi.dir().absolutePath();
    getSiblingComics(fi.absolutePath(),fi.fileName());

    setWindowTitle("YACReader - " + fi.fileName());

    enableActions();

    viewer->open(pathFile);
 }

void MainWindowViewer::openFolder()
{
	QFileDialog openDialog;
	QString pathDir = openDialog.getExistingDirectory(this,tr("Open folder"),currentDirectory);
	if (!pathDir.isEmpty())
	{
		openFolderFromPath(pathDir);
		isClient = false;
	}
}

void MainWindowViewer::openFolderFromPath(QString pathDir)
{
	currentDirectory = pathDir; //TODO ??
	QFileInfo fi(pathDir);
	getSiblingComics(fi.absolutePath(),fi.fileName());

	setWindowTitle("YACReader - " + fi.fileName());

	enableActions();

	viewer->open(pathDir);
}

void MainWindowViewer::openFolderFromPath(QString pathDir, QString atFileName)
{
	currentDirectory = pathDir; //TODO ??
	QFileInfo fi(pathDir);
	getSiblingComics(fi.absolutePath(),fi.fileName());

	setWindowTitle("YACReader - " + fi.fileName());

	enableActions();

	QDir d(pathDir);
	d.setFilter(QDir::Files|QDir::NoDotAndDotDot);
	d.setNameFilters(Comic::getSupportedImageFormats());
	d.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
	QStringList list = d.entryList();

	qSort(list.begin(),list.end(),naturalSortLessThanCI);
	int i = 0;
	foreach(QString path,list)
	{
		if(path.endsWith(atFileName))
			break;
		i++;
	}

	int index = 0;
	if(i < list.count())
		index = i;

	viewer->open(pathDir,i);	
}

void MainWindowViewer::saveImage()
{
	QFileDialog saveDialog;
	QString pathFile = saveDialog.getSaveFileName(this,tr("Save current page"),currentDirectoryImgDest+"/"+tr("page_%1.jpg").arg(viewer->getIndex()),tr("Image files (*.jpg)"));
	if (!pathFile.isEmpty())
	{
		QFileInfo fi(pathFile);
		currentDirectoryImgDest = fi.absolutePath();
		const QPixmap * p = viewer->pixmap();
		if(p!=NULL)
			p->save(pathFile);
	}
}

void MainWindowViewer::enableActions()
{
	saveImageAction->setDisabled(false);
	prevAction->setDisabled(false);
	nextAction->setDisabled(false);
    adjustHeightAction->setDisabled(false);
    adjustWidthAction->setDisabled(false);
    goToPageAction->setDisabled(false);
	//alwaysOnTopAction->setDisabled(false);
	leftRotationAction->setDisabled(false);
	rightRotationAction->setDisabled(false);
    showMagnifyingGlassAction->setDisabled(false);
	doublePageAction->setDisabled(false);
	doubleMangaPageAction->setDisabled(false);
	adjustToFullSizeAction->setDisabled(false);
	//setBookmark->setDisabled(false);
    showBookmarksAction->setDisabled(false);
    showInfoAction->setDisabled(false); //TODO enable goTo and showInfo (or update) when numPages emited
	showDictionaryAction->setDisabled(false);
	showFlowAction->setDisabled(false);
}
void MainWindowViewer::disableActions()
{
	saveImageAction->setDisabled(true);
	prevAction->setDisabled(true);
	nextAction->setDisabled(true);
    adjustHeightAction->setDisabled(true);
    adjustWidthAction->setDisabled(true);
    goToPageAction->setDisabled(true);
	//alwaysOnTopAction->setDisabled(true);
	leftRotationAction->setDisabled(true);
	rightRotationAction->setDisabled(true);
    showMagnifyingGlassAction->setDisabled(true);
	doublePageAction->setDisabled(true);
	doubleMangaPageAction->setDisabled(true);
	adjustToFullSizeAction->setDisabled(true);
    setBookmarkAction->setDisabled(true);
    showBookmarksAction->setDisabled(true);
    showInfoAction->setDisabled(true); //TODO enable goTo and showInfo (or update) when numPages emited
	openPreviousComicAction->setDisabled(true);
	openNextComicAction->setDisabled(true);
	showDictionaryAction->setDisabled(true);
	showFlowAction->setDisabled(true);
}

void MainWindowViewer::keyPressEvent(QKeyEvent *event)
{
    //TODO remove unused keys
    int _key = event->key();
    Qt::KeyboardModifiers modifiers = event->modifiers();

    if(modifiers & Qt::ShiftModifier)
        _key |= Qt::SHIFT;
    if (modifiers & Qt::ControlModifier)
        _key |= Qt::CTRL;
    if (modifiers & Qt::MetaModifier)
        _key |= Qt::META;
    if (modifiers & Qt::AltModifier)
        _key |= Qt::ALT;

    QKeySequence key(_key);

    if (key == ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_FULL_SCREEN_ACTION_Y))
    {
        toggleFullScreen();
        event->accept();
    }
    else if (key == ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_TOOL_BARS_ACTION_Y))
    {
        toggleToolBars();
        event->accept();
    }
    else if (key == ShortcutsManager::getShortcutsManager().getShortcut(CHANGE_FIT_ACTION_Y))
    {
        changeFit();
        event->accept();
    }
    else
        QWidget::keyPressEvent(event);
}

void MainWindowViewer::mouseDoubleClickEvent ( QMouseEvent * event )
{
	toggleFullScreen();
	event->accept();
}

void MainWindowViewer::toggleFullScreen()
{
	fullscreen?toNormal():toFullScreen();
	Configuration::getConfiguration().setFullScreen(fullscreen = !fullscreen);
}

void MainWindowViewer::toFullScreen()
{
    fromMaximized = this->isMaximized();

    hideToolBars();
    viewer->hide();
    viewer->fullscreen = true;//TODO, change by the right use of windowState();
    showFullScreen();
    viewer->show();
    if(viewer->magnifyingGlassIsVisible())
        viewer->showMagnifyingGlass();
}

void MainWindowViewer::toNormal()
{
    //show all
    viewer->hide();
    viewer->fullscreen = false;//TODO, change by the right use of windowState();
    //viewer->hideMagnifyingGlass();
    if(fromMaximized)
        showMaximized();
    else
        showNormal();

    if(Configuration::getConfiguration().getShowToolbars())
        showToolBars();
    viewer->show();
    if(viewer->magnifyingGlassIsVisible())
        viewer->showMagnifyingGlass();
}

void MainWindowViewer::toggleToolBars()
{
	toolbars?hideToolBars():showToolBars();

	Configuration::getConfiguration().setShowToolbars(toolbars);
#ifndef Q_OS_MAC
	comicToolBar->setMovable(false);
#endif
}
void MainWindowViewer::hideToolBars()
{
	//hide all
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
	Configuration & conf = Configuration::getConfiguration();
	if(!conf.getAdjustToWidth())
	{
		conf.setAdjustToWidth(true);
		viewer->updatePage();
	}
}
void MainWindowViewer::fitToHeight()
{
	Configuration & conf = Configuration::getConfiguration();
	if(conf.getAdjustToWidth())
	{
		conf.setAdjustToWidth(false);
		viewer->updatePage();
	}
}

void MainWindowViewer::checkNewVersion()
{
	Configuration & conf = Configuration::getConfiguration();
	QDate lastCheck = conf.getLastVersionCheck();
	QDate current = QDate::currentDate();
	if(lastCheck.isNull() || lastCheck.daysTo(current) >= conf.getNumDaysBetweenVersionChecks())
	{
		versionChecker = new HttpVersionChecker();

		connect(versionChecker,SIGNAL(newVersionDetected()),
			this,SLOT(newVersion()));

		QTimer * tT = new QTimer;
		tT->setSingleShot(true);
		connect(tT, SIGNAL(timeout()), versionChecker, SLOT(get()));
		//versionChecker->get(); //TOD�
		tT->start(100);

		conf.setLastVersionCheck(current);
    }
}

void MainWindowViewer::processReset()
{
    if(isClient)
    {
        if(siblingComics.count()>1)
        {
            bool openNextB = openNextComicAction->isEnabled();
            bool openPrevB = openPreviousComicAction->isEnabled();
            disableActions();
            openNextComicAction->setEnabled(openNextB);
            openPreviousComicAction->setEnabled(openPrevB);
        }
        else
            disableActions();
    }
    else
        disableActions();
}

void MainWindowViewer::setUpShortcutsManagement()
{
    //actions holder
    QObject * orphanActions = new QObject;

    QList<QAction *> allActions;
    QList<QAction *> tmpList;


    editShortcutsDialog->addActionsGroup(tr("Comics"),QIcon(":/images/shortcuts_group_comics.png"),
                                         tmpList = QList<QAction *>()
                                         << openAction
                                         << openFolderAction
                                         << saveImageAction
                                         << openPreviousComicAction
                                         << openNextComicAction);

    allActions << tmpList;

    //keys without actions (General)
    QAction * toggleFullScreenAction = new QAction(tr("Toggle fullscreen mode"),orphanActions);
    toggleFullScreenAction->setData(TOGGLE_FULL_SCREEN_ACTION_Y);
    toggleFullScreenAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_FULL_SCREEN_ACTION_Y));

    QAction * toggleToolbarsAction = new QAction(tr("Hide/show toolbar"),orphanActions);
    toggleToolbarsAction->setData(TOGGLE_TOOL_BARS_ACTION_Y);
    toggleToolbarsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_TOOL_BARS_ACTION_Y));

    editShortcutsDialog->addActionsGroup(tr("General"),QIcon(":/images/shortcuts_group_general.png"),
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
                                         << showEditShortcutsAction);

    allActions << tmpList;

    //keys without actions (MGlass)
    QAction * sizeUpMglassAction = new QAction(tr("Size up magnifying glass"),orphanActions);
    sizeUpMglassAction->setData(SIZE_UP_MGLASS_ACTION_Y);
    sizeUpMglassAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SIZE_UP_MGLASS_ACTION_Y));

    QAction * sizeDownMglassAction = new QAction(tr("Size down magnifying glass"),orphanActions);
    sizeDownMglassAction->setData(SIZE_DOWN_MGLASS_ACTION_Y);
    sizeDownMglassAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SIZE_DOWN_MGLASS_ACTION_Y));

    QAction * zoomInMglassAction = new QAction(tr("Zoom in magnifying glass"),orphanActions);
    zoomInMglassAction->setData(ZOOM_IN_MGLASS_ACTION_Y);
    zoomInMglassAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ZOOM_IN_MGLASS_ACTION_Y));

    QAction * zoomOutMglassAction = new QAction(tr("Zoom out magnifying glass"),orphanActions);
    zoomOutMglassAction->setData(ZOOM_OUT_MGLASS_ACTION_Y);
    zoomOutMglassAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ZOOM_OUT_MGLASS_ACTION_Y));

    editShortcutsDialog->addActionsGroup(tr("Magnifiying glass"),QIcon(":/images/shortcuts_group_mglass.png"),
                                         tmpList = QList<QAction *>()
                                         << showMagnifyingGlassAction
                                         << sizeUpMglassAction
                                         << sizeDownMglassAction
                                         << zoomInMglassAction
                                         << zoomOutMglassAction);

    allActions << tmpList;

    //keys without actions
    QAction * toggleFitToScreenAction = new QAction(tr("Toggle between fit to width and fit to height"),orphanActions);
    toggleFitToScreenAction->setData(CHANGE_FIT_ACTION_Y);
    toggleFitToScreenAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(CHANGE_FIT_ACTION_Y));

    editShortcutsDialog->addActionsGroup(tr("Page adjustement"),QIcon(":/images/shortcuts_group_page.png"),
                                         tmpList = QList<QAction *>()
                                         << adjustHeightAction
                                         << adjustWidthAction
                                         << toggleFitToScreenAction
                                         << leftRotationAction
                                         << rightRotationAction
                                         << doublePageAction
					 << doubleMangaPageAction
                                         << adjustToFullSizeAction);

    allActions << tmpList;

    QAction * autoScrollForwardAction = new QAction(tr("Autoscroll down"),orphanActions);
    autoScrollForwardAction->setData(AUTO_SCROLL_FORWARD_ACTION_Y);
    autoScrollForwardAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(AUTO_SCROLL_FORWARD_ACTION_Y));

    QAction * autoScrollBackwardAction = new QAction(tr("Autoscroll up"),orphanActions);
    autoScrollBackwardAction->setData(AUTO_SCROLL_BACKWARD_ACTION_Y);
    autoScrollBackwardAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(AUTO_SCROLL_BACKWARD_ACTION_Y));

    QAction * moveDownAction = new QAction(tr("Move down"),orphanActions);
    moveDownAction->setData(MOVE_DOWN_ACTION_Y);
    moveDownAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(MOVE_DOWN_ACTION_Y));

    QAction * moveUpAction = new QAction(tr("Move up"),orphanActions);
    moveUpAction->setData(MOVE_UP_ACTION_Y);
    moveUpAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(MOVE_UP_ACTION_Y));

    QAction * moveLeftAction = new QAction(tr("Move left"),orphanActions);
    moveLeftAction->setData(MOVE_LEFT_ACTION_Y);
    moveLeftAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(MOVE_LEFT_ACTION_Y));

    QAction * moveRightAction = new QAction(tr("Move right"),orphanActions);
    moveRightAction->setData(MOVE_RIGHT_ACTION_Y);
    moveRightAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(MOVE_RIGHT_ACTION_Y));

    QAction * goToFirstPageAction = new QAction(tr("Go to the first page"),orphanActions);
    goToFirstPageAction->setData(GO_TO_FIRST_PAGE_ACTION_Y);
    goToFirstPageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(GO_TO_FIRST_PAGE_ACTION_Y));

    QAction * goToLastPageAction = new QAction(tr("Go to the last page"),orphanActions);
    goToLastPageAction->setData(GO_TO_LAST_PAGE_ACTION_Y);
    goToLastPageAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(GO_TO_LAST_PAGE_ACTION_Y));

    editShortcutsDialog->addActionsGroup(tr("Reading"),QIcon(":/images/shortcuts_group_reading.png"),
                                         tmpList = QList<QAction *>()
                                         << nextAction
                                         << prevAction
                                         << setBookmarkAction
                                         << showBookmarksAction
                                         << autoScrollForwardAction
                                         << autoScrollBackwardAction
                                         << moveDownAction
                                         << moveUpAction
                                         << moveLeftAction
                                         << moveRightAction
                                         << goToFirstPageAction
                                         << goToLastPageAction
                                         << goToPageAction);

    allActions << tmpList;

    ShortcutsManager::getShortcutsManager().registerActions(allActions);

}

#ifdef Q_OS_MAC
void MainWindowViewer::toggleFitToWidthSlider()
{
    if(sliderAction->isVisible())
    {
        sliderAction->hide();
    }
    else
    {
        sliderAction->move(250,0);
        sliderAction->show();
    }
}
#endif

void MainWindowViewer::changeFit()
{
	Configuration & conf = Configuration::getConfiguration();
	conf.setAdjustToWidth(!conf.getAdjustToWidth());
	viewer->updatePage();
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

	switch(ret)
	{
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

void MainWindowViewer::closeEvent ( QCloseEvent * event )
{
	Q_UNUSED(event)

	if(isClient)
		sendComic();

	viewer->save();
	Configuration & conf = Configuration::getConfiguration();
	if(!fullscreen && !isMaximized())
	{
		conf.setPos(pos());
		conf.setSize(size());
	}
	conf.setMaximized(isMaximized());
	
	emit (closed());
}

void MainWindowViewer::openPreviousComic()
{
	if(!siblingComics.isEmpty() && isClient)
	{
		sendComic();

		int currentIndex = siblingComics.indexOf(currentComicDB);
		if (currentIndex == -1)
			return;
		if(currentIndex-1 >= 0 && currentIndex-1 < siblingComics.count())
		{
			siblingComics[currentIndex] = currentComicDB; //updated
			currentComicDB = siblingComics.at(currentIndex-1);
			open(currentDirectory+currentComicDB.path,currentComicDB,siblingComics);
		}
		return;
	}
	if(!previousComicPath.isEmpty())
	{
        openSiblingComic(previousComicPath);
	}
}

void MainWindowViewer::openNextComic()
{
	if(!siblingComics.isEmpty() && isClient)
	{
		sendComic();

		int currentIndex = siblingComics.indexOf(currentComicDB);
		if (currentIndex == -1)
			return;
		if(currentIndex+1 > 0 && currentIndex+1 < siblingComics.count())
		{
			siblingComics[currentIndex] = currentComicDB; //updated
			currentComicDB = siblingComics.at(currentIndex+1);
			open(currentDirectory+currentComicDB.path,currentComicDB,siblingComics);
		}
		return;
	}
	if(!nextComicPath.isEmpty())
	{
        openSiblingComic(nextComicPath);
	}
}

void MainWindowViewer::getSiblingComics(QString path,QString currentComic)
{
	QDir d(path);
	d.setFilter(QDir::Files|QDir::NoDotAndDotDot);
#ifndef use_unarr
	d.setNameFilters(QStringList() << "*.cbr" << "*.cbz" << "*.rar" << "*.zip" << "*.tar" << "*.pdf" << "*.7z" << "*.cb7" << "*.arj" << "*.cbt");
#else
	d.setNameFilters(QStringList() << "*.cbr" << "*.cbz" << "*.rar" << "*.zip" << "*.tar" << "*.pdf" << "*.cbt");
#endif
	d.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
	QStringList list = d.entryList();
	qSort(list.begin(),list.end(),naturalSortLessThanCI);
	//std::sort(list.begin(),list.end(),naturalSortLessThanCI);
	int index = list.indexOf(currentComic);
		if(index == -1) //comic not found
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
	if(index>0)
	{
		previousComicPath = path+"/"+list.at(index-1);
		openPreviousComicAction->setDisabled(false);
	}
	else
		openPreviousComicAction->setDisabled(true);

	if(index+1<list.count())
	{
		nextComicPath = path+"/"+list.at(index+1);
		openNextComicAction->setDisabled(false);
	}
	else
		openNextComicAction->setDisabled(true);
}

void MainWindowViewer::dropEvent(QDropEvent *event)
{
	QList<QUrl> urlList;
	QString fName;
	QFileInfo info;
 
	if (event->mimeData()->hasUrls())
	{
		urlList = event->mimeData()->urls();
	
		if ( urlList.size() > 0 )
		{
			fName = urlList[0].toLocalFile(); // convert first QUrl to local path
			info.setFile( fName ); // information about file
			if (info.isFile()) 
			{
				QStringList imageSuffixs = Comic::getSupportedImageLiteralFormats();
                if(imageSuffixs.contains(info.suffix())) //image dropped
					openFolderFromPath(info.absoluteDir().absolutePath(),info.fileName());
				else
					openComicFromPath(fName); // if is file, setText
			}
			else 
				if(info.isDir())
					openFolderFromPath(fName);

			isClient = false;
		}
	}

	event->acceptProposedAction();
}
void MainWindowViewer::dragEnterEvent(QDragEnterEvent *event)
{
	// accept just text/uri-list mime format
	if (event->mimeData()->hasFormat("text/uri-list")) 
	{     
		event->acceptProposedAction();
		isClient = false;
	}
}

void MainWindowViewer::alwaysOnTopSwitch()
{
	if(!Configuration::getConfiguration().getAlwaysOnTop())
	{
		setWindowFlags(this->windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint); //always on top
		show();
	}
	else
	{
		setWindowFlags(this->windowFlags() ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
		show();
	}
	Configuration::getConfiguration().setAlwaysOnTop(!Configuration::getConfiguration().getAlwaysOnTop());
}

void MainWindowViewer::adjustToFullSizeSwitch()
{
	Configuration::getConfiguration().setAdjustToFullSize(!Configuration::getConfiguration().getAdjustToFullSize());
	viewer->updatePage();
}

void MainWindowViewer::sendComic()
{
	YACReaderLocalClient  * client = new YACReaderLocalClient;
	currentComicDB.info.hasBeenOpened = true;
	viewer->updateComic(currentComicDB);
    int retries = 1;
    while(!client->sendComicInfo(libraryId,currentComicDB) && retries!=0)
        retries--;
	connect(client,SIGNAL(finished()),client,SLOT(deleteLater()));
	//delete client;
}
