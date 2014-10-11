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
#include <QHeaderView>

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

#include "yacreader_sidebar.h"

#include "comics_remover.h"
#include "yacreader_library_list_widget.h"
#include "yacreader_treeview.h"

#include "comic_vine_dialog.h"
#include "api_key_dialog.h"
//#include "yacreader_social_dialog.h"

#include "classic_comics_view.h"
#include "grid_comics_view.h"
#include "comics_view_transition.h"
#include "empty_folder_widget.h"

#include "edit_shortcuts_dialog.h"
#include "shortcuts_manager.h"

#include "no_search_results_widget.h"

#include "comic_files_manager.h"

#include "QsLog.h"

#ifdef Q_OS_WIN
    #include <shellapi.h>
#endif

#ifdef Q_OS_MAC
//#include <QtMacExtras>
#endif

LibraryWindow::LibraryWindow()
	:QMainWindow(),fullscreen(false),fetching(false),previousFilter(""),removeError(false)
{
	setupUI();
	loadLibraries();

	if(libraries.isEmpty())
	{
		showNoLibrariesWidget();
	}
	else
	{
		showRootWidget();
		selectedLibrary->setCurrentIndex(0);
	}
}

void LibraryWindow::setupUI()
{
	setWindowIcon(QIcon(":/images/iconLibrary.png"));

	setUnifiedTitleAndToolBarOnMac(true);

	libraryCreator = new LibraryCreator();
	packageManager = new PackageManager();

	settings = new QSettings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
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

	/*if(settings->contains(COMICS_VIEW_HEADERS_GEOMETRY))
        comicsView->horizontalHeader()->restoreGeometry(settings->value(COMICS_VIEW_HEADERS_GEOMETRY).toByteArray());*/

	/*socialDialog = new YACReaderSocialDialog(this);
	socialDialog->setHidden(true);*/
}

void LibraryWindow::doLayout()
{
	//LAYOUT ELEMENTS------------------------------------------------------------
	//---------------------------------------------------------------------------

	QSplitter * sHorizontal = new QSplitter(Qt::Horizontal);  //spliter principal
#ifdef Q_OS_MAC
	sHorizontal->setStyleSheet("QSplitter::handle{image:none;background-color:#B8B8B8;} QSplitter::handle:vertical {height:1px;}");
#else
	sHorizontal->setStyleSheet("QSplitter::handle:vertical {height:4px;}");
#endif

	//TOOLBARS-------------------------------------------------------------------
	//---------------------------------------------------------------------------
	editInfoToolBar = new QToolBar();
    editInfoToolBar->setStyleSheet("QToolBar {border: none;}");

#ifdef Q_OS_MAC
	libraryToolBar = addToolBar(tr("Library"));
#else
	libraryToolBar = new YACReaderMainToolBar(this);
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

    //FOLDERS FILTER-------------------------------------------------------------
    //---------------------------------------------------------------------------
    searchEdit = new YACReaderSearchLineEdit();

    //SIDEBAR--------------------------------------------------------------------
	//---------------------------------------------------------------------------
    sideBar = new YACReaderSideBar;

	foldersView = sideBar->foldersView;
	selectedLibrary = sideBar->selectedLibrary;

	YACReaderTitledToolBar * librariesTitle = sideBar->librariesTitle;

	YACReaderTitledToolBar * foldersTitle = sideBar->foldersTitle;

	librariesTitle->addAction(createLibraryAction);
	librariesTitle->addAction(openLibraryAction);
	librariesTitle->addSpacing(3);

	foldersTitle->addAction(setRootIndexAction);
	foldersTitle->addAction(expandAllNodesAction);
	foldersTitle->addAction(colapseAllNodesAction);

	//FINAL LAYOUT-------------------------------------------------------------
    comicsViewStack = new QStackedWidget();

    if(!settings->contains(COMICS_VIEW_STATUS) || settings->value(COMICS_VIEW_STATUS) == Flow) {
        comicsView = classicComicsView = new ClassicComicsView();
        comicsViewStatus = Flow;
        //comicsViewStack->setCurrentIndex(Flow);
    } else {
        comicsView = gridComicsView = new GridComicsView();
        comicsViewStatus = Grid;
        //comicsViewStack->setCurrentIndex(Grid);
    }

    doComicsViewConnections();

    comicsView->setToolBar(editInfoToolBar);
    comicsViewStack->addWidget(comicsViewTransition = new ComicsViewTransition());
    comicsViewStack->addWidget(emptyFolderWidget = new EmptyFolderWidget());
    comicsViewStack->addWidget(noSearchResultsWidget = new NoSearchResultsWidget());
    comicsViewStack->addWidget(comicsView);

    comicsViewStack->setCurrentWidget(comicsView);

	sHorizontal->addWidget(sideBar);
#ifndef Q_OS_MAC
	QVBoxLayout * rightLayout = new QVBoxLayout;
	rightLayout->addWidget(libraryToolBar);
    rightLayout->addWidget(comicsViewStack);

	rightLayout->setMargin(0);
	rightLayout->setSpacing(0);

	QWidget * rightWidget = new QWidget();
	rightWidget->setLayout(rightLayout);

	sHorizontal->addWidget(rightWidget);
#else
    sHorizontal->addWidget(comicsViewStack);
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



	//collapsible disabled in macosx (only temporaly)
#ifdef Q_OS_MAC
	sHorizontal->setCollapsible(0,false);
#endif
}

void LibraryWindow::doDialogs()
{
	createLibraryDialog = new CreateLibraryDialog(this);
	renameLibraryDialog = new RenameLibraryDialog(this);
	propertiesDialog = new PropertiesDialog(this);
	comicVineDialog = new ComicVineDialog(this);
	exportLibraryDialog = new ExportLibraryDialog(this);
	importLibraryDialog = new ImportLibraryDialog(this);
	exportComicsInfoDialog = new ExportComicsInfoDialog(this);
	importComicsInfoDialog = new ImportComicsInfoDialog(this);
	addLibraryDialog = new AddLibraryDialog(this);
	optionsDialog = new OptionsDialog(this);
	optionsDialog->restoreOptions(settings);

    editShortcutsDialog = new EditShortcutsDialog(this);
    setUpShortcutsManagement();

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

void LibraryWindow::setUpShortcutsManagement()
{

    QList<QAction *> allActions;
    QList<QAction *> tmpList;

    editShortcutsDialog->addActionsGroup("Comics",QIcon(":/images/shortcuts_group_comics.png"),
                                     tmpList = QList<QAction *>()
                                     << openComicAction
                                     << setAsReadAction
                                     << setAsNonReadAction
                                     << openContainingFolderComicAction
                                     << resetComicRatingAction
                                     << selectAllComicsAction
                                     << editSelectedComicsAction
                                     << asignOrderAction
                                     << deleteComicsAction
                                     << getInfoAction);

    allActions << tmpList;

    editShortcutsDialog->addActionsGroup("Folders",QIcon(":/images/shortcuts_group_folders.png"),
                                     tmpList = QList<QAction *>()
                                     << setRootIndexAction
                                     << expandAllNodesAction
                                     << colapseAllNodesAction
                                     << openContainingFolderAction
                                     << setFolderAsNotCompletedAction
                                     << setFolderAsCompletedAction
                                     << setFolderAsReadAction
                                     << setFolderAsUnreadAction);
    allActions << tmpList;

    editShortcutsDialog->addActionsGroup("General",QIcon(":/images/shortcuts_group_general.png"),
                                     tmpList = QList<QAction *>()
                                     << backAction
                                     << forwardAction
                                     << helpAboutAction
                                     << optionsAction
                                     << serverConfigAction
                                     << showEditShortcutsAction);

    allActions << tmpList;

    editShortcutsDialog->addActionsGroup("Libraries",QIcon(":/images/shortcuts_group_libraries.png"),
                                     tmpList = QList<QAction *>()
                                     << createLibraryAction
                                     << openLibraryAction
                                     << exportComicsInfoAction
                                     << importComicsInfoAction
                                     << exportLibraryAction
                                     << importLibraryAction
                                     << updateLibraryAction
                                     << renameLibraryAction
                                     << removeLibraryAction);

    allActions << tmpList;

    editShortcutsDialog->addActionsGroup("Visualization",QIcon(":/images/shortcuts_group_visualization.png"),
                                     tmpList = QList<QAction *>()
                                     << showHideMarksAction
                                     << toggleFullScreenAction
                                     << toggleComicsViewAction
                                     << hideComicViewAction);

    allActions << tmpList;

    ShortcutsManager::getShortcutsManager().registerActions(allActions);
}

void LibraryWindow::doModels()
{
	//folders
    foldersModel = new TreeModel();
	//comics
    comicsModel =  new TableModel();

    setSearchFilter("");
}

void LibraryWindow::disconnectComicsViewConnections(ComicsView * widget)
{
    disconnect(widget, SIGNAL(comicRated(int,QModelIndex)), comicsModel, SLOT(updateRating(int,QModelIndex)));
    disconnect(showHideMarksAction,SIGNAL(toggled(bool)),widget,SLOT(setShowMarks(bool)));
    disconnect(widget,SIGNAL(selected(unsigned int)),this,SLOT(openComic()));
    disconnect(widget,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(openComic()));
    disconnect(selectAllComicsAction,SIGNAL(triggered()),widget,SLOT(selectAll()));
    disconnect(comicsView, SIGNAL(copyComicsToCurrentFolder(QList<QString>)), this, SLOT(copyAndImportComicsToCurrentFolder(QList<QString>)));
    disconnect(comicsView, SIGNAL(moveComicsToCurrentFolder(QList<QString>)), this, SLOT(moveAndImportComicsToCurrentFolder(QList<QString>)));
}

void LibraryWindow::doComicsViewConnections()
{
    connect(comicsView, SIGNAL(comicRated(int,QModelIndex)), comicsModel, SLOT(updateRating(int,QModelIndex)));
    connect(showHideMarksAction,SIGNAL(toggled(bool)),comicsView,SLOT(setShowMarks(bool)));
    connect(comicsView,SIGNAL(selected(unsigned int)),this,SLOT(openComic()));
    connect(comicsView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(openComic()));
    connect(selectAllComicsAction,SIGNAL(triggered()),comicsView,SLOT(selectAll()));
    //Drops
    connect(comicsView, SIGNAL(copyComicsToCurrentFolder(QList<QString>)), this, SLOT(copyAndImportComicsToCurrentFolder(QList<QString>)));
    connect(comicsView, SIGNAL(moveComicsToCurrentFolder(QList<QString>)), this, SLOT(moveAndImportComicsToCurrentFolder(QList<QString>)));
}

void LibraryWindow::createActions()
{
	backAction = new QAction(this);
	QIcon icoBackButton;
	icoBackButton.addPixmap(QPixmap(":/images/main_toolbar/back.png"), QIcon::Normal);
	//icoBackButton.addPixmap(QPixmap(":/images/main_toolbar/back_disabled.png"), QIcon::Disabled);
    backAction->setData(BACK_ACTION_YL);
    backAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(BACK_ACTION_YL));
    backAction->setIcon(icoBackButton);
	backAction->setDisabled(true);

	forwardAction = new QAction(this);
	QIcon icoFordwardButton;
	icoFordwardButton.addPixmap(QPixmap(":/images/main_toolbar/forward.png"), QIcon::Normal);
	//icoFordwardButton.addPixmap(QPixmap(":/images/main_toolbar/forward_disabled.png"), QIcon::Disabled);
    forwardAction->setData(FORWARD_ACTION_YL);
    forwardAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(FORWARD_ACTION_YL));
	forwardAction->setIcon(icoFordwardButton);
	forwardAction->setDisabled(true);

	createLibraryAction = new QAction(this);
	createLibraryAction->setToolTip(tr("Create a new library"));
    createLibraryAction->setData(CREATE_LIBRARY_ACTION_YL);
    createLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(CREATE_LIBRARY_ACTION_YL));
	createLibraryAction->setIcon(QIcon(":/images/newLibraryIcon.png"));

	openLibraryAction = new QAction(this);
	openLibraryAction->setToolTip(tr("Open an existing library"));
    openLibraryAction->setData(OPEN_LIBRARY_ACTION_YL);
    openLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_LIBRARY_ACTION_YL));
	openLibraryAction->setIcon(QIcon(":/images/openLibraryIcon.png"));

    exportComicsInfoAction = new QAction(tr("Export comics info"),this);
    exportComicsInfoAction->setToolTip(tr("Export comics info"));
    exportComicsInfoAction->setData(EXPORT_COMICS_INFO_ACTION_YL);
    exportComicsInfoAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(EXPORT_COMICS_INFO_ACTION_YL));
    exportComicsInfoAction->setIcon(QIcon(":/images/exportComicsInfoIcon.png"));

    importComicsInfoAction = new QAction(tr("Import comics info"),this);
    importComicsInfoAction->setToolTip(tr("Import comics info"));
    importComicsInfoAction->setData(IMPORT_COMICS_INFO_ACTION_YL);
    importComicsInfoAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(IMPORT_COMICS_INFO_ACTION_YL));
    importComicsInfoAction->setIcon(QIcon(":/images/importComicsInfoIcon.png"));

	exportLibraryAction = new QAction(tr("Pack covers"),this);
	exportLibraryAction->setToolTip(tr("Pack the covers of the selected library"));
    exportLibraryAction->setData(EXPORT_LIBRARY_ACTION_YL);
    exportLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(EXPORT_LIBRARY_ACTION_YL));
	exportLibraryAction->setIcon(QIcon(":/images/exportLibraryIcon.png"));

	importLibraryAction = new QAction(tr("Unpack covers"),this);
	importLibraryAction->setToolTip(tr("Unpack a catalog"));
    importLibraryAction->setData(IMPORT_LIBRARY_ACTION_YL);
    importLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(IMPORT_LIBRARY_ACTION_YL));
	importLibraryAction->setIcon(QIcon(":/images/importLibraryIcon.png"));

	updateLibraryAction = new QAction(tr("Update library"),this);
	updateLibraryAction->setToolTip(tr("Update current library"));
    updateLibraryAction->setData(UPDATE_LIBRARY_ACTION_YL);
    updateLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(UPDATE_LIBRARY_ACTION_YL));
	updateLibraryAction->setIcon(QIcon(":/images/updateLibraryIcon.png"));

	renameLibraryAction = new QAction(tr("Rename library"),this);
	renameLibraryAction->setToolTip(tr("Rename current library"));
    renameLibraryAction->setData(RENAME_LIBRARY_ACTION_YL);
    renameLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(RENAME_LIBRARY_ACTION_YL));
	renameLibraryAction->setIcon(QIcon(":/images/editIcon.png"));

	removeLibraryAction = new QAction(tr("Remove library"),this);
	removeLibraryAction->setToolTip(tr("Remove current library from your collection"));
    removeLibraryAction->setData(REMOVE_LIBRARY_ACTION_YL);
    removeLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(REMOVE_LIBRARY_ACTION_YL));
	removeLibraryAction->setIcon(QIcon(":/images/removeLibraryIcon.png"));

	openComicAction = new QAction(tr("Open current comic"),this);
	openComicAction->setToolTip(tr("Open current comic on YACReader"));
    openComicAction->setData(OPEN_COMIC_ACTION_YL);
    openComicAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_COMIC_ACTION_YL));
	openComicAction->setIcon(QIcon(":/images/openInYACReader.png"));

	setAsReadAction = new QAction(tr("Set as read"),this);
	setAsReadAction->setToolTip(tr("Set comic as read"));
    setAsReadAction->setData(SET_AS_READ_ACTION_YL);
    setAsReadAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_AS_READ_ACTION_YL));
	setAsReadAction->setIcon(QIcon(":/images/setReadButton.png"));

	setAsNonReadAction = new QAction(tr("Set as unread"),this);
	setAsNonReadAction->setToolTip(tr("Set comic as unread"));
    setAsNonReadAction->setData(SET_AS_NON_READ_ACTION_YL);
    setAsNonReadAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_AS_NON_READ_ACTION_YL));
	setAsNonReadAction->setIcon(QIcon(":/images/setUnread.png"));

	/*setAllAsReadAction = new QAction(tr("Set all as read"),this);
	setAllAsReadAction->setToolTip(tr("Set all comics as read"));
	setAllAsReadAction->setIcon(QIcon(":/images/setAllRead.png"));

	setAllAsNonReadAction = new QAction(tr("Set all as unread"),this);
	setAllAsNonReadAction->setToolTip(tr("Set all comics as unread"));
	setAllAsNonReadAction->setIcon(QIcon(":/images/setAllUnread.png"));*/

	showHideMarksAction = new QAction(tr("Show/Hide marks"),this);
	showHideMarksAction->setToolTip(tr("Show or hide readed marks"));
    showHideMarksAction->setData(SHOW_HIDE_MARKS_ACTION_YL);
    showHideMarksAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_HIDE_MARKS_ACTION_YL));
	showHideMarksAction->setCheckable(true);
	showHideMarksAction->setIcon(QIcon(":/images/showMarks.png"));
	showHideMarksAction->setChecked(true);

	toggleFullScreenAction = new QAction(tr("Fullscreen mode on/off"),this);
    toggleFullScreenAction->setToolTip(tr("Fullscreen mode on/off"));
    toggleFullScreenAction->setData(TOGGLE_FULL_SCREEN_ACTION_YL);
    toggleFullScreenAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_FULL_SCREEN_ACTION_YL));
	QIcon icoFullscreenButton;
	icoFullscreenButton.addPixmap(QPixmap(":/images/main_toolbar/fullscreen.png"), QIcon::Normal);
	toggleFullScreenAction->setIcon(icoFullscreenButton);

	helpAboutAction = new QAction(this);
	helpAboutAction->setToolTip(tr("Help, About YACReader"));
    helpAboutAction->setData(HELP_ABOUT_ACTION_YL);
    helpAboutAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(HELP_ABOUT_ACTION_YL));
	QIcon icoHelpButton;
	icoHelpButton.addPixmap(QPixmap(":/images/main_toolbar/help.png"), QIcon::Normal);
	helpAboutAction->setIcon(icoHelpButton);

	setRootIndexAction = new QAction(this);
    setRootIndexAction->setData(SET_ROOT_INDEX_ACTION_YL);
    setRootIndexAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_ROOT_INDEX_ACTION_YL));
	setRootIndexAction->setToolTip(tr("Select root node"));
	setRootIndexAction->setIcon(QIcon(":/images/setRoot.png"));

	expandAllNodesAction = new QAction(this);
	expandAllNodesAction->setToolTip(tr("Expand all nodes"));
    expandAllNodesAction->setData(EXPAND_ALL_NODES_ACTION_YL);
    expandAllNodesAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(EXPAND_ALL_NODES_ACTION_YL));
	expandAllNodesAction->setIcon(QIcon(":/images/expand.png"));

	colapseAllNodesAction = new QAction(this);
	colapseAllNodesAction->setToolTip(tr("Colapse all nodes"));
    colapseAllNodesAction->setData(COLAPSE_ALL_NODES_ACTION_YL);
    colapseAllNodesAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(COLAPSE_ALL_NODES_ACTION_YL));
	colapseAllNodesAction->setIcon(QIcon(":/images/colapse.png"));

	optionsAction = new QAction(this);
	optionsAction->setToolTip(tr("Show options dialog"));
    optionsAction->setData(OPTIONS_ACTION_YL);
    optionsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPTIONS_ACTION_YL));
	QIcon icoSettingsButton;
	icoSettingsButton.addPixmap(QPixmap(":/images/main_toolbar/settings.png"), QIcon::Normal);
	optionsAction->setIcon(icoSettingsButton);

	serverConfigAction = new QAction(this);
	serverConfigAction->setToolTip(tr("Show comics server options dialog"));
    serverConfigAction->setData(SERVER_CONFIG_ACTION_YL);
    serverConfigAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SERVER_CONFIG_ACTION_YL));
	QIcon icoServerButton;
	icoServerButton.addPixmap(QPixmap(":/images/main_toolbar/server.png"), QIcon::Normal);
	serverConfigAction->setIcon(icoServerButton);

    toggleComicsViewAction = new QAction(tr("Change between comics views"),this);
    toggleComicsViewAction->setToolTip(tr("Change between comics views"));
    QIcon icoViewsButton;
    if(!settings->contains(COMICS_VIEW_STATUS) || settings->value(COMICS_VIEW_STATUS) == Flow)
        icoViewsButton.addPixmap(QPixmap(":/images/main_toolbar/grid.png"), QIcon::Normal);
    else
        icoViewsButton.addPixmap(QPixmap(":/images/main_toolbar/flow.png"), QIcon::Normal);
    toggleComicsViewAction->setData(TOGGLE_COMICS_VIEW_ACTION_YL);
    toggleComicsViewAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_COMICS_VIEW_ACTION_YL));
    toggleComicsViewAction->setIcon(icoViewsButton);
	//socialAction = new QAction(this);

	openContainingFolderAction = new QAction(this);
	openContainingFolderAction->setText(tr("Open folder..."));
    openContainingFolderAction->setData(OPEN_CONTAINING_FOLDER_ACTION_YL);
    openContainingFolderAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_CONTAINING_FOLDER_ACTION_YL));
	openContainingFolderAction->setIcon(QIcon(":/images/open.png"));

    setFolderAsNotCompletedAction = new QAction(this);
    setFolderAsNotCompletedAction->setText(tr("Set as uncompleted"));
    setFolderAsNotCompletedAction->setVisible(false);
    setFolderAsNotCompletedAction->setData(SET_FOLDER_AS_NOT_COMPLETED_ACTION_YL);
    setFolderAsNotCompletedAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_NOT_COMPLETED_ACTION_YL));

    setFolderAsCompletedAction = new QAction(this);
    setFolderAsCompletedAction->setText(tr("Set as completed"));
    setFolderAsCompletedAction->setVisible(false);
    setFolderAsCompletedAction->setData(SET_FOLDER_AS_COMPLETED_ACTION_YL);
    setFolderAsCompletedAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_COMPLETED_ACTION_YL));

    setFolderAsReadAction = new QAction(this);
    setFolderAsReadAction->setText(tr("Set as read"));
    setFolderAsReadAction->setVisible(false);
    setFolderAsReadAction->setData(SET_FOLDER_AS_READ_ACTION_YL);
    setFolderAsReadAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_READ_ACTION_YL));

    setFolderAsUnreadAction = new QAction(this);
    setFolderAsUnreadAction->setText(tr("Set as unread"));
    setFolderAsUnreadAction->setVisible(false);
    setFolderAsUnreadAction->setData(SET_FOLDER_AS_UNREAD_ACTION_YL);
    setFolderAsUnreadAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_UNREAD_ACTION_YL));

	openContainingFolderComicAction = new QAction(this);
	openContainingFolderComicAction->setText(tr("Open containing folder..."));
    openContainingFolderComicAction->setData(OPEN_CONTAINING_FOLDER_COMIC_ACTION_YL);
    openContainingFolderComicAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_CONTAINING_FOLDER_COMIC_ACTION_YL));
	openContainingFolderComicAction->setIcon(QIcon(":/images/open.png"));

    resetComicRatingAction = new QAction(this);
    resetComicRatingAction->setText(tr("Reset comic rating"));
    resetComicRatingAction->setData(RESET_COMIC_RATING_ACTION_YL);
    resetComicRatingAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(RESET_COMIC_RATING_ACTION_YL));

	//Edit comics actions------------------------------------------------------
	selectAllComicsAction = new QAction(this);
	selectAllComicsAction->setText(tr("Select all comics"));
    selectAllComicsAction->setData(SELECT_ALL_COMICS_ACTION_YL);
    selectAllComicsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SELECT_ALL_COMICS_ACTION_YL));
    selectAllComicsAction->setIcon(QIcon(":/images/selectAll.png"));

	editSelectedComicsAction = new QAction(this);
	editSelectedComicsAction->setText(tr("Edit"));
    editSelectedComicsAction->setData(EDIT_SELECTED_COMICS_ACTION_YL);
    editSelectedComicsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(EDIT_SELECTED_COMICS_ACTION_YL));
	editSelectedComicsAction->setIcon(QIcon(":/images/editComic.png"));

    asignOrderAction = new QAction(this);
    asignOrderAction->setText(tr("Asign current order to comics"));
    asignOrderAction->setData(ASIGN_ORDER_ACTION_YL);
    asignOrderAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ASIGN_ORDER_ACTION_YL));
    asignOrderAction->setIcon(QIcon(":/images/asignNumber.png"));

    forceCoverExtractedAction = new QAction(this);
    forceCoverExtractedAction->setText(tr("Update cover"));
    forceCoverExtractedAction->setData(FORCE_COVER_EXTRACTED_ACTION_YL);
    forceCoverExtractedAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(FORCE_COVER_EXTRACTED_ACTION_YL));
    forceCoverExtractedAction->setIcon(QIcon(":/images/importCover.png"));

	deleteComicsAction = new QAction(this);
	deleteComicsAction->setText(tr("Delete selected comics"));
    deleteComicsAction->setData(DELETE_COMICS_ACTION_YL);
    deleteComicsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(DELETE_COMICS_ACTION_YL));
	deleteComicsAction->setIcon(QIcon(":/images/trash.png"));

    hideComicViewAction = new QAction(this);
    hideComicViewAction->setText(tr("Hide comic flow"));
    hideComicViewAction->setData(HIDE_COMIC_VIEW_ACTION_YL);
    hideComicViewAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(HIDE_COMIC_VIEW_ACTION_YL));
    hideComicViewAction->setIcon(QIcon(":/images/hideComicFlow.png"));
    hideComicViewAction->setCheckable(true);
    hideComicViewAction->setChecked(false);

	getInfoAction = new QAction(this);
    getInfoAction->setData(GET_INFO_ACTION_YL);
    getInfoAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(GET_INFO_ACTION_YL));
	getInfoAction->setText(tr("Download tags from Comic Vine"));
	getInfoAction->setIcon(QIcon(":/images/getInfo.png"));
	//-------------------------------------------------------------------------

    showEditShortcutsAction = new QAction(tr("Edit shortcuts"),this);
    showEditShortcutsAction->setData(SHOW_EDIT_SHORTCUTS_ACTION_YL);
    showEditShortcutsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_EDIT_SHORTCUTS_ACTION_YL));
    showEditShortcutsAction->setShortcutContext(Qt::ApplicationShortcut);
    addAction(showEditShortcutsAction);
	//disable actions
	disableAllActions();
}
void LibraryWindow::disableComicsActions(bool disabled)
{
	//if there aren't comics, no fullscreen option will be available
	toggleFullScreenAction->setDisabled(disabled);
	//edit toolbar
	openComicAction->setDisabled(disabled);
	editSelectedComicsAction->setDisabled(disabled);
	selectAllComicsAction->setDisabled(disabled);
    asignOrderAction->setDisabled(disabled);
	setAsReadAction->setDisabled(disabled);
	setAsNonReadAction->setDisabled(disabled);
	//setAllAsReadAction->setDisabled(disabled);
	//setAllAsNonReadAction->setDisabled(disabled);
	showHideMarksAction->setDisabled(disabled);
	deleteComicsAction->setDisabled(disabled);
	//context menu
	openContainingFolderComicAction->setDisabled(disabled);
    resetComicRatingAction->setDisabled(disabled);

	getInfoAction->setDisabled(disabled);


}
void LibraryWindow::disableLibrariesActions(bool disabled)
{
	updateLibraryAction->setDisabled(disabled);
	renameLibraryAction->setDisabled(disabled);
	removeLibraryAction->setDisabled(disabled);
    exportComicsInfoAction->setDisabled(disabled);
    importComicsInfoAction->setDisabled(disabled);
	exportLibraryAction->setDisabled(disabled);
	//importLibraryAction->setDisabled(disabled);
}

void LibraryWindow::disableNoUpdatedLibrariesActions(bool disabled)
{
	updateLibraryAction->setDisabled(disabled);
    exportComicsInfoAction->setDisabled(disabled);
    importComicsInfoAction->setDisabled(disabled);
	exportLibraryAction->setDisabled(disabled);
}

void LibraryWindow::disableFoldersActions(bool disabled)
{
	setRootIndexAction->setDisabled(disabled);
	expandAllNodesAction->setDisabled(disabled);
	colapseAllNodesAction->setDisabled(disabled);

	openContainingFolderAction->setDisabled(disabled);

    if(disabled == false)
    {
        setFolderAsNotCompletedAction->setVisible(false);
        setFolderAsCompletedAction->setVisible(false);
        setFolderAsReadAction->setVisible(false);
        setFolderAsUnreadAction->setVisible(false);
    }
}

void LibraryWindow::disableAllActions()
{
	disableComicsActions(true);
	disableLibrariesActions(true);
	disableFoldersActions(true);
}

void LibraryWindow::createToolBars()
{

#ifdef Q_OS_MAC
	libraryToolBar->setIconSize(QSize(16,16)); //TODO make icon size dynamic

	libraryToolBar->addAction(backAction);
	libraryToolBar->addAction(forwardAction);

	{QWidget * w = new QWidget();
	w->setFixedWidth(10);
	libraryToolBar->addWidget(w);}

#ifdef SERVER_RELEASE
	libraryToolBar->addAction(serverConfigAction);
#endif
	libraryToolBar->addAction(optionsAction);
	libraryToolBar->addAction(helpAboutAction);

	{ QWidget * w2 = new QWidget();
	 w2->setFixedWidth(10);
	 libraryToolBar->addWidget(w2);}

    libraryToolBar->addAction(toggleComicsViewAction);
	libraryToolBar->addAction(toggleFullScreenAction);

	libraryToolBar->addWidget(new QToolBarStretch());
	libraryToolBar->addWidget(foldersFilter);

	libraryToolBar->setMovable(false);

	
#else
	libraryToolBar->backButton->setDefaultAction(backAction);
	libraryToolBar->forwardButton->setDefaultAction(forwardAction);
	libraryToolBar->settingsButton->setDefaultAction(optionsAction);
	libraryToolBar->serverButton->setDefaultAction(serverConfigAction);
	libraryToolBar->helpButton->setDefaultAction(helpAboutAction);
    libraryToolBar->toggleComicsViewButton->setDefaultAction(toggleComicsViewAction);
	libraryToolBar->fullscreenButton->setDefaultAction(toggleFullScreenAction);
    libraryToolBar->setSearchWidget(searchEdit);
#endif

	editInfoToolBar->setIconSize(QSize(18,18));
	editInfoToolBar->addAction(openComicAction);
	editInfoToolBar->addSeparator();
	editInfoToolBar->addAction(editSelectedComicsAction);
	editInfoToolBar->addAction(getInfoAction);
    editInfoToolBar->addAction(asignOrderAction);
	
	editInfoToolBar->addSeparator();
	
	editInfoToolBar->addAction(selectAllComicsAction);

	editInfoToolBar->addSeparator();

	editInfoToolBar->addAction(setAsReadAction);
	//editInfoToolBar->addAction(setAllAsReadAction);
	editInfoToolBar->addAction(setAsNonReadAction);
	//editInfoToolBar->addAction(setAllAsNonReadAction);

	editInfoToolBar->addAction(showHideMarksAction);

	editInfoToolBar->addSeparator();
	
	editInfoToolBar->addAction(deleteComicsAction);

	editInfoToolBar->addWidget(new QToolBarStretch());
    editInfoToolBar->addAction(hideComicViewAction);
}

void LibraryWindow::createMenus()
{
    itemActions << openComicAction
                << YACReader::createSeparator()
                << openContainingFolderComicAction
                << YACReader::createSeparator()
                << resetComicRatingAction
                << YACReader::createSeparator()
                << editSelectedComicsAction
                << getInfoAction
                << asignOrderAction
                << YACReader::createSeparator()
                << setAsReadAction
                << setAsNonReadAction
                << YACReader::createSeparator()
                << deleteComicsAction;

    viewActions << openComicAction
                << YACReader::createSeparator()
                << openContainingFolderComicAction
                << YACReader::createSeparator()
                << resetComicRatingAction
                << YACReader::createSeparator()
                << editSelectedComicsAction
                << getInfoAction
                << asignOrderAction
                << YACReader::createSeparator()
                << selectAllComicsAction
                << YACReader::createSeparator()
                << setAsReadAction
                << setAsNonReadAction
                << showHideMarksAction
                << YACReader::createSeparator()
                << deleteComicsAction
                << YACReader::createSeparator()
                << toggleFullScreenAction;

    comicsView->setItemActions(itemActions);
    comicsView->setViewActions(viewActions);

	foldersView->addAction(openContainingFolderAction);
    YACReader::addSperator(foldersView);

    foldersView->addAction(setFolderAsNotCompletedAction);
    foldersView->addAction(setFolderAsCompletedAction);
    YACReader::addSperator(foldersView);

    foldersView->addAction(setFolderAsReadAction);
    foldersView->addAction(setFolderAsUnreadAction);

	selectedLibrary->addAction(updateLibraryAction); 
	selectedLibrary->addAction(renameLibraryAction);
	selectedLibrary->addAction(removeLibraryAction);
    YACReader::addSperator(selectedLibrary);

    selectedLibrary->addAction(exportComicsInfoAction);
    selectedLibrary->addAction(importComicsInfoAction);
    YACReader::addSperator(selectedLibrary);

	selectedLibrary->addAction(exportLibraryAction);
	selectedLibrary->addAction(importLibraryAction);



	
//MacOSX app menus
#ifdef Q_OS_MACX
    QMenuBar * menu = this->menuBar();
    //about / preferences
    //TODO

    //library
    QMenu * libraryMenu = new QMenu(tr("Library"));

    libraryMenu->addAction(updateLibraryAction);
    libraryMenu->addAction(renameLibraryAction);
    libraryMenu->addAction(removeLibraryAction);
    libraryMenu->addSeparator();

    libraryMenu->addAction(exportComicsInfoAction);
    libraryMenu->addAction(importComicsInfoAction);

    libraryMenu->addSeparator();

    libraryMenu->addAction(exportLibraryAction);
    libraryMenu->addAction(importLibraryAction);

    //folder
    QMenu * folderMenu = new QMenu(tr("Folder"));
    folderMenu->addAction(openContainingFolderAction);
    folderMenu->addSeparator();
    folderMenu->addAction(setFolderAsNotCompletedAction);
    folderMenu->addAction(setFolderAsCompletedAction);
    folderMenu->addSeparator();
    folderMenu->addAction(setFolderAsReadAction);
    folderMenu->addAction(setFolderAsUnreadAction);

    //comic
    QMenu * comicMenu = new QMenu(tr("Comic"));
    comicMenu->addAction(openContainingFolderComicAction);
    comicMenu->addSeparator();
    comicMenu->addAction(resetComicRatingAction);

    menu->addMenu(libraryMenu);
    menu->addMenu(folderMenu);
    menu->addMenu(comicMenu);
#endif
}

void LibraryWindow::createConnections()
{
	//history navigation
	connect(backAction,SIGNAL(triggered()),this,SLOT(backward()));
	connect(forwardAction,SIGNAL(triggered()),this,SLOT(forward()));

	//libraryCreator connections
	connect(createLibraryDialog,SIGNAL(createLibrary(QString,QString,QString)),this,SLOT(create(QString,QString,QString)));
	connect(createLibraryDialog,SIGNAL(libraryExists(QString)),this,SLOT(libraryAlreadyExists(QString)));
	connect(importComicsInfoDialog,SIGNAL(finished(int)),this,SLOT(reloadCurrentLibrary()));

	//connect(libraryCreator,SIGNAL(coverExtracted(QString)),createLibraryDialog,SLOT(showCurrentFile(QString)));
	//connect(libraryCreator,SIGNAL(coverExtracted(QString)),updateLibraryDialog,SLOT(showCurrentFile(QString)));
	connect(libraryCreator,SIGNAL(finished()),this,SLOT(showRootWidget()));
	connect(libraryCreator,SIGNAL(updated()),this,SLOT(reloadCurrentLibrary()));
	connect(libraryCreator,SIGNAL(created()),this,SLOT(openLastCreated()));
    connect(libraryCreator,SIGNAL(updatedCurrentFolder()), this, SLOT(showRootWidget()));
    connect(libraryCreator,SIGNAL(updatedCurrentFolder()), this, SLOT(reloadAfterCopyMove()));
	connect(libraryCreator,SIGNAL(comicAdded(QString,QString)),importWidget,SLOT(newComic(QString,QString)));
	//libraryCreator errors
	connect(libraryCreator,SIGNAL(failedCreatingDB(QString)),this,SLOT(manageCreatingError(QString)));
	connect(libraryCreator,SIGNAL(failedUpdatingDB(QString)),this,SLOT(manageUpdatingError(QString))); //TODO: implement failedUpdatingDB
	
	//new import widget
	connect(importWidget,SIGNAL(stop()),this,SLOT(stopLibraryCreator()));

	//packageManager connections
	connect(exportLibraryDialog,SIGNAL(exportPath(QString)),this,SLOT(exportLibrary(QString)));
	connect(exportLibraryDialog,SIGNAL(rejected()),packageManager,SLOT(cancel()));
	connect(packageManager,SIGNAL(exported()),exportLibraryDialog,SLOT(close()));
	connect(importLibraryDialog,SIGNAL(unpackCLC(QString,QString,QString)),this,SLOT(importLibrary(QString,QString,QString)));
	connect(importLibraryDialog,SIGNAL(rejected()),packageManager,SLOT(cancel()));
	connect(importLibraryDialog,SIGNAL(rejected()),this,SLOT(deleteCurrentLibrary()));
	connect(importLibraryDialog,SIGNAL(libraryExists(QString)),this,SLOT(libraryAlreadyExists(QString)));
	connect(packageManager,SIGNAL(imported()),importLibraryDialog,SLOT(hide()));
	connect(packageManager,SIGNAL(imported()),this,SLOT(openLastCreated()));


	//create and update dialogs
	connect(createLibraryDialog,SIGNAL(cancelCreate()),this,SLOT(cancelCreating()));

	//open existing library from dialog.
	connect(addLibraryDialog,SIGNAL(addLibrary(QString,QString)),this,SLOT(openLibrary(QString,QString)));

	//load library when selected library changes
	connect(selectedLibrary,SIGNAL(currentIndexChanged(QString)),this,SLOT(loadLibrary(QString)));

	//rename library dialog
	connect(renameLibraryDialog,SIGNAL(renameLibrary(QString)),this,SLOT(rename(QString)));

	//navigations between view modes (tree,list and flow)
    connect(foldersView, SIGNAL(pressed(QModelIndex)), this, SLOT(updateFoldersViewConextMenu(QModelIndex)));
	connect(foldersView, SIGNAL(clicked(QModelIndex)), this, SLOT(loadCovers(QModelIndex)));
	connect(foldersView, SIGNAL(clicked(QModelIndex)), this, SLOT(updateHistory(QModelIndex)));

    //drops in folders view
    connect(foldersView, SIGNAL(copyComicsToFolder(QList<QString>,QModelIndex)), this, SLOT(copyAndImportComicsToFolder(QList<QString>,QModelIndex)));
    connect(foldersView, SIGNAL(moveComicsToFolder(QList<QString>,QModelIndex)), this, SLOT(moveAndImportComicsToFolder(QList<QString>,QModelIndex)));

	//actions
	connect(createLibraryAction,SIGNAL(triggered()),this,SLOT(createLibrary()));
	connect(exportLibraryAction,SIGNAL(triggered()),exportLibraryDialog,SLOT(show()));
	connect(importLibraryAction,SIGNAL(triggered()),this,SLOT(importLibraryPackage()));

	connect(openLibraryAction,SIGNAL(triggered()),this,SLOT(showAddLibrary()));
	connect(setAsReadAction,SIGNAL(triggered()),this,SLOT(setCurrentComicReaded()));
	connect(setAsNonReadAction,SIGNAL(triggered()),this,SLOT(setCurrentComicUnreaded()));
	//connect(setAllAsReadAction,SIGNAL(triggered()),this,SLOT(setComicsReaded()));
	//connect(setAllAsNonReadAction,SIGNAL(triggered()),this,SLOT(setComicsUnreaded()));


	//comicsInfoManagement
    connect(exportComicsInfoAction,SIGNAL(triggered()),this,SLOT(showExportComicsInfo()));
    connect(importComicsInfoAction,SIGNAL(triggered()),this,SLOT(showImportComicsInfo()));

	//properties & config
	connect(propertiesDialog,SIGNAL(accepted()),this,SLOT(reloadCovers()));

	//comic vine
	connect(comicVineDialog,SIGNAL(accepted()),this,SLOT(reloadCovers()));

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
    connect(toggleComicsViewAction,SIGNAL(triggered()),this,SLOT(toggleComicsView()));
	connect(optionsAction, SIGNAL(triggered()),optionsDialog,SLOT(show()));
#ifdef SERVER_RELEASE
	connect(serverConfigAction, SIGNAL(triggered()), serverConfigDialog, SLOT(show()));
#endif
	connect(optionsDialog, SIGNAL(optionsChanged()),this,SLOT(reloadOptions()));
    connect(optionsDialog, SIGNAL(editShortcuts()),editShortcutsDialog,SLOT(show()));

	//Folders filter
	//connect(clearFoldersFilter,SIGNAL(clicked()),foldersFilter,SLOT(clear()));
    connect(searchEdit,SIGNAL(textChanged(QString)),this,SLOT(setSearchFilter(QString)));
	//connect(includeComicsCheckBox,SIGNAL(stateChanged(int)),this,SLOT(searchInFiles(int)));

	//ContextMenus
	connect(openContainingFolderComicAction,SIGNAL(triggered()),this,SLOT(openContainingFolderComic()));
    connect(setFolderAsNotCompletedAction,SIGNAL(triggered()),this,SLOT(setFolderAsNotCompleted()));
    connect(setFolderAsCompletedAction,SIGNAL(triggered()),this,SLOT(setFolderAsCompleted()));
    connect(setFolderAsReadAction,SIGNAL(triggered()),this,SLOT(setFolderAsRead()));
    connect(setFolderAsUnreadAction,SIGNAL(triggered()),this,SLOT(setFolderAsUnread()));
    connect(openContainingFolderAction,SIGNAL(triggered()),this,SLOT(openContainingFolder()));
    connect(resetComicRatingAction,SIGNAL(triggered()),this,SLOT(resetComicRating()));

	//connect(dm,SIGNAL(directoryLoaded(QString)),foldersView,SLOT(expandAll()));
	//connect(dm,SIGNAL(directoryLoaded(QString)),this,SLOT(updateFoldersView(QString)));
	//Comicts edition
	connect(editSelectedComicsAction,SIGNAL(triggered()),this,SLOT(showProperties()));
    connect(asignOrderAction,SIGNAL(triggered()),this,SLOT(asignNumbers()));

	connect(deleteComicsAction,SIGNAL(triggered()),this,SLOT(deleteComics()));

    connect(hideComicViewAction, SIGNAL(toggled(bool)),this, SLOT(hideComicFlow(bool)));

	connect(getInfoAction,SIGNAL(triggered()),this,SLOT(showComicVineScraper()));

	//connect(socialAction,SIGNAL(triggered()),this,SLOT(showSocial()));

    connect(comicsViewTransition,SIGNAL(transitionFinished()),this,SLOT(showComicsView()));

    connect(comicsModel,SIGNAL(isEmpty()),this,SLOT(showEmptyFolderView()));
    connect(comicsModel,SIGNAL(searchNumResults(int)),this,SLOT(checkSearchNumResults(int)));
    connect(emptyFolderWidget,SIGNAL(subfolderSelected(QModelIndex,int)),this,SLOT(selectSubfolder(QModelIndex,int)));

    connect(showEditShortcutsAction,SIGNAL(triggered()),editShortcutsDialog,SLOT(show()));
}

void LibraryWindow::loadLibrary(const QString & name)
{
	if(!libraries.isEmpty())  //si hay bibliotecas...
	{	
		currentFolderNavigation=0;
		backAction->setDisabled(true);
		forwardAction->setDisabled(true);
		history.clear();
		history.append(QModelIndex());

		showRootWidget();
		QString path=libraries.getPath(name)+"/.yacreaderlibrary";
		QDir d; //TODO change this by static methods (utils class?? with delTree for example)
		QString dbVersion;
		if(d.exists(path) && d.exists(path+"/library.ydb") && (dbVersion = DataBaseManagement::checkValidDB(path+"/library.ydb")) != "") //si existe en disco la biblioteca seleccionada, y es válida..
		{
			int comparation = DataBaseManagement::compareVersions(dbVersion,VERSION);
			bool updated = false;
			if(comparation < 0)
				{
					int ret = QMessageBox::question(this,tr("Update needed"),tr("This library was created with a previous version of YACReaderLibrary. It needs to be updated. Update now?"),QMessageBox::Yes,QMessageBox::No);
					if(ret == QMessageBox::Yes)
					{
						updated = DataBaseManagement::updateToCurrentVersion(path+"/library.ydb");
						if(!updated)
							QMessageBox::critical(this,tr("Update failed"), tr("The current library can't be udpated. Check for write write permissions on: ") + path+"/library.ydb");
					}
					else
					{
                        comicsView->setModel(NULL);
						foldersView->setModel(NULL);
						disableAllActions();//TODO comprobar que se deben deshabilitar
						//será possible renombrar y borrar estas bibliotecas
						renameLibraryAction->setEnabled(true);
						removeLibraryAction->setEnabled(true);
					}
				}

			if(comparation == 0 || updated) //en caso de que la versión se igual que la actual
			{
				index = 0;

                foldersModel->setupModelData(path);
                foldersView->setModel(foldersModel);

                if(foldersModel->rowCount(QModelIndex())>0)
					disableFoldersActions(false);
				else
					disableFoldersActions(true);

				d.setCurrent(libraries.getPath(name));
				d.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
				if(d.count()<=1) //librería de sólo lectura
				{
					//QMessageBox::critical(NULL,QString::number(d.count()),QString::number(d.count()));
					disableLibrariesActions(false);
					updateLibraryAction->setDisabled(true);
					openContainingFolderAction->setDisabled(true);
					disableComicsActions(true);
					toggleFullScreenAction->setEnabled(true);

					importedCovers = true;
				}
				else //librería normal abierta
				{
					disableLibrariesActions(false);
					importedCovers = false;
				}

				setRootIndex();
				//TODO encontrar el bug que provoca que no se carguen adecuadamente las carátulas en root.
				setRootIndex();

                searchEdit->clear();
			}
			else if(comparation > 0)
			{
					int ret = QMessageBox::question(this,tr("Download new version"),tr("This library was created with a newer version of YACReaderLibrary. Download the new version now?"),QMessageBox::Yes,QMessageBox::No);
					if(ret == QMessageBox::Yes)
						QDesktopServices::openUrl(QUrl("http://www.yacreader.com"));

                    comicsView->setModel(NULL);
					foldersView->setModel(NULL);
					disableAllActions();//TODO comprobar que se deben deshabilitar
					//será possible renombrar y borrar estas bibliotecas
					renameLibraryAction->setEnabled(true);
					removeLibraryAction->setEnabled(true);
			}
		}
		else
		{
            comicsView->setModel(NULL);
			foldersView->setModel(NULL);
			disableAllActions();//TODO comprobar que se deben deshabilitar

			//si la librería no existe en disco, se ofrece al usuario la posibiliad de eliminarla
			if(!d.exists(path))
			{
				QString currentLibrary = selectedLibrary->currentText();
				if(QMessageBox::question(this,tr("Library not available"),tr("Library '%1' is no longer available. Do you want to remove it?").arg(currentLibrary),QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes)
				{
					deleteCurrentLibrary();
				}
				//será possible renombrar y borrar estas bibliotecas
				renameLibraryAction->setEnabled(true);
				removeLibraryAction->setEnabled(true);

			}
			else//si existe el path, puede ser que la librería sea alguna versión pre-5.0 ó que esté corrupta o que no haya drivers sql
			{
				
				if(d.exists(path+"/library.ydb"))
				{
					QSqlDatabase db = DataBaseManagement::loadDatabase(path);
					manageOpeningLibraryError(db.lastError().databaseText() + "-" + db.lastError().driverText());
					//será possible renombrar y borrar estas bibliotecas
					renameLibraryAction->setEnabled(true);
					removeLibraryAction->setEnabled(true);
				}
				else
				{
					QString currentLibrary = selectedLibrary->currentText();
					QString path = libraries.getPath(selectedLibrary->currentText());
					if(QMessageBox::question(this,tr("Old library"),tr("Library '%1' has been created with an older version of YACReaderLibrary. It must be created again. Do you want to create the library now?").arg(currentLibrary),QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes)
					{
						QDir d(path+"/.yacreaderlibrary");
						delTree(d);
						d.rmdir(path+"/.yacreaderlibrary");
						createLibraryDialog->setDataAndStart(currentLibrary,path);
						//create(path,path+"/.yacreaderlibrary",currentLibrary);
					}
					//será possible renombrar y borrar estas bibliotecas
					renameLibraryAction->setEnabled(true);
					removeLibraryAction->setEnabled(true);
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
	unsigned long long int folderId = 1;
	if(mi.isValid())
	{
		TreeItem *item = static_cast<TreeItem*>(mi.internalPointer());
		folderId = item->id;
#ifndef Q_OS_MAC
		libraryToolBar->setCurrentFolderName(item->data(0).toString());
#endif
	}
#ifndef Q_OS_MAC
	else libraryToolBar->setCurrentFolderName(selectedLibrary->currentText());
#endif



	//cambiado de orden, ya que al llamar a foldersFilter->clear() se invalidan los model index
    if(searchEdit->text()!="")
	{
		//setFoldersFilter("");
		if(mi.isValid())
		{
			index = static_cast<TreeItem *>(mi.internalPointer())->originalItem;
			column = mi.column();
            searchEdit->clear();
		}
	}
	else
	{
		index = static_cast<TreeItem *>(mi.internalPointer());
		column = mi.column();
	}

    //comicsView->setModel(NULL);
    comicsModel->setupModelData(folderId,foldersModel->getDatabase());
	
    comicsView->setModel(comicsModel);
    QStringList paths = comicsModel->getPaths(currentPath());
	checkEmptyFolder(&paths);

    if(paths.size()>0) {
        comicsView->setCurrentIndex(comicsModel->index(0,0));
        if(comicsViewStack->currentWidget() == emptyFolderWidget)
            comicsViewStack->setCurrentWidget(comicsView);
    }
    else
        emptyFolderWidget->setSubfolders(mi,foldersModel->getSubfoldersNames(mi));
}

void LibraryWindow::loadCoversFromCurrentModel()
{
    comicsView->setModel(comicsModel);
    QStringList paths = comicsModel->getPaths(currentPath());

    if(paths.size()>0) {
        comicsView->setCurrentIndex(comicsModel->index(0,0));
    }
}

void LibraryWindow::copyAndImportComicsToCurrentFolder(const QList<QString> &comics)
{
    QString destFolderPath = currentFolderPath();

    copyMoveIndexDestination = getCurrentFolderIndex();

    QProgressDialog * progressDialog = newProgressDialog(tr("Copying comics..."),comics.size());

    ComicFilesManager * comicFilesManager = new ComicFilesManager();
    comicFilesManager->copyComicsTo(comics,destFolderPath);

    processComicFiles(comicFilesManager, progressDialog);
}

void LibraryWindow::moveAndImportComicsToCurrentFolder(const QList<QString> &comics)
{
    QString destFolderPath = currentFolderPath();

    copyMoveIndexDestination = getCurrentFolderIndex();

    QProgressDialog * progressDialog = newProgressDialog(tr("Moving comics..."),comics.size());

    ComicFilesManager * comicFilesManager = new ComicFilesManager();
    comicFilesManager->moveComicsTo(comics,destFolderPath);

    processComicFiles(comicFilesManager, progressDialog);
}

void LibraryWindow::copyAndImportComicsToFolder(const QList<QString> &comics, const QModelIndex &miFolder)
{
    if(miFolder.isValid())
    {
        QString destFolderPath = QDir::cleanPath(currentPath()+foldersModel->getFolderPath(miFolder));

        copyMoveIndexDestination = miFolder;

        QLOG_DEBUG() << "Coping to " << destFolderPath;

        QProgressDialog * progressDialog = newProgressDialog(tr("Copying comics..."),comics.size());

        ComicFilesManager * comicFilesManager = new ComicFilesManager();
        comicFilesManager->copyComicsTo(comics,destFolderPath);

        processComicFiles(comicFilesManager, progressDialog);
    }
}

void LibraryWindow::moveAndImportComicsToFolder(const QList<QString> &comics, const QModelIndex &miFolder)
{
    if(miFolder.isValid())
    {
        QString destFolderPath = QDir::cleanPath(currentPath()+foldersModel->getFolderPath(miFolder));

        copyMoveIndexDestination = miFolder;

        QLOG_DEBUG() << "Moving to " << destFolderPath;

        QProgressDialog * progressDialog = newProgressDialog(tr("Moving comics..."),comics.size());

        ComicFilesManager * comicFilesManager = new ComicFilesManager();
        comicFilesManager->moveComicsTo(comics,destFolderPath);

        processComicFiles(comicFilesManager, progressDialog);
    }
}

void LibraryWindow::processComicFiles(ComicFilesManager * comicFilesManager, QProgressDialog * progressDialog)
{
    connect(comicFilesManager,SIGNAL(progress(int)), progressDialog, SLOT(setValue(int)));

    QThread * thread = NULL;

    thread = new QThread();

    comicFilesManager->moveToThread(thread);

    connect(thread, SIGNAL(started()), comicFilesManager, SLOT(process()));
    connect(comicFilesManager, SIGNAL(success()), this, SLOT(updateCopyMoveFolderDestination()));
    connect(comicFilesManager, SIGNAL(finished()), thread, SLOT(quit()));
    connect(comicFilesManager, SIGNAL(finished()), comicFilesManager, SLOT(deleteLater()));
    connect(comicFilesManager, SIGNAL(finished()), progressDialog, SLOT(close()));
    connect(comicFilesManager, SIGNAL(finished()), progressDialog, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    if(thread != NULL)
        thread->start();
}

void LibraryWindow::updateCopyMoveFolderDestination()
{
    importWidget->setUpdateLook();
    showImportingWidget();

    QString currentLibrary = selectedLibrary->currentText();
    QString path = libraries.getPath(currentLibrary);
    _lastAdded = currentLibrary;
    libraryCreator->updateFolder(QDir::cleanPath(path),QDir::cleanPath(path+"/.yacreaderlibrary"),QDir::cleanPath(currentPath()+foldersModel->getFolderPath(copyMoveIndexDestination)));
    libraryCreator->start();
}

QProgressDialog *LibraryWindow::newProgressDialog(const QString &label, int maxValue)
{
    QProgressDialog * progressDialog = new QProgressDialog(label,"Cancel",0,maxValue,this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumWidth(350);
    progressDialog->show();
    return progressDialog;
}

void LibraryWindow::reloadAfterCopyMove()
{
    if(getCurrentFolderIndex() == copyMoveIndexDestination)
        reloadCovers();
}

QModelIndex LibraryWindow::getCurrentFolderIndex()
{
    if(foldersView->selectionModel()->selectedRows().length()>0)
       return foldersView->currentIndex();
    else
        return QModelIndex();
}

void LibraryWindow::selectSubfolder(const QModelIndex &mi, int child)
{
    QModelIndex dest = foldersModel->index(child,0,mi);
    foldersView->setCurrentIndex(dest);
    updateHistory(dest);
    loadCovers(dest);
}

void LibraryWindow::checkEmptyFolder(QStringList * paths)
{
	if(paths == 0)
	{
        QStringList pathList = comicsModel->getPaths(currentPath());
		paths = &pathList;
	}

	if(paths->size()>0 && !importedCovers)
	{
		disableComicsActions(false);
	}
	else
	{
		disableComicsActions(true);
		if(paths->size()>0)
			toggleFullScreenAction->setEnabled(true);
	}
}

void LibraryWindow::reloadCovers()
{
    //comics view switch when filter/search is enabled
    if(!searchEdit->text().isEmpty())
    {
       loadCoversFromCurrentModel();
       comicsView->enableFilterMode(true);
       return;
    }

    if(foldersView->selectionModel()->selectedRows().length()>0)
        loadCovers(foldersView->currentIndex());
    else
        loadCovers(QModelIndex());
QLOG_INFO() << "reloaded covers at row : " << foldersView->currentIndex().row();
    QModelIndex mi = comicsModel->getIndexFromId(_comicIdEdited);
    if(mi.isValid())
    {
        comicsView->scrollTo(mi,QAbstractItemView::PositionAtCenter);
        comicsView->setCurrentIndex(mi);
    }
	//centerComicFlow(mi);
}

void LibraryWindow::openComic()
{
	if(!importedCovers)
	{
        ComicDB comic = comicsModel->getComic(comicsView->currentIndex());
        QString path = currentPath();
        QList<ComicDB> siblings = comicsModel->getAllComics();

		quint64 comicId = comic.id;
		//TODO generate IDS for libraries...
        quint64 libraryId = libraries.getId(selectedLibrary->currentText());
	
		//                 %1        %2      %3        NO-->%4          %5        %6        %7        %8         %9       %10
		//Invoke YACReader comicPath comicId libraryId NO-->currentPage bookmark1 bookmark2 bookmark3 brightness contrast gamma
        bool yacreaderFound = false;
#ifdef Q_OS_MAC
        QString comicIdS = QString("--comicId=") + QString("%1").arg(comicId);
        QString libraryIdS = QString("--libraryId=") + QString("%1").arg(libraryId);
        QString yacreaderPath = QDir::cleanPath(QCoreApplication::applicationDirPath()+"/../../../YACReader.app");
        if(yacreaderFound = QFileInfo(yacreaderPath).exists())
            QProcess::startDetached("open", QStringList() << "-n" << yacreaderPath << "--args" << path << comicIdS << libraryIdS ); /*<< page << bookmark1 << bookmark2 << bookmark3 << brightness << contrast << gamma*///,QStringList() << path);

#endif

#ifdef Q_OS_WIN																														  /* \"%4\" \"%5\" \"%6\" \"%7\" \"%8\" \"%9\" \"%10\" */
        yacreaderFound = QProcess::startDetached(QDir::cleanPath(QCoreApplication::applicationDirPath())+QString("/YACReader \"%1\" \"%2\" \"%3\"").arg(path).arg(QString("--comicId=") + QString::number(comicId)).arg(QString("--libraryId=") + QString::number(libraryId))/*.arg(page).arg(bookmark1).arg(bookmark2).arg(bookmark3).arg(brightness).arg(contrast).arg(gamma)*/,QStringList());
#endif

#if defined Q_OS_UNIX && !defined Q_OS_MAC
        QStringList parameters = QStringList() << path << (QString("--comicId=") + QString::number(comicId)) << (QString("--libraryId=") + QString::number(libraryId));
	yacreaderFound = QProcess::startDetached(QString("YACReader"),parameters);
#endif
        if(!yacreaderFound)
            QMessageBox::critical(this,tr("YACReader not found"),tr("YACReader not found, YACReader should be installed in the same folder as YACReaderLibrary."));

		setCurrentComicOpened();
	}
}

void LibraryWindow::setCurrentComicsStatusReaded(YACReaderComicReadStatus readStatus) {
    comicsModel->setComicsRead(getSelectedComics(),readStatus);
}

void LibraryWindow::setCurrentComicReaded() {
	this->setCurrentComicsStatusReaded(YACReader::Read);
}

void LibraryWindow::setCurrentComicOpened()
{
    //TODO: remove?
}

void LibraryWindow::setCurrentComicUnreaded() {
	this->setCurrentComicsStatusReaded(YACReader::Unread);
}

void LibraryWindow::createLibrary() {
	createLibraryDialog->show(libraries);
}

void LibraryWindow::create(QString source, QString dest, QString name)
{
    QLOG_INFO() << QString("About to create a library from '%1' to '%2' with name '%3'").arg(source).arg(dest).arg(name);
	libraryCreator->createLibrary(source,dest);
	libraryCreator->start();
	_lastAdded = name;
	_sourceLastAdded = source;

	importWidget->setImportLook();
	showImportingWidget();

}

void LibraryWindow::reloadCurrentLibrary() {
	loadLibrary(selectedLibrary->currentText());
}

void LibraryWindow::openLastCreated()
{
	
	selectedLibrary->disconnect();

	selectedLibrary->setCurrentIndex(selectedLibrary->findText(_lastAdded));
	libraries.addLibrary(_lastAdded,_sourceLastAdded);
	selectedLibrary->addItem(_lastAdded,_sourceLastAdded);
	selectedLibrary->setCurrentIndex(selectedLibrary->findText(_lastAdded));
	libraries.save();

	connect(selectedLibrary,SIGNAL(currentIndexChanged(QString)),this,SLOT(loadLibrary(QString)));
	
	loadLibrary(_lastAdded);
}

void LibraryWindow::showAddLibrary()
{
	addLibraryDialog->show();
}

void LibraryWindow::openLibrary(QString path, QString name)
{	
	if(!libraries.contains(name))
	{
		//TODO: fix bug, /a/b/c/.yacreaderlibrary/d/e
		path.remove("/.yacreaderlibrary");
		QDir d; //TODO change this by static methods (utils class?? with delTree for example) 
		if(d.exists(path + "/.yacreaderlibrary"))
		{
			_lastAdded = name;
			_sourceLastAdded = path;
			openLastCreated();
			addLibraryDialog->close();
		}
		else
			QMessageBox::warning(this,tr("Library not found"),tr("The selected folder doesn't contain any library."));
	}
	else
	{
		libraryAlreadyExists(name);
	}
}

void LibraryWindow::loadLibraries()
{
	libraries.load();
	foreach(QString name,libraries.getNames())
			selectedLibrary->addItem(name,libraries.getPath(name));
}


void LibraryWindow::saveLibraries() {
	libraries.save();
}

void LibraryWindow::updateLibrary()
{
	importWidget->setUpdateLook();
	showImportingWidget();

	QString currentLibrary = selectedLibrary->currentText();
	QString path = libraries.getPath(currentLibrary);
	_lastAdded = currentLibrary;
	libraryCreator->updateLibrary(path,path+"/.yacreaderlibrary");
	libraryCreator->start();
}

void LibraryWindow::deleteCurrentLibrary()
{
	QString path = libraries.getPath(selectedLibrary->currentText());
	libraries.remove(selectedLibrary->currentText());
	selectedLibrary->removeItem(selectedLibrary->currentIndex());
	//selectedLibrary->setCurrentIndex(0);
	path = path+"/.yacreaderlibrary";

	QDir d(path);
	delTree(d);
	d.rmdir(path);
	if(libraries.isEmpty())//no more libraries available.
	{
        comicsView->setModel(NULL);
		foldersView->setModel(NULL);

        disableAllActions();
        showNoLibrariesWidget();
	}
	libraries.save();
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
		//selectedLibrary->setCurrentIndex(0);
		if(libraries.isEmpty())//no more libraries available.
		{
            comicsView->setModel(NULL);
			foldersView->setModel(NULL);

            disableAllActions();
            showNoLibrariesWidget();
		}
		libraries.save();
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

void LibraryWindow::rename(QString newName) //TODO replace
{
	QString currentLibrary = selectedLibrary->currentText();
	if(newName != currentLibrary)
	{
		if(!libraries.contains(newName))
		{
			libraries.rename(currentLibrary,newName);
			//selectedLibrary->removeItem(selectedLibrary->currentIndex());
			//libraries.addLibrary(newName,path);
			selectedLibrary->renameCurrentLibrary(newName);
			libraries.save();
			renameLibraryDialog->close();
#ifndef Q_OS_MAC
			if(!foldersView->currentIndex().isValid())
				libraryToolBar->setCurrentFolderName(selectedLibrary->currentText());
#endif
		}
		else
		{
			libraryAlreadyExists(newName);
		}
	}
	else
		renameLibraryDialog->close();
	//selectedLibrary->setCurrentIndex(selectedLibrary->findText(newName));
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
	if(!libraries.isEmpty())
	{	
		QString path=libraries.getPath(selectedLibrary->currentText())+"/.yacreaderlibrary";
		QDir d; //TODO change this by static methods (utils class?? with delTree for example) 
		if(d.exists(path))
		{
			loadCovers(QModelIndex());
			if(history.count()>1)
				updateHistory(QModelIndex());
		}
		else
		{
            comicsView->setModel(NULL);
		}

        foldersView->selectionModel()->clear();
	}

    setFolderAsNotCompletedAction->setVisible(false);
    setFolderAsCompletedAction->setVisible(false);
    setFolderAsReadAction->setVisible(false);
    setFolderAsUnreadAction->setVisible(false);
}


void LibraryWindow::toggleFullScreen()
{
	fullscreen?toNormal():toFullScreen();
	fullscreen = !fullscreen;
}

void LibraryWindow::toFullScreen()
{
	fromMaximized = this->isMaximized();

    sideBar->hide();
	libraryToolBar->hide();

    comicsView->toFullScreen();

	showFullScreen();
}

void LibraryWindow::toNormal()
{
	sideBar->show();
	
    comicsView->toNormal();

	if(fromMaximized)
		showMaximized();
	else
		showNormal();

#ifdef Q_OS_MAC
	QTimer * timer = new QTimer();
	timer->setSingleShot(true);
	timer->start();
	connect(timer,SIGNAL(timeout()),libraryToolBar,SLOT(show()));
	connect(timer,SIGNAL(timeout()),timer,SLOT(deleteLater()));
#else
	libraryToolBar->show();
#endif

}

void LibraryWindow::setSearchFilter(QString filter)
{
    if(filter.isEmpty() && foldersModel->isFilterEnabled())
	{
        foldersModel->resetFilter();
        comicsView->enableFilterMode(false);
		//foldersView->collapseAll();
		if(index != 0)
		{
            QModelIndex mi = foldersModel->indexFromItem(index,column);
			foldersView->scrollTo(mi,QAbstractItemView::PositionAtTop);
			updateHistory(mi);
			foldersView->setCurrentIndex(mi);

		}

        reloadCovers();
	}
	else
	{
		if(!filter.isEmpty())
		{
            foldersModel->setFilter(filter, true);//includeComicsCheckBox->isChecked());
            comicsModel->setupModelData(filter, foldersModel->getDatabase());
            comicsView->enableFilterMode(true);
			foldersView->expandAll();
            //loadCoversFromCurrentModel();
		}
	}
}

void LibraryWindow::showProperties()
{
	QModelIndexList indexList = getSelectedComics();

    QList<ComicDB> comics = comicsModel->getComics(indexList);
	ComicDB c = comics[0];
	_comicIdEdited = c.id;//static_cast<TableItem*>(indexList[0].internalPointer())->data(4).toULongLong();

    propertiesDialog->databasePath = foldersModel->getDatabase();
	propertiesDialog->basePath = currentPath();
	propertiesDialog->setComics(comics);
	
	propertiesDialog->show();
}

void LibraryWindow::showComicVineScraper()
{
    QSettings s(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
    s.beginGroup("ComicVine");

    if(!s.contains(COMIC_VINE_API_KEY))
    {
        ApiKeyDialog d;
        d.exec();
    }

    //check if the api key was inserted
    if(s.contains(COMIC_VINE_API_KEY))
    {
        QModelIndexList indexList = getSelectedComics();

        QList<ComicDB> comics = comicsModel->getComics(indexList);
        ComicDB c = comics[0];
        _comicIdEdited = c.id;//static_cast<TableItem*>(indexList[0].internalPointer())->data(4).toULongLong();

        comicVineDialog->databasePath = foldersModel->getDatabase();
        comicVineDialog->basePath = currentPath();
        comicVineDialog->setComics(comics);

        comicVineDialog->show();
    }
}

void LibraryWindow::setRemoveError()
{
	removeError = true;
}

void LibraryWindow::checkRemoveError()
{
	if(removeError)
	{
		QMessageBox::critical(this,tr("Unable to delete"),tr("There was an issue trying to delete the selected comics. Please, check for write permissions in the selected files or containing folder."));
	}
    removeError = false;
}

void LibraryWindow::resetComicRating()
{
    QModelIndexList indexList = getSelectedComics();

    comicsModel->startTransaction();
    for(auto & index:indexList)
    {
        comicsModel->resetComicRating(index);
    }
    comicsModel->finishTransaction();
}

void LibraryWindow::switchToComicsView(ComicsView * from, ComicsView * to)
{
    disconnectComicsViewConnections(from);
    from->close();

    comicsView = to;
    doComicsViewConnections();
    to->setItemActions(itemActions);
    to->setViewActions(viewActions);

    comicsView->setToolBar(editInfoToolBar);

    comicsViewStack->removeWidget(from);
    comicsViewStack->addWidget(comicsView);

    delete from;

    reloadCovers();
}

void LibraryWindow::showComicsViewTransition()
{
    comicsViewStack->setCurrentWidget(comicsViewTransition);
    comicsViewTransition->startMovie();
}

void LibraryWindow::toggleComicsView_delayed()
{
    if(comicsViewStatus == Flow){
        QIcon icoViewsButton;
        icoViewsButton.addPixmap(QPixmap(":/images/main_toolbar/flow.png"), QIcon::Normal);
        toggleComicsViewAction->setIcon(icoViewsButton);
        switchToComicsView(classicComicsView, gridComicsView = new GridComicsView());
        comicsViewStatus = Grid;
    }
    else{
        QIcon icoViewsButton;
        icoViewsButton.addPixmap(QPixmap(":/images/main_toolbar/grid.png"), QIcon::Normal);
        toggleComicsViewAction->setIcon(icoViewsButton);
        switchToComicsView(gridComicsView, classicComicsView = new ClassicComicsView());
        comicsViewStatus = Flow;
    }

    settings->setValue(COMICS_VIEW_STATUS, comicsViewStatus);
}

void LibraryWindow::showComicsView()
{
    comicsViewStack->setCurrentWidget(comicsView);
}

void LibraryWindow::showEmptyFolderView()
{
    comicsViewStack->setCurrentWidget(emptyFolderWidget);
}

void LibraryWindow::showNoSearchResultsView()
{
    comicsViewStack->setCurrentWidget(noSearchResultsWidget);
}

//TODO recover the current comics selection and restore it in the destination
void LibraryWindow::toggleComicsView()
{
    if(comicsViewStack->currentWidget()!=emptyFolderWidget) {
        QTimer::singleShot(0,this,SLOT(showComicsViewTransition()));
        QTimer::singleShot(32,this,SLOT(toggleComicsView_delayed()));
    } else
        toggleComicsView_delayed();
}

void LibraryWindow::checkSearchNumResults(int numResults)
{
    if(numResults == 0)
        showNoSearchResultsView();
    else
        showComicsView();
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
    _comicIdEdited = comicsModel->asignNumbers(indexList,startingNumber);
	
	reloadCovers();
}

void LibraryWindow::openContainingFolderComic()
{
QModelIndex modelIndex = comicsView->currentIndex();
QFileInfo file = QDir::cleanPath(currentPath() + comicsModel->getComicPath(modelIndex));
#if defined Q_OS_UNIX && !defined Q_OS_MAC
	QString path = file.absolutePath();
	QDesktopServices::openUrl(QUrl("file:///"+path, QUrl::TolerantMode));
#endif

#ifdef Q_OS_MAC
	QString filePath = file.absoluteFilePath();
	QStringList args;
	args << "-e";
	args << "tell application \"Finder\"";
	args << "-e";
	args << "activate";
	args << "-e";
	args << "select POSIX file \""+filePath+"\"";
	args << "-e";
	args << "end tell";
	QProcess::startDetached("osascript", args);
#endif
	
#ifdef Q_OS_WIN
    QString filePath = file.absoluteFilePath();
    QString cmdArgs = QString("/select,\"") + QDir::toNativeSeparators(filePath) + QStringLiteral("\"");
    ShellExecuteW(0, L"open", L"explorer.exe", reinterpret_cast<LPCWSTR>(cmdArgs.utf16()), 0, SW_NORMAL);
#endif
}

void LibraryWindow::openContainingFolder()
{
	QModelIndex modelIndex = foldersView->currentIndex();
    QString path;
    if(modelIndex.isValid())
        path = QDir::cleanPath(currentPath() + foldersModel->getFolderPath(modelIndex));
    else
        path = QDir::cleanPath(currentPath());
    QDesktopServices::openUrl(QUrl("file:///"+path, QUrl::TolerantMode));
}

void LibraryWindow::setFolderAsNotCompleted()
{
    foldersModel->updateFolderCompletedStatus(foldersView->selectionModel()->selectedRows(),false);
}

void LibraryWindow::setFolderAsCompleted()
{
    foldersModel->updateFolderCompletedStatus(foldersView->selectionModel()->selectedRows(),true);
}

void LibraryWindow::setFolderAsRead()
{
    foldersModel->updateFolderFinishedStatus(foldersView->selectionModel()->selectedRows(),true);
}

void LibraryWindow::setFolderAsUnread()
{
   foldersModel->updateFolderFinishedStatus(foldersView->selectionModel()->selectedRows(),false);
}

void LibraryWindow::exportLibrary(QString destPath)
{
	QString currentLibrary = selectedLibrary->currentText();
	QString path = libraries.getPath(currentLibrary)+"/.yacreaderlibrary";
	packageManager->createPackage(path,destPath+"/"+currentLibrary);
}

void LibraryWindow::importLibrary(QString clc,QString destPath,QString name)
{
	packageManager->extractPackage(clc,destPath+"/"+name);
	_lastAdded = name;
	_sourceLastAdded = destPath+"/"+name;
}

void LibraryWindow::reloadOptions()
{
	//comicFlow->setFlowType(flowType);
    comicsView->updateConfig(settings);
}

QString LibraryWindow::currentPath()
{
    return libraries.getPath(selectedLibrary->currentText());
}

QString LibraryWindow::currentFolderPath()
{
    QString path;

    if(foldersView->selectionModel()->selectedRows().length()>0)
        path = foldersModel->getFolderPath(foldersView->currentIndex());
    else
        path = foldersModel->getFolderPath(QModelIndex());

    QLOG_DEBUG() << "current folder path : " << QDir::cleanPath(currentPath()+path);

    return QDir::cleanPath(currentPath()+path);
}

//TODO ComicsView: some actions in the comics toolbar can be relative to a certain view
//show/hide actions on show/hide widget
void LibraryWindow::hideComicFlow(bool hide)
{
    /*
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
*/
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
#include "startup.h"
extern Startup * s;
void LibraryWindow::closeEvent ( QCloseEvent * event )
{
    s->stop();
    settings->setValue(MAIN_WINDOW_GEOMETRY, saveGeometry());

    comicsView->close();

    QApplication::instance()->processEvents();
        event->accept();
        QMainWindow::closeEvent(event);
}

void LibraryWindow::showNoLibrariesWidget()
{
	disableAllActions();
    searchEdit->setDisabled(true);
	mainWidget->setCurrentIndex(1);
}

void LibraryWindow::showRootWidget()
{
	libraryToolBar->setDisabled(false);
    searchEdit->setEnabled(true);
	mainWidget->setCurrentIndex(0);
}

void LibraryWindow::showImportingWidget()
{
	disableAllActions();
	importWidget->clear();
	libraryToolBar->setDisabled(true);
    searchEdit->setDisabled(true);
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

bool lessThanModelIndexRow(const QModelIndex & m1, const QModelIndex & m2)
{
	return m1.row()<m2.row();
}

QModelIndexList LibraryWindow::getSelectedComics()
{
	//se fuerza a que haya almenos una fila seleccionada TODO comprobar se se puede forzar a la tabla a que lo haga automáticamente
    //avoid selection.count()==0 forcing selection in comicsView
    QModelIndexList selection = comicsView->selectionModel()->selectedRows();
    QLOG_INFO() << "selection count " << selection.length();
	qSort(selection.begin(),selection.end(),lessThanModelIndexRow);

    /*if(selection.count()==0)
	{
        comicsView->selectRow(comicFlow->centerIndex());
        selection = comicsView->selectionModel()->selectedRows();
    }*/
	return selection;
}

void LibraryWindow::deleteComics()
{
	int ret = QMessageBox::question(this,tr("Delete comics"),tr("All the selected comics will be deleted from your disk. Are you sure?"),QMessageBox::Yes,QMessageBox::No);

	if(ret == QMessageBox::Yes)
	{

		QModelIndexList indexList = getSelectedComics();

        QList<ComicDB> comics = comicsModel->getComics(indexList);

		QList<QString> paths;
		QString libraryPath = currentPath();
		foreach(ComicDB comic, comics)
		{
            paths.append(libraryPath + comic.path);
            QLOG_INFO() << comic.path;
            QLOG_INFO() << comic.id;
            QLOG_INFO() << comic.parentId;
		}

		ComicsRemover * remover = new ComicsRemover(indexList,paths);

        //comicsView->showDeleteProgress();
        comicsModel->startTransaction();

        connect(remover, SIGNAL(remove(int)), comicsModel, SLOT(remove(int)));
		connect(remover,SIGNAL(removeError()),this,SLOT(setRemoveError()));
        connect(remover, SIGNAL(finished()), comicsModel, SLOT(finishTransaction()));
        //connect(remover, SIGNAL(finished()), comicsView, SLOT(hideDeleteProgress()));
		connect(remover, SIGNAL(finished()),this,SLOT(checkEmptyFolder()));
		connect(remover, SIGNAL(finished()),this,SLOT(checkRemoveError()));
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

void LibraryWindow::backward()
{
	if(currentFolderNavigation>0)
	{
		currentFolderNavigation--;
		loadCovers(history.at(currentFolderNavigation));
		foldersView->setCurrentIndex(history.at(currentFolderNavigation));
		forwardAction->setEnabled(true);
	}
	if(currentFolderNavigation==0)
	{
		backAction->setEnabled(false);
	}
}

void LibraryWindow::forward()
{
	if(currentFolderNavigation<history.count()-1)
	{
		currentFolderNavigation++;
		loadCovers(history.at(currentFolderNavigation));
		foldersView->setCurrentIndex(history.at(currentFolderNavigation));
		backAction->setEnabled(true);
	}
	if(currentFolderNavigation==history.count()-1)
	{
		forwardAction->setEnabled(false);
	}
}

void LibraryWindow::updateHistory(const QModelIndex &mi)
{
	//remove history from current index
	if(!mi.isValid())
		return;
	int numElementsToRemove = history.count() - (currentFolderNavigation+1);
	while(numElementsToRemove>0)
	{
		numElementsToRemove--;
		history.removeLast();
	}

	if(mi!=history.at(currentFolderNavigation))
	{
		history.append(mi);

		backAction->setEnabled(true);
		currentFolderNavigation++;
	}

    forwardAction->setEnabled(false);
}

void LibraryWindow::updateFoldersViewConextMenu(const QModelIndex &mi)
{
    if(!mi.isValid())
        return;

    TreeItem * item = static_cast<TreeItem *>(mi.internalPointer());
    bool isFinished = item->data(TreeModel::Finished).toBool();
    bool isCompleted = item->data(TreeModel::Completed).toBool();

    setFolderAsReadAction->setVisible(!isFinished);
    setFolderAsUnreadAction->setVisible(isFinished);

    setFolderAsCompletedAction->setVisible(!isCompleted);
    setFolderAsNotCompletedAction->setVisible(isCompleted);
}

void LibraryWindow::libraryAlreadyExists(const QString & name)
{
	QMessageBox::information(this,tr("Library name already exists"),tr("There is another library with the name '%1'.").arg(name));
}

void LibraryWindow::importLibraryPackage()
{
	importLibraryDialog->show(libraries);
}

void LibraryWindow::updateComicsView(quint64 libraryId, const ComicDB & comic)
{
	//TODO comprobar la biblioteca....
    if(libraryId == selectedLibrary->currentIndex()) {
        comicsModel->reload(comic);
	}
}
