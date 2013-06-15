#include "library_window.h"
#include "custom_widgets.h"
#include "treeitem.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QDir>
#include <QDirModel>
#include <QHeaderView>
#include <QProcess>
#include <QtCore>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFileIconProvider>
#include <QMatrix>
#include <QSettings>
#include <QGLFormat>

#include <iterator>
#include <typeinfo>

#include "data_base_management.h"
#include "yacreader_global.h"
#include "onstart_flow_selection_dialog.h"
#include "no_libraries_widget.h"
#include "import_widget.h"

#include "yacreader_search_line_edit.h"
#include "comic_db.h"
#include "library_creator.h"
#include "package_manager.h"
#include "comic_flow_widget.h"
#include "create_library_dialog.h"
#include "rename_library_dialog.h"
#include "properties_dialog.h"
#include "export_library_dialog.h"
#include "import_library_dialog.h"
#include "export_comics_info_dialog.h"
#include "import_comics_info_dialog.h"
#include "add_library_dialog.h"
#include "options_dialog.h"
#include "help_about_dialog.h"
#include "server_config_dialog.h"
#include "tablemodel.h"
#include "yacreader_tool_bar_stretch.h"
#include "yacreader_table_view.h"

#include "yacreader_dark_menu.h"
#include "yacreader_titled_toolbar.h"
#include "yacreader_main_toolbar.h"

#include "comics_remover.h"

//#include "yacreader_social_dialog.h"
//

LibraryWindow::LibraryWindow()
	:QMainWindow(),fullscreen(false),fetching(false)
{
	setupUI();
	loadLibraries();

	if(libraries.size()==0)
	{
		showNoLibrariesWidget();
	}
	else
	{
		showRootWidget();
	}
}

void LibraryWindow::setupUI()
{
	setWindowIcon(QIcon(":/images/iconLibrary.png"));

	setUnifiedTitleAndToolBarOnMac(true);

	libraryCreator = new LibraryCreator();
	packageManager = new PackageManager();

	settings = new QSettings(QCoreApplication::applicationDirPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
	settings->beginGroup("libraryConfig");

	createActions();
	doModels();
	
	doLayout();
	createToolBars();
	doDialogs();
	createMenus();
	createConnections();

	setWindowTitle(tr("YACReader Library"));

	setMinimumSize(800,480);

	//restore
	if(settings->contains(MAIN_WINDOW_GEOMETRY))
		restoreGeometry(settings->value(MAIN_WINDOW_GEOMETRY).toByteArray());
	else
		//if(settings->value(USE_OPEN_GL).toBool() == false)
			showMaximized();
	if(settings->contains(COMICS_VIEW_HEADERS))
		comicView->horizontalHeader()->restoreState(settings->value(COMICS_VIEW_HEADERS).toByteArray());
	/*if(settings->contains(COMICS_VIEW_HEADERS_GEOMETRY))
		comicView->horizontalHeader()->restoreGeometry(settings->value(COMICS_VIEW_HEADERS_GEOMETRY).toByteArray());*/

	/*socialDialog = new YACReaderSocialDialog(this);
	socialDialog->setHidden(true);*/
}

//#define NEW_LAYOUT

void LibraryWindow::doLayout()
{
	//LAYOUT ELEMENTS------------------------------------------------------------
	//---------------------------------------------------------------------------
	sVertical = new QSplitter(Qt::Vertical);  //spliter derecha
	QSplitter * sHorizontal = new QSplitter(Qt::Horizontal);  //spliter principal
	sHorizontal->setStyleSheet("QSplitter::handle:vertical {height:4px;}");

	//TOOLBARS-------------------------------------------------------------------
	//---------------------------------------------------------------------------
	editInfoToolBar = new QToolBar();
#ifdef NEW_LAYOUT
	libraryToolBar = new QToolBar();
#else
	libraryToolBar = addToolBar(tr("Library"));
#endif

	//FLOW-----------------------------------------------------------------------
	//---------------------------------------------------------------------------
	if(QGLFormat::hasOpenGL() && !settings->contains(USE_OPEN_GL))
	{
		OnStartFlowSelectionDialog * flowSelDialog = new OnStartFlowSelectionDialog();

		flowSelDialog->exec();
		if(flowSelDialog->result() == QDialog::Accepted)
			settings->setValue(USE_OPEN_GL,2);
		else
			settings->setValue(USE_OPEN_GL,0);

		delete flowSelDialog;
	}

	if(QGLFormat::hasOpenGL() && (settings->value(USE_OPEN_GL).toBool() == true))
		comicFlow = new ComicFlowWidgetGL(0);
	else
		comicFlow = new ComicFlowWidgetSW(0);

	comicFlow->updateConfig(settings);
	comicFlow->setFocusPolicy(Qt::StrongFocus);
	comicFlow->setShowMarks(true);
	setFocusProxy(comicFlow);

	fullScreenToolTip = new QLabel(comicFlow);
	fullScreenToolTip->setText(tr("<font color='white'> press 'F' to close fullscreen mode </font>"));
	fullScreenToolTip->setPalette(QPalette(QColor(0,0,0)));
	fullScreenToolTip->setFont(QFont("courier new",15,234));
	fullScreenToolTip->setAutoFillBackground(true);
	fullScreenToolTip->hide();
	fullScreenToolTip->adjustSize();

	comicFlow->setFocus(Qt::OtherFocusReason);

	comicFlow->addAction(toggleFullScreenAction);
	comicFlow->addAction(openComicAction);

	//SIDEBAR-----------------------------------------------------------------------
	//---------------------------------------------------------------------------
	foldersView = new QTreeView;
	foldersView->setContextMenuPolicy(Qt::ActionsContextMenu);
	foldersView->setContextMenuPolicy(Qt::ActionsContextMenu);
	foldersView->header()->hide();
	foldersView->setUniformRowHeights(true);
	foldersView->setSelectionBehavior(QAbstractItemView::SelectRows);
	foldersView->setAttribute(Qt::WA_MacShowFocusRect,false);
#ifdef Q_OS_MAC
    foldersView->setStyleSheet("QTreeView::branch:open:selected:has-children {image: url(':/images/expanded_branch_osx.png');}"
                               "QTreeView::branch:closed:selected:has-children {image: url(':/images/collapsed_branch_osx.png');}");
#endif
	sideBar = new QWidget;
	QVBoxLayout * l = new QVBoxLayout;
	selectedLibrary = new QComboBox;
	selectedLibrary->setAttribute(Qt::WA_MacShowFocusRect,false);
    selectedLibrary->setFocusPolicy(Qt::NoFocus);
	l->setContentsMargins(sHorizontal->handleWidth(),0,0,0);

	YACReaderTitledToolBar * librariesTitle = new YACReaderTitledToolBar(tr("Libraries"));

	l->addWidget(librariesTitle);
	l->addWidget(selectedLibrary);
	
	YACReaderTitledToolBar * foldersTitle = new YACReaderTitledToolBar(tr("Folders"));

	foldersTitle->addAction(setRootIndexAction);
	foldersTitle->addAction(expandAllNodesAction);
	foldersTitle->addAction(colapseAllNodesAction);

	l->addWidget(foldersTitle);
	l->addWidget(foldersView);

	QVBoxLayout * searchLayout = new QVBoxLayout;

	QHBoxLayout * filter = new QHBoxLayout;
	filter->addWidget(foldersFilter = new YACReaderSearchLineEdit());
    foldersFilter->setAttribute(Qt::WA_MacShowFocusRect,false);
	foldersFilter->setPlaceholderText(tr("Search folders and comics"));
	previousFilter = "";

	searchLayout->addLayout(filter);

	l->addLayout(searchLayout);
	l->setSpacing(1);
	sideBar->setLayout(l);

	//FINAL LAYOUT-------------------------------------------------------------
	sVertical->addWidget(comicFlow);
	comics = new QWidget;
	QVBoxLayout * comicsLayout = new QVBoxLayout;
	comicsLayout->setSpacing(0);
	comicsLayout->setContentsMargins(0,0,0,0);
	comicsLayout->addWidget(editInfoToolBar);

	editInfoToolBar->setStyleSheet("QToolBar {border: none;}");
	
	comicView = new YACReaderTableView;
	comicView->verticalHeader()->hide();
	comicsLayout->addWidget(comicView);
	comics->setLayout(comicsLayout);
	sVertical->addWidget(comics);
	sHorizontal->addWidget(sideBar);
#ifdef NEW_LAYOUT
	QVBoxLayout * rightLayout = new QVBoxLayout;
	rightLayout->addWidget(new YACReaderMainToolBar);
	rightLayout->addWidget(sVertical);

	rightLayout->setMargin(0);
	rightLayout->setSpacing(0);

	QWidget * rightWidget = new QWidget();
	rightWidget->setLayout(rightLayout);

	sHorizontal->addWidget(rightWidget);
#else
	sHorizontal->addWidget(sVertical);
#endif
	
	sHorizontal->setStretchFactor(0,0);
	sHorizontal->setStretchFactor(1,1);
	mainWidget = new QStackedWidget(this);
	mainWidget->addWidget(sHorizontal);
	setCentralWidget(mainWidget);
	//FINAL LAYOUT-------------------------------------------------------------


	//OTHER----------------------------------------------------------------------
	//---------------------------------------------------------------------------
	noLibrariesWidget = new NoLibrariesWidget();
	mainWidget->addWidget(noLibrariesWidget);

	importWidget = new ImportWidget();
	mainWidget->addWidget(importWidget);

	connect(noLibrariesWidget,SIGNAL(createNewLibrary()),this,SLOT(createLibrary()));
	connect(noLibrariesWidget,SIGNAL(addExistingLibrary()),this,SLOT(showAddLibrary()));

	comicFlow->addAction(toggleFullScreenAction);
	comicFlow->addAction(openComicAction);

	comicFlow->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void LibraryWindow::doDialogs()
{
	createLibraryDialog = new CreateLibraryDialog(this);
	updateLibraryDialog = new UpdateLibraryDialog(this);
	renameLibraryDialog = new RenameLibraryDialog(this);
	propertiesDialog = new PropertiesDialog(this);
	exportLibraryDialog = new ExportLibraryDialog(this);
	importLibraryDialog = new ImportLibraryDialog(this);
	exportComicsInfoDialog = new ExportComicsInfoDialog(this);
	importComicsInfoDialog = new ImportComicsInfoDialog(this);
	addLibraryDialog = new AddLibraryDialog(this);
	optionsDialog = new OptionsDialog(this);
	optionsDialog->restoreOptions(settings);

#ifdef SERVER_RELEASE
	serverConfigDialog = new ServerConfigDialog(this);
#endif

	had = new HelpAboutDialog(this); //TODO load data.
	QString sufix = QLocale::system().name();
	if(QFile(":/files/about_"+sufix+".html").exists())
		had->loadAboutInformation(":/files/about_"+sufix+".html");
	else
		had->loadAboutInformation(":/files/about.html");

	if(QFile(":/files/helpYACReaderLibrary_"+sufix+".html").exists())
		had->loadHelp(":/files/helpYACReaderLibrary_"+sufix+".html");
	else
		had->loadHelp(":/files/helpYACReaderLibrary.html");


}

void LibraryWindow::doModels()
{
	//folders
	dm = new TreeModel();
	//comics
	dmCV =  new TableModel();
	//comics selection
	sm = new QItemSelectionModel(dm);

	setFoldersFilter("");
}

void LibraryWindow::createActions()
{
	createLibraryAction = new QAction(this);
	createLibraryAction->setToolTip(tr("Create a new library"));
	createLibraryAction->setShortcut(Qt::Key_A);
	createLibraryAction->setIcon(QIcon(":/images/new.png"));

	openLibraryAction = new QAction(this);
	openLibraryAction->setToolTip(tr("Open an existing library"));
	openLibraryAction->setShortcut(Qt::Key_O);
	openLibraryAction->setIcon(QIcon(":/images/openLibrary.png"));

	exportComicsInfo = new QAction(tr("Export comics info"),this);
	exportComicsInfo->setToolTip(tr("Export comics info"));
	exportComicsInfo->setIcon(QIcon(":/images/exportComicsInfo.png"));

	importComicsInfo = new QAction(tr("Import comics info"),this);
	importComicsInfo->setToolTip(tr("Import comics info"));
	importComicsInfo->setIcon(QIcon(":/images/importComicsInfo.png"));

	exportLibraryAction = new QAction(tr("Pack covers"),this);
	exportLibraryAction->setToolTip(tr("Pack the covers of the selected library"));
	exportLibraryAction->setIcon(QIcon(":/images/exportLibrary.png"));

	importLibraryAction = new QAction(tr("Unpack covers"),this);
	importLibraryAction->setToolTip(tr("Unpack a catalog"));
	importLibraryAction->setIcon(QIcon(":/images/importLibrary.png"));

	updateLibraryAction = new QAction(this);
	updateLibraryAction->setToolTip(tr("Update current library"));
	updateLibraryAction->setShortcut(Qt::Key_U);
	updateLibraryAction->setIcon(QIcon(":/images/updateLibrary.png"));

	renameLibraryAction = new QAction(this);
	renameLibraryAction->setToolTip(tr("Rename current library"));
	renameLibraryAction->setShortcut(Qt::Key_R);
	renameLibraryAction->setIcon(QIcon(":/images/edit.png"));

	removeLibraryAction = new QAction(this);
	removeLibraryAction->setToolTip(tr("Remove current library from your collection"));
	removeLibraryAction->setIcon(QIcon(":/images/removeLibrary.png"));

	openComicAction = new QAction(tr("Open current comic"),this);
	openComicAction->setToolTip(tr("Open current comic on YACReader"));
	openComicAction->setShortcut(Qt::Key_Return);
	openComicAction->setIcon(QIcon(":/images/openInYACReader.png"));

	setAsReadAction = new QAction(tr("Set as read"),this);
	setAsReadAction->setToolTip(tr("Set comic as read"));
	setAsReadAction->setIcon(QIcon(":/images/setReadButton.png"));

	setAsNonReadAction = new QAction(tr("Set as unread"),this);
	setAsNonReadAction->setToolTip(tr("Set comic as unread"));
	setAsNonReadAction->setIcon(QIcon(":/images/setUnread.png"));

	setAllAsReadAction = new QAction(tr("Set all as read"),this);
	setAllAsReadAction->setToolTip(tr("Set all comics as read"));
	setAllAsReadAction->setIcon(QIcon(":/images/setAllRead.png"));

	setAllAsNonReadAction = new QAction(tr("Set all as unread"),this);
	setAllAsNonReadAction->setToolTip(tr("Set all comics as unread"));
	setAllAsNonReadAction->setIcon(QIcon(":/images/setAllUnread.png"));

	showHideMarksAction = new QAction(tr("Show/Hide marks"),this);
	showHideMarksAction->setToolTip(tr("Show or hide readed marks"));
	showHideMarksAction->setShortcut(Qt::Key_M);
	showHideMarksAction->setCheckable(true);
	showHideMarksAction->setIcon(QIcon(":/images/showMarks.png"));
	showHideMarksAction->setChecked(true);


	showPropertiesAction = new QAction(this);
	showPropertiesAction->setToolTip(tr("Show properties of current comic"));
	showPropertiesAction->setShortcut(Qt::Key_P);
	showPropertiesAction->setIcon(QIcon(":/images/properties.png"));

	toggleFullScreenAction = new QAction(tr("Fullscreen mode on/off"),this);
	toggleFullScreenAction->setToolTip(tr("Fullscreen mode on/off (F)"));
	toggleFullScreenAction->setShortcut(Qt::Key_F);
	toggleFullScreenAction->setIcon(QIcon(":/images/fit.png"));

	helpAboutAction = new QAction(this);
	helpAboutAction->setToolTip(tr("Help, About YACReader"));
	helpAboutAction->setShortcut(Qt::Key_F1);
	helpAboutAction->setIcon(QIcon(":/images/help.png"));

	setRootIndexAction = new QAction(this);
	setRootIndexAction->setShortcut(Qt::Key_0);
	setRootIndexAction->setToolTip(tr("Select root node"));
	setRootIndexAction->setIcon(QIcon(":/images/setRoot.png"));

	expandAllNodesAction = new QAction(this);
	expandAllNodesAction->setShortcut(tr("+"));
	expandAllNodesAction->setToolTip(tr("Expand all nodes"));
	expandAllNodesAction->setIcon(QIcon(":/images/expand.png"));

	colapseAllNodesAction = new QAction(this);
	colapseAllNodesAction->setShortcut(tr("-"));
	colapseAllNodesAction->setToolTip(tr("Colapse all nodes"));
	colapseAllNodesAction->setIcon(QIcon(":/images/colapse.png"));

	optionsAction = new QAction(this);
	optionsAction->setShortcut(Qt::Key_C);
	optionsAction->setToolTip(tr("Show options dialog"));
	optionsAction->setIcon(QIcon(":/images/options.png"));

	serverConfigAction = new QAction(this);
	serverConfigAction->setShortcut(Qt::Key_S);
	serverConfigAction->setToolTip(tr("Show comics server options dialog"));
	serverConfigAction->setIcon(QIcon(":/images/server.png"));

	//socialAction = new QAction(this);

	//disable actions
	disableAllActions();

	openContainingFolderAction = new QAction(this);
	openContainingFolderAction->setText(tr("Open folder..."));
	openContainingFolderAction->setIcon(QIcon(":/images/open.png"));

	openContainingFolderComicAction = new QAction(this);
	openContainingFolderComicAction->setText(tr("Open containing folder..."));
	openContainingFolderComicAction->setIcon(QIcon(":/images/open.png"));

	//Edit comics actions------------------------------------------------------
	selectAllComicsAction = new QAction(this);
	selectAllComicsAction->setText(tr("Select all comics"));
	selectAllComicsAction->setIcon(QIcon(":/images/selectAll.png"));

	editSelectedComicsAction = new QAction(this);
	editSelectedComicsAction->setText(tr("Edit"));
	editSelectedComicsAction->setIcon(QIcon(":/images/editComic.png"));

	asignOrderActions = new QAction(this);
	asignOrderActions->setText(tr("Asign current order to comics"));
	asignOrderActions->setIcon(QIcon(":/images/asignNumber.png"));

	forceConverExtractedAction = new QAction(this);
	forceConverExtractedAction->setText(tr("Update cover"));
	forceConverExtractedAction->setIcon(QIcon(":/images/importCover.png"));

	deleteComicsAction = new QAction(this);
	deleteComicsAction->setText(tr("Delete selected comics"));
	deleteComicsAction->setIcon(QIcon(":/images/trash.png"));

	hideComicViewAction = new QAction(this);
	hideComicViewAction->setText(tr("Hide comic flow"));
	hideComicViewAction->setIcon(QIcon(":/images/hideComicFlow.png"));
	hideComicViewAction->setCheckable(true);
	hideComicViewAction->setChecked(false);
	//-------------------------------------------------------------------------
}

//TODO unificar con disableActions
void LibraryWindow::disableAllActions()
{
	updateLibraryAction->setEnabled(false);
	renameLibraryAction->setEnabled(false);
	//deleteLibraryAction->setEnabled(false);
	removeLibraryAction->setEnabled(false);
	openComicAction->setEnabled(false);
	showPropertiesAction->setEnabled(false);
	setAsReadAction->setEnabled(false);
	setAsNonReadAction->setEnabled(false);
	setAllAsReadAction->setEnabled(false);
	setAllAsNonReadAction->setEnabled(false);

	showHideMarksAction->setEnabled(false);
	importComicsInfo->setEnabled(false);
	exportComicsInfo->setEnabled(false);
	exportLibraryAction->setEnabled(false);
	toggleFullScreenAction->setEnabled(false);
}

//librería de sólo lectura
void LibraryWindow::disableActions()
{
	updateLibraryAction->setEnabled(false);
	openComicAction->setEnabled(false);
	showPropertiesAction->setEnabled(false);
	openContainingFolderAction->setEnabled(false);
	openContainingFolderComicAction->setEnabled(false);
	setAsReadAction->setEnabled(false);
	setAsNonReadAction->setEnabled(false);
	setAllAsReadAction->setEnabled(false);
	setAllAsNonReadAction->setEnabled(false);
	selectAllComicsAction->setEnabled(false);
	editSelectedComicsAction->setEnabled(false);
	asignOrderActions->setEnabled(false);
}
//librería abierta
void LibraryWindow::enableActions()
{
	updateLibraryAction->setEnabled(true);
	openComicAction->setEnabled(true);
	showPropertiesAction->setEnabled(true);
	openContainingFolderAction->setEnabled(true);
	openContainingFolderComicAction->setEnabled(true);
	setAsReadAction->setEnabled(true);
	setAsNonReadAction->setEnabled(true);
	setAllAsReadAction->setEnabled(true);
	setAllAsNonReadAction->setEnabled(true);

	showHideMarksAction->setEnabled(true);
	importComicsInfo->setEnabled(true);
	exportComicsInfo->setEnabled(true);
	exportLibraryAction->setEnabled(true);
	toggleFullScreenAction->setEnabled(true);
}
void LibraryWindow::enableLibraryActions()
{
	renameLibraryAction->setEnabled(true);
	//deleteLibraryAction->setEnabled(true);
	removeLibraryAction->setEnabled(true);
	foldersFilter->setEnabled(true);
	//clearFoldersFilter->setEnabled(true);
}

void LibraryWindow::createToolBars()
{
	libraryToolBar->setIconSize(QSize(32,32)); //TODO make icon size dynamic

	libraryToolBar->addAction(createLibraryAction);
	libraryToolBar->addAction(openLibraryAction);

	libraryToolBar->addSeparator();

	libraryToolBar->addAction(exportComicsInfo);
	libraryToolBar->addAction(importComicsInfo);

	libraryToolBar->addSeparator();

	libraryToolBar->addAction(exportLibraryAction);
	libraryToolBar->addAction(importLibraryAction);
	

	libraryToolBar->addSeparator();
	libraryToolBar->addAction(updateLibraryAction); 
	libraryToolBar->addAction(renameLibraryAction);
	libraryToolBar->addAction(removeLibraryAction);
	//libraryToolBar->addAction(deleteLibraryAction);

	libraryToolBar->addSeparator();
	libraryToolBar->addAction(toggleFullScreenAction);
#ifdef Q_OS_MAC
   libraryToolBar->addSeparator();
#else
	libraryToolBar->addWidget(new QToolBarStretch());
#endif

#ifdef SERVER_RELEASE
	libraryToolBar->addAction(serverConfigAction);
#endif
	libraryToolBar->addAction(optionsAction);
	libraryToolBar->addAction(helpAboutAction);
	//libraryToolBar->addAction(socialAction);

#ifdef Q_OS_MAC
    libraryToolBar->addWidget(new QToolBarStretch());
    libraryToolBar->addWidget(foldersFilter);
#endif

	libraryToolBar->setMovable(false);

	editInfoToolBar->setIconSize(QSize(18,18));
	editInfoToolBar->addAction(openComicAction);
	editInfoToolBar->addSeparator();
	editInfoToolBar->addAction(editSelectedComicsAction);
	editInfoToolBar->addAction(selectAllComicsAction);
	editInfoToolBar->addSeparator();
	editInfoToolBar->addAction(asignOrderActions);

	editInfoToolBar->addSeparator();

	editInfoToolBar->addAction(setAsReadAction);
	editInfoToolBar->addAction(setAllAsReadAction);
	editInfoToolBar->addAction(setAsNonReadAction);
	editInfoToolBar->addAction(setAllAsNonReadAction);

	editInfoToolBar->addAction(showHideMarksAction);

	editInfoToolBar->addSeparator();
	
	editInfoToolBar->addAction(deleteComicsAction);

	editInfoToolBar->addWidget(new QToolBarStretch());
	editInfoToolBar->addAction(hideComicViewAction);

}

void LibraryWindow::createMenus()
{
	comicView->addAction(openContainingFolderComicAction);
	foldersView->addAction(openContainingFolderAction);
}

void LibraryWindow::createConnections()
{
	//libraryCreator connections
	connect(createLibraryDialog,SIGNAL(createLibrary(QString,QString,QString)),this,SLOT(create(QString,QString,QString)));
	connect(importComicsInfoDialog,SIGNAL(finished(int)),this,SLOT(reloadCurrentLibrary()));

	connect(libraryCreator,SIGNAL(coverExtracted(QString)),createLibraryDialog,SLOT(showCurrentFile(QString)));
	connect(libraryCreator,SIGNAL(coverExtracted(QString)),updateLibraryDialog,SLOT(showCurrentFile(QString)));
	connect(libraryCreator,SIGNAL(finished()),this,SLOT(showRootWidget()));
	connect(libraryCreator,SIGNAL(updated()),this,SLOT(reloadCurrentLibrary()));
	connect(libraryCreator,SIGNAL(created()),this,SLOT(openLastCreated()));
	connect(libraryCreator,SIGNAL(comicAdded(QString,QString)),importWidget,SLOT(newComic(QString,QString)));
	//libraryCreator errors
	connect(libraryCreator,SIGNAL(failedCreatingDB(QString)),this,SLOT(manageCreatingError(QString)));
	connect(libraryCreator,SIGNAL(failedUpdatingDB(QString)),this,SLOT(manageUpdatingError(QString)));
	
	//new import widget
	connect(importWidget,SIGNAL(stop()),this,SLOT(stopLibraryCreator()));

	//packageManager connections
	connect(exportLibraryDialog,SIGNAL(exportPath(QString)),this,SLOT(exportLibrary(QString)));
	connect(exportLibraryDialog,SIGNAL(rejected()),packageManager,SLOT(cancel()));
	connect(packageManager,SIGNAL(exported()),exportLibraryDialog,SLOT(close()));
	connect(importLibraryDialog,SIGNAL(unpackCLC(QString,QString,QString)),this,SLOT(importLibrary(QString,QString,QString)));
	connect(importLibraryDialog,SIGNAL(rejected()),packageManager,SLOT(cancel()));
	connect(importLibraryDialog,SIGNAL(rejected()),this,SLOT(deleteCurrentLibrary()));
	connect(packageManager,SIGNAL(imported()),importLibraryDialog,SLOT(hide()));
	connect(packageManager,SIGNAL(imported()),this,SLOT(openLastCreated()));


	//create and update dialogs
	connect(createLibraryDialog,SIGNAL(cancelCreate()),this,SLOT(cancelCreating()));
	connect(updateLibraryDialog,SIGNAL(cancelUpdate()),this,SLOT(stopLibraryCreator()));

	//open existing library from dialog.
	connect(addLibraryDialog,SIGNAL(addLibrary(QString,QString)),this,SLOT(openLibrary(QString,QString)));

	//load library when selected library changes
	connect(selectedLibrary,SIGNAL(currentIndexChanged(QString)),this,SLOT(loadLibrary(QString)));

	//rename library dialog
	connect(renameLibraryDialog,SIGNAL(renameLibrary(QString)),this,SLOT(rename(QString)));

	//navigations between view modes (tree,list and flow)
	connect(foldersView, SIGNAL(pressed(QModelIndex)), this, SLOT(loadCovers(QModelIndex)));
	connect(comicView, SIGNAL(pressed(QModelIndex)), this, SLOT(centerComicFlow(QModelIndex)));
	connect(comicFlow, SIGNAL(centerIndexChanged(int)), this, SLOT(updateComicView(int)));

	//actions
	connect(createLibraryAction,SIGNAL(triggered()),this,SLOT(createLibrary()));
	connect(exportLibraryAction,SIGNAL(triggered()),exportLibraryDialog,SLOT(show()));
	connect(importLibraryAction,SIGNAL(triggered()),importLibraryDialog,SLOT(show()));

	connect(openLibraryAction,SIGNAL(triggered()),this,SLOT(showAddLibrary()));
	connect(showPropertiesAction,SIGNAL(triggered()),this,SLOT(showProperties()));
	connect(setAsReadAction,SIGNAL(triggered()),this,SLOT(setCurrentComicReaded()));
	connect(setAsNonReadAction,SIGNAL(triggered()),this,SLOT(setCurrentComicUnreaded()));
	connect(setAllAsReadAction,SIGNAL(triggered()),this,SLOT(setComicsReaded()));
	connect(setAllAsNonReadAction,SIGNAL(triggered()),this,SLOT(setComicsUnreaded()));

	connect(showHideMarksAction,SIGNAL(toggled(bool)),comicFlow,SLOT(setShowMarks(bool)));
	
	//comicsInfoManagement
	connect(exportComicsInfo,SIGNAL(triggered()),this,SLOT(showExportComicsInfo()));
	connect(importComicsInfo,SIGNAL(triggered()),this,SLOT(showImportComicsInfo()));

	//properties & config
	connect(propertiesDialog,SIGNAL(accepted()),this,SLOT(reloadCovers()));

	connect(updateLibraryAction,SIGNAL(triggered()),this,SLOT(updateLibrary()));
	connect(renameLibraryAction,SIGNAL(triggered()),this,SLOT(renameLibrary()));
	//connect(deleteLibraryAction,SIGNAL(triggered()),this,SLOT(deleteLibrary()));
	connect(removeLibraryAction,SIGNAL(triggered()),this,SLOT(removeLibrary()));
	connect(openComicAction,SIGNAL(triggered()),this,SLOT(openComic()));
	connect(helpAboutAction,SIGNAL(triggered()),had,SLOT(show()));
	connect(setRootIndexAction,SIGNAL(triggered()),this,SLOT(setRootIndex()));
	connect(expandAllNodesAction,SIGNAL(triggered()),foldersView,SLOT(expandAll()));
	connect(colapseAllNodesAction,SIGNAL(triggered()),foldersView,SLOT(collapseAll()));
	connect(toggleFullScreenAction,SIGNAL(triggered()),this,SLOT(toggleFullScreen()));
	connect(optionsAction, SIGNAL(triggered()),optionsDialog,SLOT(show()));
#ifdef SERVER_RELEASE
	connect(serverConfigAction, SIGNAL(triggered()), serverConfigDialog, SLOT(show()));
#endif
	connect(optionsDialog, SIGNAL(optionsChanged()),this,SLOT(reloadOptions()));
	//ComicFlow
	connect(comicFlow,SIGNAL(selected(unsigned int)),this,SLOT(openComic()));
	connect(comicView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(openComic()));
	//Folders filter
	//connect(clearFoldersFilter,SIGNAL(clicked()),foldersFilter,SLOT(clear()));
	connect(foldersFilter,SIGNAL(textChanged(QString)),this,SLOT(setFoldersFilter(QString)));
	//connect(includeComicsCheckBox,SIGNAL(stateChanged(int)),this,SLOT(searchInFiles(int)));

	//ContextMenus
	connect(openContainingFolderComicAction,SIGNAL(triggered()),this,SLOT(openContainingFolderComic()));
	connect(openContainingFolderAction,SIGNAL(triggered()),this,SLOT(openContainingFolder()));

	//connect(dm,SIGNAL(directoryLoaded(QString)),foldersView,SLOT(expandAll()));
	//connect(dm,SIGNAL(directoryLoaded(QString)),this,SLOT(updateFoldersView(QString)));
	//Comicts edition
	connect(selectAllComicsAction,SIGNAL(triggered()),comicView,SLOT(selectAll()));
	connect(editSelectedComicsAction,SIGNAL(triggered()),this,SLOT(showProperties()));
	connect(asignOrderActions,SIGNAL(triggered()),this,SLOT(asignNumbers()));

	connect(deleteComicsAction,SIGNAL(triggered()),this,SLOT(deleteComics()));

	connect(hideComicViewAction, SIGNAL(toggled(bool)),this, SLOT(hideComicFlow(bool)));

	//connect(socialAction,SIGNAL(triggered()),this,SLOT(showSocial()));

}

void LibraryWindow::loadLibrary(const QString & name)
{
	if(libraries.size()>0)  //si hay bibliotecas...
	{	
		showRootWidget();
		QString path=libraries.value(name)+"/.yacreaderlibrary";
		QDir d; //TODO change this by static methods (utils class?? with delTree for example)
		QString dbVersion;
		if(d.exists(path) && (dbVersion = DataBaseManagement::checkValidDB(path+"/library.ydb")) != "") //si existe en disco la biblioteca seleccionada, y es válida..
		{
			int comparation = DataBaseManagement::compareVersions(dbVersion,VERSION);
			bool updated = false;
			if(comparation < 0)
				{
					int ret = QMessageBox::question(this,tr("Update needed"),tr("This library was created with a previous version of YACReaderLibrary. It needs to be updated. Update now?"),QMessageBox::Yes,QMessageBox::No);
					if(ret == QMessageBox::Yes)
					{
						//TODO update to new version
						updated = DataBaseManagement::updateToCurrentVersion(path+"/library.ydb");
						if(!updated)
							QMessageBox::critical(this,tr("Update failed"), tr("The current library can't be udpated. Check for write write permissions on: ") + path+"/library.ydb");
					}
					else
					{
						comicView->setModel(NULL);
						foldersView->setModel(NULL);
						comicFlow->clear();
						disableAllActions();//TODO comprobar que se deben deshabilitar
					}
				}

			if(comparation == 0 || updated) //en caso de que la versión se igual que la actual
			{
				index = 0;
				sm->clear();

				dm->setupModelData(path);
				foldersView->setModel(dm);

				d.setCurrent(libraries.value(name));
				d.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
				if(d.count()<=1) //librería de sólo lectura
				{
					//QMessageBox::critical(NULL,QString::number(d.count()),QString::number(d.count()));
					disableActions();
					importedCovers = true;
				}
				else //librería normal abierta
				{
					enableActions();
					importedCovers = false;
				}
				enableLibraryActions();

				setRootIndex();
				//TODO encontrar el bug que provoca que no se carguen adecuadamente las carátulas en root.
				setRootIndex();

				foldersFilter->clear();
			}
			else if(comparation > 0)
			{
					int ret = QMessageBox::question(this,tr("Download new version"),tr("This library was created with a newer version of YACReaderLibrary. Download the new version now?"),QMessageBox::Yes,QMessageBox::No);
					if(ret == QMessageBox::Yes)
						QDesktopServices::openUrl(QUrl("http://www.yacreader.com"));

					comicView->setModel(NULL);
					foldersView->setModel(NULL);
					comicFlow->clear();
					disableAllActions();//TODO comprobar que se deben deshabilitar
			}
		}
		else
		{
			comicView->setModel(NULL);
			foldersView->setModel(NULL);
			comicFlow->clear();
			disableAllActions();//TODO comprobar que se deben deshabilitar

			//si la librería no existe en disco, se ofrece al usuario la posibiliad de eliminarla
			if(!d.exists(path))
			{
				QString currentLibrary = selectedLibrary->currentText();
				if(QMessageBox::question(this,tr("Library not available"),tr("Library ")+currentLibrary+tr(" is no longer available. Do you want to remove it?"),QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes)
				{
					deleteCurrentLibrary();
				}
			}
			else//si existe el path, puede ser que la librería sea alguna versión pre-5.0 ó que esté corrupta o que no haya drivers sql
			{
				QSqlDatabase db = DataBaseManagement::loadDatabase(path);
				if(d.exists(path+"/library.ydb"))
				{
					manageOpeningLibraryError(db.lastError().databaseText() + "-" + db.lastError().driverText());
				}
				else
				{
					QString currentLibrary = selectedLibrary->currentText();
					QString path = libraries.value(selectedLibrary->currentText());
					if(QMessageBox::question(this,tr("Old library"),tr("Library ")+currentLibrary+tr("has been created with an older version of YACReaderLibrary. It must be created again. Do you want to create the library now?"),QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes)
					{
						QDir d(path+"/.yacreaderlibrary");
						delTree(d);
						d.rmdir(path+"/.yacreaderlibrary");
						createLibraryDialog->setDataAndStart(currentLibrary,path);
						//create(path,path+"/.yacreaderlibrary",currentLibrary);
					}
				}
			}
		}
	}
	else //en caso de que no exista ninguna biblioteca se desactivan los botones pertinentes
	{
		disableAllActions();
		showNoLibrariesWidget();
	}
}

void LibraryWindow::loadCovers(const QModelIndex & mi)
{
	_rootIndexCV = mi;
	unsigned long long int folderId = 1;
	if(mi.isValid())
	{
		TreeItem *item = static_cast<TreeItem*>(mi.internalPointer());
		folderId = item->id;
	}

	//cambiado de orden, ya que al llamar a foldersFilter->clear() se invalidan los model index
	if(foldersFilter->text()!="")
	{
		//setFoldersFilter("");
		if(mi.isValid())
		{
			index = static_cast<TreeItem *>(mi.internalPointer())->originalItem;
			column = mi.column();
			foldersFilter->clear();
		}
	}
	else
	{
		index = static_cast<TreeItem *>(mi.internalPointer());
		column = mi.column();
	}

	//comicView->setModel(NULL);
	dmCV->setupModelData(folderId,dm->getDatabase());
	comicView->setModel(dmCV);
	comicView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	comicView->horizontalHeader()->setMovable(true);
	//TODO parametrizar la configuración de las columnas
	for(int i = 0;i<comicView->horizontalHeader()->count();i++)
		comicView->horizontalHeader()->hideSection(i);

	comicView->horizontalHeader()->showSection(0);
	comicView->horizontalHeader()->showSection(1);
	comicView->horizontalHeader()->showSection(2);
	comicView->horizontalHeader()->showSection(3);
	comicView->horizontalHeader()->showSection(7);
	comicView->horizontalHeader()->showSection(8);


	//debido a un bug, qt4 no es capaz de ajustar el ancho teniendo en cuenta todas la filas (no sólo las visibles)
	//así que se ecala la primera vez y después se deja el control al usuario.
	if(!settings->contains(COMICS_VIEW_HEADERS))
		comicView->resizeColumnsToContents();
	comicView->horizontalHeader()->setStretchLastSection(true);

	QStringList paths = dmCV->getPaths(currentPath());
	comicFlow->setImagePaths(paths);
	comicFlow->setMarks(dmCV->getReadList());
	comicFlow->setFocus(Qt::OtherFocusReason);

	checkEmptyFolder(&paths);

	if(paths.size()>0)
		comicView->setCurrentIndex(dmCV->index(0,0));
}

void LibraryWindow::checkEmptyFolder(QStringList * paths)
{
	if(paths == 0)
		paths = &dmCV->getPaths(currentPath());

	if(paths->size()>0 && !importedCovers)
	{
		openComicAction->setEnabled(true);
		showPropertiesAction->setEnabled(true);
		setAsReadAction->setEnabled(true);
		setAsNonReadAction->setEnabled(true);
		setAllAsReadAction->setEnabled(true);
		setAllAsNonReadAction->setEnabled(true);
		selectAllComicsAction->setEnabled(true);
		editSelectedComicsAction->setEnabled(true);
		asignOrderActions->setEnabled(true);

		showHideMarksAction->setEnabled(true);
		toggleFullScreenAction->setEnabled(true);

		deleteComicsAction->setEnabled(true);
	}
	else
	{
		openComicAction->setEnabled(false);
		showPropertiesAction->setEnabled(false);
		setAsReadAction->setEnabled(false);
		setAsNonReadAction->setEnabled(false);
		setAllAsReadAction->setEnabled(false);
		setAllAsNonReadAction->setEnabled(false);
		selectAllComicsAction->setEnabled(false);
		editSelectedComicsAction->setEnabled(false);
		asignOrderActions->setEnabled(false);

		showHideMarksAction->setEnabled(false);
		toggleFullScreenAction->setEnabled(false);

		deleteComicsAction->setEnabled(false);
	}
}

void LibraryWindow::reloadCovers()
{
	loadCovers(_rootIndexCV);

	QModelIndex mi = dmCV->getIndexFromId(_comicIdEdited);
	comicView->scrollTo(mi,QAbstractItemView::PositionAtCenter);
	comicView->setCurrentIndex(mi);
	//centerComicFlow(mi);
	comicFlow->setCenterIndex(mi.row());
}

void LibraryWindow::centerComicFlow(const QModelIndex & mi)
{
	comicFlow->showSlide(mi.row());
	comicFlow->setFocus(Qt::OtherFocusReason);
}

void LibraryWindow::updateComicView(int i)
{
		QModelIndex mi = dmCV->index(i,2);
		comicView->setCurrentIndex(mi);
		comicView->scrollTo(mi,QAbstractItemView::EnsureVisible);
}

void LibraryWindow::openComic()
{
	if(!importedCovers)
	{
		QString path = currentPath() + dmCV->getComicPath(comicView->currentIndex());
		
#ifdef Q_OS_MAC
		
		QProcess::startDetached("open", QStringList() << "-n" << QDir::cleanPath(QCoreApplication::applicationDirPath()+"/../../../YACReader.app") << "--args" << path);//,QStringList() << path);
		//Comic is readed
#else
        
		QProcess::startDetached(QDir::cleanPath(QCoreApplication::applicationDirPath())+"/YACReader",QStringList() << path);
#endif        
		//Comic is readed
		setCurrentComicReaded();
	}
}

void LibraryWindow::setCurrentComicsStatusReaded(bool readed)
{

	comicFlow->setMarks(dmCV->setComicsRead(getSelectedComics(),readed));
	comicFlow->updateMarks();
}

void LibraryWindow::setCurrentComicReaded()
{
	this->setCurrentComicsStatusReaded(true);
}

void LibraryWindow::setComicsReaded()
{
	comicFlow->setMarks(dmCV->setAllComicsRead(true));
	comicFlow->updateMarks();
}

void LibraryWindow::setCurrentComicUnreaded()
{
	this->setCurrentComicsStatusReaded(false);
}

void LibraryWindow::setComicsUnreaded()
{
	comicFlow->setMarks(dmCV->setAllComicsRead(false));
	comicFlow->updateMarks();
}

void LibraryWindow::createLibrary()
{
	createLibraryDialog->show();
}

void LibraryWindow::create(QString source, QString dest, QString name)
{
	libraryCreator->createLibrary(source,dest);
	libraryCreator->start();
	_lastAdded = name;
	_sourceLastAdded = source;

	importWidget->setImportLook();
	showImportingWidget();

}

void LibraryWindow::reloadCurrentLibrary()
{
	loadLibrary(selectedLibrary->currentText());
}

void LibraryWindow::openLastCreated()
{
	
	selectedLibrary->disconnect();

	selectedLibrary->setCurrentIndex(selectedLibrary->findText(_lastAdded));
	libraries.insert(_lastAdded,_sourceLastAdded);
	selectedLibrary->addItem(_lastAdded,_sourceLastAdded);
	selectedLibrary->setCurrentIndex(selectedLibrary->findText(_lastAdded));
	saveLibraries();

	connect(selectedLibrary,SIGNAL(currentIndexChanged(QString)),this,SLOT(loadLibrary(QString)));
	
	loadLibrary(_lastAdded);
}

void LibraryWindow::showAddLibrary()
{
	addLibraryDialog->show();
}

void LibraryWindow::openLibrary(QString path, QString name)
{	
	path.remove("/.yacreaderlibrary");
	QDir d; //TODO change this by static methods (utils class?? with delTree for example) 
	if(d.exists(path + "/.yacreaderlibrary"))
	{
		_lastAdded = name;
		_sourceLastAdded = path;
		openLastCreated();
	}
	else
		QMessageBox::warning(this,tr("Library not found"),tr("The selected folder doesn't contain any library."));
}

void LibraryWindow::loadLibraries()
{
	QFile f(QCoreApplication::applicationDirPath()+"/libraries.yacr");
	f.open(QIODevice::ReadOnly);
	QTextStream txtS(&f);
	QString content = txtS.readAll();
	QStringList lines = content.split('\n');
	QString line,name;
	int i=0;
	bool librariesAvailable = false;
	foreach(line,lines)
	{
		if((i%2)==0)
		{
			name = line;
		}
		else
		{
			librariesAvailable = true;
			libraries.insert(name.trimmed(),line.trimmed());
			selectedLibrary->addItem(name.trimmed(),line.trimmed());
		}
		i++;
	}

	if(!librariesAvailable)
	{
		disableAllActions();
	}
}

void LibraryWindow::saveLibraries()
{
	QFile f(QCoreApplication::applicationDirPath()+"/libraries.yacr");
	if(!f.open(QIODevice::WriteOnly))
	{
		QMessageBox::critical(NULL,tr("Saving libraries file...."),tr("There was a problem saving YACReaderLibrary libraries file. Please, check if you have enough permissions in the YACReader root folder."));
	}
	else
	{
		QTextStream txtS(&f);
		for(QMap<QString,QString>::iterator i = libraries.begin();i!=libraries.end();i++)
		{
			txtS << i.key() << "\n";
			txtS << i.value() << "\n";
		}
	}
}

void LibraryWindow::updateLibrary()
{
	//updateLibraryDialog->show();
	importWidget->setUpdateLook();
	showImportingWidget();

	QString currentLibrary = selectedLibrary->currentText();
	QString path = libraries.value(currentLibrary);
	_lastAdded = currentLibrary;
	libraryCreator->updateLibrary(path,path+"/.yacreaderlibrary");
	libraryCreator->start();
}

void LibraryWindow::deleteCurrentLibrary()
{
	QString path = libraries.value(selectedLibrary->currentText());
	libraries.remove(selectedLibrary->currentText());
	selectedLibrary->removeItem(selectedLibrary->currentIndex());
	selectedLibrary->setCurrentIndex(0);
	path = path+"/.yacreaderlibrary";

	QDir d(path);
	delTree(d);
	d.rmdir(path);
	if(libraries.size()==0)//no more libraries avaliable.
	{
		comicView->setModel(NULL);
		foldersView->setModel(NULL);
		comicFlow->clear();
	}
	saveLibraries();
}

void LibraryWindow::removeLibrary()
{
	QString currentLibrary = selectedLibrary->currentText();
	QMessageBox * messageBox = new QMessageBox(tr("Are you sure?"),tr("Do you want remove ")+currentLibrary+tr(" library?"),QMessageBox::Question,QMessageBox::Yes,QMessageBox::YesToAll,QMessageBox::No);
	messageBox->button(QMessageBox::YesToAll)->setText(tr("Remove and delete metadata"));
	int ret = messageBox->exec();
	if(ret == QMessageBox::Yes)
	{
		libraries.remove(currentLibrary);
		selectedLibrary->removeItem(selectedLibrary->currentIndex());
		selectedLibrary->setCurrentIndex(0);
		if(libraries.size()==0)//no more libraries avaliable.
		{
			comicView->setModel(NULL);
			foldersView->setModel(NULL);
			comicFlow->clear();
		}
		saveLibraries();
	}
	else if(ret == QMessageBox::YesToAll)
	{
		deleteCurrentLibrary();
	}
}

void LibraryWindow::renameLibrary()
{
	renameLibraryDialog->show();
}

void LibraryWindow::rename(QString newName)
{
	QString currentLibrary = selectedLibrary->currentText();
	QString path = libraries.value(currentLibrary);
	libraries.remove(currentLibrary);
	selectedLibrary->removeItem(selectedLibrary->currentIndex());
	libraries.insert(newName,path);
	selectedLibrary->addItem(newName,path);
	saveLibraries();
	selectedLibrary->setCurrentIndex(selectedLibrary->findText(newName));
}

void LibraryWindow::cancelCreating()
{
	stopLibraryCreator();
}

void LibraryWindow::stopLibraryCreator()
{
	libraryCreator->stop();
	libraryCreator->wait();
}

void LibraryWindow::setRootIndex()
{
	if(libraries.size()>0)
	{	
		QString path=libraries.value(selectedLibrary->currentText())+"/.yacreaderlibrary";
		QDir d; //TODO change this by static methods (utils class?? with delTree for example) 
		if(d.exists(path))
		{
			loadCovers(QModelIndex());
		}
		else
		{
			comicView->setModel(NULL);
			comicFlow->clear();
		}

        foldersView->clearSelection();
	}
}


void LibraryWindow::toggleFullScreen()
{
	fullscreen?toNormal():toFullScreen();
	fullscreen = !fullscreen;
}

void LibraryWindow::toFullScreen()
{
	fromMaximized = this->isMaximized();

	comicFlow->hide();
	//comicFlow->setSlideSize(slideSizeF);
	comicFlow->setCenterIndex(comicFlow->centerIndex());
	comics->hide();
	sideBar->hide();
	libraryToolBar->hide();

	showFullScreen();

	comicFlow->show();
	comicFlow->setFocus(Qt::OtherFocusReason);

	fullScreenToolTip->move((width()-fullScreenToolTip->width())/2,0);
	fullScreenToolTip->adjustSize();
	fullScreenToolTip->show();
}

void LibraryWindow::toNormal()
{
	fullScreenToolTip->hide();
	comicFlow->hide();
	//comicFlow->setSlideSize(slideSizeW);
	comicFlow->setCenterIndex(comicFlow->centerIndex());
	comicFlow->render();
	comics->show();
	sideBar->show();
	
	libraryToolBar->show();
	comicFlow->show();

	if(fromMaximized)
		showMaximized();
	else
		showNormal();
}

void LibraryWindow::setFoldersFilter(QString filter)
{
	if(filter.isEmpty() && dm->isFilterEnabled())
	{
		dm->resetFilter();
		//foldersView->collapseAll();
		if(index != 0)
		{
			QModelIndex mi = dm->indexFromItem(index,column);
			foldersView->scrollTo(mi,QAbstractItemView::PositionAtTop);
			sm->select(mi,QItemSelectionModel::Select);
			foldersView->setSelectionModel(sm);
		}
	}
	else
	{
		if(!filter.isEmpty())
		{
			dm->setFilter(filter, true);//includeComicsCheckBox->isChecked());
			foldersView->expandAll();
		}
	}
}

void LibraryWindow::showProperties()
{
	QModelIndexList indexList = getSelectedComics();

	QList<ComicDB> comics = dmCV->getComics(indexList);
	ComicDB c = comics[0];
	_comicIdEdited = c.id;//static_cast<TableItem*>(indexList[0].internalPointer())->data(4).toULongLong();

	propertiesDialog->databasePath = dm->getDatabase();
	propertiesDialog->basePath = currentPath();
	propertiesDialog->setComics(comics);
	
	propertiesDialog->show();
}

void LibraryWindow::asignNumbers()
{
	QModelIndexList indexList = getSelectedComics();

	int startingNumber = indexList[0].row()+1;
	if(indexList.count()>1)
	{
		bool ok;
		int n = QInputDialog::getInt(this, tr("Asign comics numbers"),
			tr("Asign numbers starting in:"), startingNumber,0,2147483647,1,&ok);
		if (ok)
			startingNumber = n;
		else
			return;
	}
	_comicIdEdited = dmCV->asignNumbers(indexList,startingNumber);
	
	reloadCovers();
}

void LibraryWindow::openContainingFolderComic()
{
	QModelIndex modelIndex = comicView->currentIndex();
	QFileInfo file = QDir::cleanPath(currentPath() + dmCV->getComicPath(modelIndex)); 
	QString path = file.absolutePath();
	QDesktopServices::openUrl(QUrl("file:///"+path, QUrl::TolerantMode));
}

void LibraryWindow::openContainingFolder()
{
	QModelIndex modelIndex = foldersView->currentIndex();
	QString path = QDir::cleanPath(currentPath() + dm->getFolderPath(modelIndex));
	QDesktopServices::openUrl(QUrl("file:///"+path, QUrl::TolerantMode));
}

void LibraryWindow::exportLibrary(QString destPath)
{
	QString currentLibrary = selectedLibrary->currentText();
	QString path = libraries.value(currentLibrary)+"/.yacreaderlibrary";
	packageManager->createPackage(path,destPath+"/"+currentLibrary);
}

void LibraryWindow::importLibrary(QString clc,QString destPath,QString name)
{
	packageManager->extractPackage(clc,destPath+"/"+name);
	openLibrary(destPath+"/"+name,name);
}

void LibraryWindow::reloadOptions()
{
	//comicFlow->setFlowType(flowType);
	comicFlow->updateConfig(settings);
}

QString LibraryWindow::currentPath()
{
	return libraries.value(selectedLibrary->currentText());
}

void LibraryWindow::hideComicFlow(bool hide)
{
	if(hide)
	{
		QList<int> sizes;
		sizes.append(0);
		int total = sVertical->sizes().at(0) + sVertical->sizes().at(1);
		sizes.append(total);
		sVertical->setSizes(sizes);	
	}
	else
	{
		QList<int> sizes;
		int total = sVertical->sizes().at(0) + sVertical->sizes().at(1);
		sizes.append(2*total/3);
		sizes.append(total/3);
		sVertical->setSizes(sizes);	
	}

}

void LibraryWindow::showExportComicsInfo()
{
	exportComicsInfoDialog->source = currentPath() + "/.yacreaderlibrary/library.ydb";
	exportComicsInfoDialog->show();
}

void LibraryWindow::showImportComicsInfo()
{
	importComicsInfoDialog->dest = currentPath() + "/.yacreaderlibrary/library.ydb";
	importComicsInfoDialog->show();
}

void LibraryWindow::closeEvent ( QCloseEvent * event )
{
	settings->setValue(MAIN_WINDOW_GEOMETRY, saveGeometry());
	settings->setValue(COMICS_VIEW_HEADERS,comicView->horizontalHeader()->saveState());
	//settings->setValue(COMICS_VIEW_HEADERS_GEOMETRY,comicView->horizontalHeader()->saveGeometry());
}

void LibraryWindow::showNoLibrariesWidget()
{
	mainWidget->setCurrentIndex(1);
}

void LibraryWindow::showRootWidget()
{
	libraryToolBar->setDisabled(false);
	mainWidget->setCurrentIndex(0);
}

void LibraryWindow::showImportingWidget()
{
	importWidget->clear();
	libraryToolBar->setDisabled(true);
	mainWidget->setCurrentIndex(2);
}

void LibraryWindow::manageCreatingError(const QString & error)
{
	QMessageBox::critical(this,tr("Error creating the library"),error);
}

void LibraryWindow::manageUpdatingError(const QString & error)
{
	QMessageBox::critical(this,tr("Error updating the library"),error);
}

void LibraryWindow::manageOpeningLibraryError(const QString & error)
{
	QMessageBox::critical(this,tr("Error opening the library"),error);
}

QModelIndexList LibraryWindow::getSelectedComics()
{
	//se fuerza a que haya almenos una fila seleccionada TODO comprobar se se puede forzar a la tabla a que lo haga automáticamente
	QModelIndexList selection = comicView->selectionModel()->selectedRows();
	if(selection.count()==0)
	{
		comicView->selectRow(comicFlow->centerIndex());
		selection = comicView->selectionModel()->selectedRows();
	}
	return selection;
}

void LibraryWindow::deleteComics()
{
	int ret = QMessageBox::question(this,tr("Delete comics"),tr("All the selected comics will be deleted from your disk. Are you sure?"),QMessageBox::Yes,QMessageBox::No);

	if(ret == QMessageBox::Yes)
	{

		QModelIndexList indexList = getSelectedComics();

		QList<ComicDB> comics = dmCV->getComics(indexList);

		QList<QString> paths;
		QString libraryPath = currentPath();
		foreach(ComicDB comic, comics)
		{
			paths.append(libraryPath + comic.path);
		}

		ComicsRemover * remover = new ComicsRemover(indexList,paths);

		//comicView->showDeleteProgress();
		dmCV->startTransaction();

		connect(remover, SIGNAL(remove(int)), dmCV, SLOT(remove(int)));
		connect(remover, SIGNAL(remove(int)), comicFlow, SLOT(remove(int)));
		connect(remover, SIGNAL(finished()), dmCV, SLOT(finishTransaction()));
		//connect(remover, SIGNAL(finished()), comicView, SLOT(hideDeleteProgress()));
		connect(remover, SIGNAL(finished()),this,SLOT(checkEmptyFolder()));
		connect(remover, SIGNAL(finished()), remover, SLOT(deleteLater()));
		//connect(remover, SIGNAL(finished()), thread, SLOT(deleteLater()));

		remover->start();
	}
}

/*
void LibraryWindow::showSocial()
{	
	socialDialog->move(this->mapToGlobal(QPoint(width()-socialDialog->width()-10, centralWidget()->pos().y()+10)));

	QModelIndexList indexList = getSelectedComics();

	ComicDB comic = dmCV->getComic(indexList.at(0));

	socialDialog->setComic(comic,currentPath());
	socialDialog->setHidden(false);
}*/
