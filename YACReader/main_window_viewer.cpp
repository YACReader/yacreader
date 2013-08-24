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

#include <ctime>
#include <algorithm>

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
#endif

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
	delete adjustHeight;
	delete adjustWidth;
	delete leftRotationAction;
	delete rightRotationAction;
	delete doublePageAction;
	delete goToPage;
	delete optionsAction;
	delete helpAboutAction;
	delete showMagnifyingGlass;
	delete setBookmark;
	delete showBookmarks;
	delete showShorcutsAction;
	delete showInfo;
	delete closeAction;
	delete showDictionaryAction;
	delete alwaysOnTopAction;
	delete adjustToFullSizeAction;
	delete showFlowAction;

}
void MainWindowViewer::loadConfiguration()
{
	settings = new QSettings(QCoreApplication::applicationDirPath()+"/YACReader.ini",QSettings::IniFormat);

	Configuration & config = Configuration::getConfiguration();
	config.load(settings);
	currentDirectory = config.getDefaultPath();
	fullscreen = config.getFullScreen();
}

void MainWindowViewer::setupUI()
{
	setWindowIcon(QIcon(":/images/icon.png"));

	setUnifiedTitleAndToolBarOnMac(true);

	viewer = new Viewer(this);
	connect(viewer,SIGNAL(reset()),this,SLOT(disableActions()));

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
	shortcutsDialog = new ShortcutsDialog(this);

	createActions();
	createToolBars();

	setWindowTitle("YACReader");

	openFromArgv();

	versionChecker = new HttpVersionChecker();

	connect(versionChecker,SIGNAL(newVersionDetected()),
		this,SLOT(newVersion()));
	
	QTimer * tT = new QTimer;
	
	tT->setSingleShot(true);
	connect(tT, SIGNAL(timeout()), versionChecker, SLOT(get()));
	//versionChecker->get(); //TODÓ
	tT->start(100);
	
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

void MainWindowViewer::openFromArgv()
{
	if(QCoreApplication::argc() == 2) //only path...
	{
		isClient = false;
		//TODO: new method open(QString)
		QString pathFile = QCoreApplication::arguments().at(1);
		QFileInfo fi(pathFile);
		currentDirectory = fi.absoluteDir().path();
		getSiblingComics(fi.absolutePath(),fi.fileName());

		setWindowTitle("YACReader - " + fi.fileName());
		enableActions();
		viewer->open(pathFile);
	}
	else if(QCoreApplication::argc() == 4)
	{
		
		QString pathFile = QCoreApplication::arguments().at(1);
		currentDirectory = pathFile;
		quint64 comicId = QCoreApplication::arguments().at(2).toULongLong();
		libraryId = QCoreApplication::arguments().at(3).toULongLong();
		
		enableActions();
	
		currentComicDB.id = comicId;
		YACReaderLocalClient client;
		/*int tries = 0;
		bool success = false;
		while(!(success = client.requestComicInfo(libraryId,currentComicDB,siblingComics)) && tries < 3)
		{
			tries++;
		}*/
		if(client.requestComicInfo(libraryId,currentComicDB,siblingComics))
		{
			isClient = true;
			open(pathFile+currentComicDB.path,currentComicDB,siblingComics);
		}
		else
		{isClient = false;/*error*/}

		optionsDialog->setFilters(currentComicDB.info.brightness, currentComicDB.info.contrast, currentComicDB.info.gamma);
	}
}

void MainWindowViewer::createActions()
{
	openAction = new QAction(tr("&Open"),this);
	openAction->setShortcut(tr("O"));
	openAction->setIcon(QIcon(":/images/viewer_toolbar/open.png"));
	openAction->setToolTip(tr("Open a comic"));
	connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

	openFolderAction = new QAction(tr("Open Folder"),this);
	openFolderAction->setShortcut(tr("Ctrl+O"));
	openFolderAction->setIcon(QIcon(":/images/viewer_toolbar/openFolder.png"));
	openFolderAction->setToolTip(tr("Open image folder"));
	connect(openFolderAction, SIGNAL(triggered()), this, SLOT(openFolder()));

	saveImageAction = new QAction(tr("Save"),this);
	saveImageAction->setIcon(QIcon(":/images/viewer_toolbar/save.png"));
	saveImageAction->setToolTip(tr("Save current page"));
	saveImageAction->setDisabled(true);
	connect(saveImageAction,SIGNAL(triggered()),this,SLOT(saveImage()));

	openPreviousComicAction = new QAction(tr("Previous Comic"),this);
	openPreviousComicAction->setIcon(QIcon(":/images/viewer_toolbar/openPrevious.png"));
	openPreviousComicAction->setShortcut(Qt::CTRL + Qt::Key_Left);
	openPreviousComicAction->setToolTip(tr("Open previous comic"));
	openPreviousComicAction->setDisabled(true);
	connect(openPreviousComicAction,SIGNAL(triggered()),this,SLOT(openPreviousComic()));

	openNextComicAction = new QAction(tr("Next Comic"),this);
	openNextComicAction->setIcon(QIcon(":/images/viewer_toolbar/openNext.png"));
	openNextComicAction->setShortcut(Qt::CTRL + Qt::Key_Right);
	openNextComicAction->setToolTip(tr("Open next comic"));
	openNextComicAction->setDisabled(true);
	connect(openNextComicAction,SIGNAL(triggered()),this,SLOT(openNextComic()));

	prevAction = new QAction(tr("&Previous"),this);
	prevAction->setIcon(QIcon(":/images/viewer_toolbar/previous.png"));
	prevAction->setShortcut(Qt::Key_Left);
	prevAction->setToolTip(tr("Go to previous page"));
	prevAction->setDisabled(true);
	connect(prevAction, SIGNAL(triggered()),viewer,SLOT(prev()));

	nextAction = new QAction(tr("&Next"),this);
	nextAction->setIcon(QIcon(":/images/viewer_toolbar/next.png"));
	nextAction->setShortcut(Qt::Key_Right);
	nextAction->setToolTip(tr("Go to next page"));
	nextAction->setDisabled(true);
	connect(nextAction, SIGNAL(triggered()),viewer,SLOT(next()));

	adjustHeight = new QAction(tr("Fit Width"),this);
	adjustHeight->setIcon(QIcon(":/images/viewer_toolbar/toHeight.png"));
	//adjustWidth->setCheckable(true);
	adjustHeight->setDisabled(true);
	adjustHeight->setChecked(Configuration::getConfiguration().getAdjustToWidth());
	adjustHeight->setToolTip(tr("Fit image to height"));
	//adjustWidth->setIcon(QIcon(":/images/fitWidth.png"));
	connect(adjustHeight, SIGNAL(triggered()),this,SLOT(fitToHeight()));

	adjustWidth = new QAction(tr("Fit Width"),this);
	adjustWidth->setIcon(QIcon(":/images/viewer_toolbar/toWidth.png"));
	//adjustWidth->setCheckable(true);
	adjustWidth->setDisabled(true);
	adjustWidth->setChecked(Configuration::getConfiguration().getAdjustToWidth());
	adjustWidth->setToolTip(tr("Fit image to width"));
	//adjustWidth->setIcon(QIcon(":/images/fitWidth.png"));
	connect(adjustWidth, SIGNAL(triggered()),this,SLOT(fitToWidth()));

	leftRotationAction = new QAction(tr("Rotate image to the left"),this);
	leftRotationAction->setShortcut(tr("L"));
	leftRotationAction->setIcon(QIcon(":/images/viewer_toolbar/rotateL.png"));
	leftRotationAction->setDisabled(true);
	connect(leftRotationAction, SIGNAL(triggered()),viewer,SLOT(rotateLeft()));

	rightRotationAction = new QAction(tr("Rotate image to the right"),this);
	rightRotationAction->setShortcut(tr("R"));
	rightRotationAction->setIcon(QIcon(":/images/viewer_toolbar/rotateR.png"));
	rightRotationAction->setDisabled(true);
	connect(rightRotationAction, SIGNAL(triggered()),viewer,SLOT(rotateRight()));

	doublePageAction = new QAction(tr("Double page mode"),this);
	doublePageAction->setToolTip(tr("Switch to double page mode"));
	doublePageAction->setShortcut(tr("D"));
	doublePageAction->setIcon(QIcon(":/images/viewer_toolbar/doublePage.png"));
	doublePageAction->setDisabled(true);
	doublePageAction->setCheckable(true);
	doublePageAction->setChecked(Configuration::getConfiguration().getDoublePage());
	connect(doublePageAction, SIGNAL(triggered()),viewer,SLOT(doublePageSwitch()));

	goToPage = new QAction(tr("Go To"),this);
	goToPage->setShortcut(tr("G"));
	goToPage->setIcon(QIcon(":/images/viewer_toolbar/goto.png"));
	goToPage->setDisabled(true);
	goToPage->setToolTip(tr("Go to page ..."));
	connect(goToPage, SIGNAL(triggered()),viewer,SLOT(showGoToDialog()));

	optionsAction = new QAction(tr("Options"),this);
	optionsAction->setShortcut(tr("C"));
	optionsAction->setToolTip(tr("YACReader options"));
	optionsAction->setIcon(QIcon(":/images/viewer_toolbar/options.png"));

	connect(optionsAction, SIGNAL(triggered()),optionsDialog,SLOT(show()));

	helpAboutAction = new QAction(tr("Help"),this);
	helpAboutAction->setToolTip(tr("Help, About YACReader"));
	helpAboutAction->setShortcut(Qt::Key_F1);
	helpAboutAction->setIcon(QIcon(":/images/viewer_toolbar/help.png"));
	connect(helpAboutAction, SIGNAL(triggered()),had,SLOT(show()));

	showMagnifyingGlass = new QAction(tr("Magnifying glass"),this);
	showMagnifyingGlass->setToolTip(tr("Switch Magnifying glass"));
	showMagnifyingGlass->setShortcut(tr("Z"));
	showMagnifyingGlass->setIcon(QIcon(":/images/viewer_toolbar/magnifyingGlass.png"));
	showMagnifyingGlass->setDisabled(true);
	showMagnifyingGlass->setCheckable(true);
	connect(showMagnifyingGlass, SIGNAL(triggered()),viewer,SLOT(magnifyingGlassSwitch()));

	setBookmark = new QAction(tr("Set bookmark"),this);
	setBookmark->setToolTip(tr("Set a bookmark on the current page"));
	setBookmark->setShortcut(Qt::CTRL+Qt::Key_M);
	setBookmark->setIcon(QIcon(":/images/viewer_toolbar/bookmark.png"));
	setBookmark->setDisabled(true);
	setBookmark->setCheckable(true);
	connect(setBookmark,SIGNAL(triggered (bool)),viewer,SLOT(setBookmark(bool)));
	connect(viewer,SIGNAL(pageAvailable(bool)),setBookmark,SLOT(setEnabled(bool)));
	connect(viewer,SIGNAL(pageIsBookmark(bool)),setBookmark,SLOT(setChecked(bool)));

	showBookmarks = new QAction(tr("Show bookmarks"),this);
	showBookmarks->setToolTip(tr("Show the bookmarks of the current comic"));
	showBookmarks->setShortcut(tr("M"));
	showBookmarks->setIcon(QIcon(":/images/viewer_toolbar/showBookmarks.png"));
	showBookmarks->setDisabled(true);
	connect(showBookmarks, SIGNAL(triggered()),viewer->getBookmarksDialog(),SLOT(show()));

	showShorcutsAction = new QAction(tr("Show keyboard shortcuts"), this );
	showShorcutsAction->setIcon(QIcon(":/images/viewer_toolbar/shortcuts.png"));
	connect(showShorcutsAction, SIGNAL(triggered()),shortcutsDialog,SLOT(show()));

	showInfo = new QAction(tr("Show Info"),this);
	showInfo->setShortcut(tr("I"));
	showInfo->setIcon(QIcon(":/images/viewer_toolbar/info.png"));
	showInfo->setDisabled(true);
	connect(showInfo, SIGNAL(triggered()),viewer,SLOT(informationSwitch()));

	closeAction = new QAction(tr("Close"),this);
	closeAction->setShortcut(Qt::Key_Escape);
	closeAction->setIcon(QIcon(":/images/viewer_toolbar/close.png"));
	connect(closeAction,SIGNAL(triggered()),this,SLOT(close()));

	showDictionaryAction = new QAction(tr("Show Dictionary"),this);
	showDictionaryAction->setShortcut(Qt::Key_T);
	showDictionaryAction->setIcon(QIcon(":/images/viewer_toolbar/translator.png"));
	//showDictionaryAction->setCheckable(true);
	showDictionaryAction->setDisabled(true);
	connect(showDictionaryAction,SIGNAL(triggered()),viewer,SLOT(translatorSwitch()));

	alwaysOnTopAction = new QAction(tr("Always on top"),this);
	alwaysOnTopAction->setShortcut(Qt::Key_Q);
	alwaysOnTopAction->setIcon(QIcon(":/images/alwaysOnTop.png"));
	alwaysOnTopAction->setCheckable(true);
	alwaysOnTopAction->setDisabled(true);
	alwaysOnTopAction->setChecked(Configuration::getConfiguration().getAlwaysOnTop());
	connect(alwaysOnTopAction,SIGNAL(triggered()),this,SLOT(alwaysOnTopSwitch()));

	adjustToFullSizeAction = new QAction(tr("Show full size"),this);
	adjustToFullSizeAction->setShortcut(Qt::Key_W);
	adjustToFullSizeAction->setIcon(QIcon(":/images/viewer_toolbar/full.png"));
	adjustToFullSizeAction->setCheckable(true);
	adjustToFullSizeAction->setDisabled(true);
	adjustToFullSizeAction->setChecked(Configuration::getConfiguration().getAdjustToFullSize());
	connect(adjustToFullSizeAction,SIGNAL(triggered()),this,SLOT(adjustToFullSizeSwitch()));

	showFlowAction = new QAction(tr("Show go to flow"),this);
	showFlowAction->setShortcut(Qt::Key_S);
	showFlowAction->setIcon(QIcon(":/images/viewer_toolbar/flow.png"));
	showFlowAction->setDisabled(true);
	connect(showFlowAction,SIGNAL(triggered()),viewer,SLOT(goToFlowSwitch()));
}

void MainWindowViewer::createToolBars()
{
	comicToolBar = addToolBar(tr("&File"));

	comicToolBar->setStyleSheet("QToolBar{border:none;}");
#ifdef Q_OS_MAC
	comicToolBar->setIconSize(QSize(16,16));
#else
	comicToolBar->setIconSize(QSize(18,18));
#endif

	QToolButton * tb = new QToolButton();
	tb->addAction(openAction);
	tb->addAction(openFolderAction);
	tb->setPopupMode(QToolButton::MenuButtonPopup);
	tb->setDefaultAction(openAction);

	comicToolBar->addWidget(tb);
	comicToolBar->addAction(saveImageAction);
	comicToolBar->addAction(openPreviousComicAction);
	comicToolBar->addAction(openNextComicAction);
#ifdef Q_OS_MAC
	comicToolBar->addWidget(new MacToolBarSeparator);
#else
	comicToolBar->addSeparator();
#endif
	comicToolBar->addAction(prevAction);
	comicToolBar->addAction(nextAction);
	comicToolBar->addAction(goToPage);

//#ifndef Q_OS_MAC
//	comicToolBar->addSeparator();
//	comicToolBar->addAction(alwaysOnTopAction);
//#else
//	alwaysOnTopAction->setEnabled(false);
//#endif

#ifdef Q_OS_MAC
	comicToolBar->addWidget(new MacToolBarSeparator);
#else
	comicToolBar->addSeparator();
#endif

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

	

	QMenu * menu = new QMenu();
	sliderAction = new YACReaderSliderAction(this);
	menu->setAutoFillBackground(false);
	menu->setStyleSheet(" QMenu {background:transparent; border: 0px;padding: 0px; }"
		);
	menu->addAction(sliderAction);
		QToolButton * tb2 = new QToolButton();
	tb2->addAction(adjustWidth);
	tb2->setMenu(menu);

	connect(sliderAction,SIGNAL(fitToWidthRatioChanged(float)),viewer,SLOT(updateFitToWidthRatio(float)));
	connect(optionsDialog,SIGNAL(fitToWidthRatioChanged(float)),sliderAction,SLOT(updateFitToWidthRatio(float)));


	//tb2->addAction();
	tb2->setPopupMode(QToolButton::MenuButtonPopup);
	tb2->setDefaultAction(adjustWidth);
	comicToolBar->addWidget(tb2);
	comicToolBar->addAction(adjustHeight);
	comicToolBar->addAction(adjustToFullSizeAction);
	comicToolBar->addAction(leftRotationAction);
	comicToolBar->addAction(rightRotationAction);
	comicToolBar->addAction(doublePageAction);

#ifdef Q_OS_MAC
	comicToolBar->addWidget(new MacToolBarSeparator);
#else
	comicToolBar->addSeparator();
#endif
	comicToolBar->addAction(showMagnifyingGlass);

#ifdef Q_OS_MAC
	comicToolBar->addWidget(new MacToolBarSeparator);
#else
	comicToolBar->addSeparator();
#endif
	comicToolBar->addAction(setBookmark);
	comicToolBar->addAction(showBookmarks);
	
#ifdef Q_OS_MAC
	comicToolBar->addWidget(new MacToolBarSeparator);
#else
	comicToolBar->addSeparator();
#endif
	comicToolBar->addAction(showDictionaryAction);
	comicToolBar->addAction(showFlowAction);
	comicToolBar->addAction(showInfo);

#ifdef Q_OS_MAC
	comicToolBar->addWidget(new MacToolBarSeparator);
#else
	comicToolBar->addWidget(new QToolBarStretch());
#endif

	
	comicToolBar->addAction(showShorcutsAction);
	comicToolBar->addAction(optionsAction);
	comicToolBar->addAction(helpAboutAction);
	//comicToolBar->addAction(closeAction);

	comicToolBar->setMovable(false);


	viewer->addAction(openAction);
	viewer->addAction(openFolderAction);
	viewer->addAction(saveImageAction);
	viewer->addAction(openPreviousComicAction);
	viewer->addAction(openNextComicAction);
	QAction * separator = new QAction("",this);
	separator->setSeparator(true);
	viewer->addAction(separator);
	viewer->addAction(prevAction);
	viewer->addAction(nextAction);
	viewer->addAction(goToPage);
	viewer->addAction(adjustHeight);
	viewer->addAction(adjustWidth);
	viewer->addAction(adjustToFullSizeAction);
	viewer->addAction(leftRotationAction);
	viewer->addAction(rightRotationAction);
		viewer->addAction(doublePageAction);
	separator = new QAction("",this);
	separator->setSeparator(true);
	viewer->addAction(separator);
	viewer->addAction(showMagnifyingGlass);
	separator = new QAction("",this);
	separator->setSeparator(true);
	viewer->addAction(separator);

	viewer->addAction(setBookmark);
	viewer->addAction(showBookmarks);
	separator = new QAction("",this);
	separator->setSeparator(true);
	viewer->addAction(separator);

	viewer->addAction(showDictionaryAction);
	viewer->addAction(showFlowAction);
	viewer->addAction(showInfo);
	separator = new QAction("",this);
	separator->setSeparator(true);
	viewer->addAction(separator);

	viewer->addAction(showShorcutsAction);
	viewer->addAction(optionsAction);
	viewer->addAction(helpAboutAction);
	separator = new QAction("",this);
	separator->setSeparator(true);
	viewer->addAction(separator);
	viewer->addAction(closeAction);

	viewer->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void MainWindowViewer::reloadOptions()
{
	viewer->updateConfig(settings);
}

void MainWindowViewer::open()
{
	QFileDialog openDialog;
	QString pathFile = openDialog.getOpenFileName(this,tr("Open Comic"),currentDirectory,tr("Comic files") + "(*.cbr *.cbz *.rar *.zip *.tar *.pdf *.7z *.cb7 *.arj *.cbt)");
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

	if(comic.info.title != 0 && !comic.info.title->isEmpty())
		setWindowTitle("YACReader - " + *comic.info.title);
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

void MainWindowViewer::openComicFromPath(QString pathFile)
{
	QFileInfo fi(pathFile);
	currentDirectory = fi.dir().absolutePath();
	getSiblingComics(fi.absolutePath(),fi.fileName());

	setWindowTitle("YACReader - " + fi.fileName());

	enableActions();

	viewer->open(pathFile);

	isClient = false;
	
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
	adjustHeight->setDisabled(false);
	adjustWidth->setDisabled(false);
	goToPage->setDisabled(false);
	//alwaysOnTopAction->setDisabled(false);
	leftRotationAction->setDisabled(false);
	rightRotationAction->setDisabled(false);
	showMagnifyingGlass->setDisabled(false);
	doublePageAction->setDisabled(false);
	adjustToFullSizeAction->setDisabled(false);
	//setBookmark->setDisabled(false);
	showBookmarks->setDisabled(false);
	showInfo->setDisabled(false); //TODO enable goTo and showInfo (or update) when numPages emited
	showDictionaryAction->setDisabled(false);
	showFlowAction->setDisabled(false);
}
void MainWindowViewer::disableActions()
{
	saveImageAction->setDisabled(true);
	prevAction->setDisabled(true);
	nextAction->setDisabled(true);
	adjustHeight->setDisabled(true);
	adjustWidth->setDisabled(true);
	goToPage->setDisabled(true);
	//alwaysOnTopAction->setDisabled(true);
	leftRotationAction->setDisabled(true);
	rightRotationAction->setDisabled(true);
	showMagnifyingGlass->setDisabled(true);
	doublePageAction->setDisabled(true);
	adjustToFullSizeAction->setDisabled(true);
	setBookmark->setDisabled(true);
	showBookmarks->setDisabled(true);
	showInfo->setDisabled(true); //TODO enable goTo and showInfo (or update) when numPages emited
	openPreviousComicAction->setDisabled(true);
	openNextComicAction->setDisabled(true);
	showDictionaryAction->setDisabled(true);
	showFlowAction->setDisabled(true);
}

void MainWindowViewer::keyPressEvent(QKeyEvent *event)
{
	//TODO remove unused keys
	switch (event->key())
	{
	case Qt::Key_Escape:
		this->close();
		break;
	case Qt::Key_F:   
		toggleFullScreen();
		break;
	case Qt::Key_H:   
		toggleToolBars();
		break;
	case Qt::Key_O:   
		open();
		break;
	case Qt::Key_A:   
		changeFit();
		break;
	default:
		QWidget::keyPressEvent(event);
		break;
	}
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

	comicToolBar->setMovable(false);
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
void MainWindowViewer::changeFit()
{
	Configuration & conf = Configuration::getConfiguration();
	conf.setAdjustToWidth(!conf.getAdjustToWidth());
	viewer->updatePage();
}

void MainWindowViewer::newVersion()
{
	QMessageBox msgBox;
	msgBox.setText(tr("There is a new version avaliable"));
	msgBox.setInformativeText(tr("Do you want to download the new version?"));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
	msgBox.setModal(true);
	int ret = msgBox.exec();

	if(ret==QMessageBox::Yes){
		QDesktopServices::openUrl(QUrl("http://www.yacreader.com"));
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
		viewer->open(previousComicPath);
		QFileInfo fi(previousComicPath);
		getSiblingComics(fi.absolutePath(),fi.fileName());

		setWindowTitle("YACReader - " + fi.fileName());
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
		viewer->open(nextComicPath);
		QFileInfo fi(nextComicPath);
		getSiblingComics(fi.absolutePath(),fi.fileName());

		setWindowTitle("YACReader - " + fi.fileName());
	}
}

void MainWindowViewer::getSiblingComics(QString path,QString currentComic)
{
	QDir d(path);
	d.setFilter(QDir::Files|QDir::NoDotAndDotDot);
	d.setNameFilters(QStringList() << "*.cbr" << "*.cbz" << "*.rar" << "*.zip" << "*.tar" << "*.pdf" << "*.7z" << "*.cb7" << "*.arj" << "*.cbt");
	d.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
	QStringList list = d.entryList();
	qSort(list.begin(),list.end(),naturalSortLessThanCI);
	//std::sort(list.begin(),list.end(),naturalSortLessThanCI);
	int index = list.indexOf(currentComic);
		if(index == -1) //comic not found
		{
			QFile f(QCoreApplication::applicationDirPath()+"/errorLog.txt");
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
		}
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
				if(imageSuffixs.contains("."+info.suffix())) //image dropped
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
	client->sendComicInfo(libraryId,currentComicDB);
	connect(client,SIGNAL(finished()),client,SLOT(deleteLater()));
	//delete client;
}