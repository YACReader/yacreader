#include "library_window.h"

#include "yacreader_global.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QDir>
#include <QHeaderView>
#include <QProcess>
#include <QtCore>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFileIconProvider>
#include <QSettings>
#include <QHeaderView>

#include <algorithm>
#include <future>

#include "folder_item.h"
#include "data_base_management.h"
#include "no_libraries_widget.h"
#include "import_widget.h"

#include "yacreader_search_line_edit.h"
#include "comic_db.h"
#include "library_creator.h"
#include "package_manager.h"
#include "xml_info_library_scanner.h"
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
#include "comic_model.h"
#include "yacreader_tool_bar_stretch.h"

#include "yacreader_titled_toolbar.h"
#include "yacreader_main_toolbar.h"

#include "yacreader_sidebar.h"

#include "comics_remover.h"
#include "yacreader_library_list_widget.h"
#include "yacreader_folders_view.h"

#include "comic_vine_dialog.h"
#include "api_key_dialog.h"
// #include "yacreader_social_dialog.h"

#include "comics_view.h"

#include "edit_shortcuts_dialog.h"
#include "shortcuts_manager.h"

#include "comic_files_manager.h"

#include "reading_list_model.h"
#include "yacreader_reading_lists_view.h"
#include "add_label_dialog.h"

#include "yacreader_history_controller.h"
#include "db_helper.h"

#include "reading_list_item.h"
#include "opengl_checker.h"

#include "yacreader_content_views_manager.h"
#include "folder_content_view.h"

#include "trayicon_controller.h"

#include "whats_new_controller.h"

#include "library_comic_opener.h"

#include "recent_visibility_coordinator.h"

#include "QsLog.h"

#include "yacreader_http_server.h"
extern YACReaderHttpServer *httpServer;

#ifdef Q_OS_WIN
#include <shellapi.h>
#endif

#include <KDSignalThrottler.h>

namespace {
template<class Remover>
void moveAndConnectRemoverToThread(Remover *remover, QThread *thread)
{
    Q_ASSERT(remover);
    Q_ASSERT(thread);
    remover->moveToThread(thread);
    QObject::connect(thread, &QThread::started, remover, &Remover::process);
    QObject::connect(remover, &Remover::finished, remover, &QObject::deleteLater);
    QObject::connect(remover, &Remover::finished, thread, &QThread::quit);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
}
}

using namespace YACReader;

LibraryWindow::LibraryWindow()
    : QMainWindow(), fullscreen(false), previousFilter(""), fetching(false), status(LibraryWindow::Normal), removeError(false)
{
    createSettings();

    setupUI();

    loadLibraries();

    if (libraries.isEmpty()) {
        showNoLibrariesWidget();
    } else {
        showRootWidget();
        selectedLibrary->setCurrentIndex(0);
    }

    afterLaunchTasks();
}

void LibraryWindow::afterLaunchTasks()
{
    if (!libraries.isEmpty()) {
        WhatsNewController whatsNewController;
        whatsNewController.showWhatsNewIfNeeded(this);
    }
}

bool LibraryWindow::eventFilter(QObject *object, QEvent *event)
{
    if (this->isActiveWindow()) {
        if (event->type() == QEvent::MouseButtonRelease) {
            auto mouseEvent = static_cast<QMouseEvent *>(event);

            if (mouseEvent->button() == Qt::ForwardButton) {
                actions.forwardAction->trigger();
                event->accept();
                return true;
            }

            if (mouseEvent->button() == Qt::BackButton) {
                actions.backAction->trigger();
                event->accept();
                return true;
            }
        }
    }

    if (this->foldersView->hasFocus() && event->type() == QEvent::Shortcut) {
        auto shortcutEvent = static_cast<QShortcutEvent *>(event);
        auto keySequence = shortcutEvent->key();

        if (keySequence.count() > 1) {
            return QMainWindow::eventFilter(object, event);
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto keyCombination = keySequence[0];

        if (keyCombination.keyboardModifiers() != Qt::NoModifier) {
            return QMainWindow::eventFilter(object, event);
        }
#endif

        auto string = keySequence.toString();

        if (string.size() > 1) {
            return QMainWindow::eventFilter(object, event);
        }

        event->ignore();

        foldersView->keyboardSearch(keySequence.toString());
        return true;
    }

    return QMainWindow::eventFilter(object, event);
}

void LibraryWindow::createSettings()
{
    settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat); // TODO unificar la creación del fichero de config con el servidor
    settings->beginGroup("libraryConfig");
}

void LibraryWindow::setupOpenglSetting()
{
#ifndef NO_OPENGL
    // FLOW-----------------------------------------------------------------------
    //---------------------------------------------------------------------------

    OpenGLChecker openGLChecker;
    bool openGLAvailable = openGLChecker.hasCompatibleOpenGLVersion();

    if (openGLAvailable && !settings->contains(USE_OPEN_GL))
        settings->setValue(USE_OPEN_GL, 2);
    else if (!openGLAvailable)
        settings->setValue(USE_OPEN_GL, 0);
#endif
}

void LibraryWindow::setupUI()
{
    setupOpenglSetting();

    setUnifiedTitleAndToolBarOnMac(true);

    libraryCreator = new LibraryCreator(settings);
    packageManager = new PackageManager();
    xmlInfoLibraryScanner = new XMLInfoLibraryScanner();

    historyController = new YACReaderHistoryController(this);

    actions.createActions(this, settings);
    doModels();

    doDialogs();
    doLayout();
    createToolBars();
    createMenus();

    setupCoordinators();

    navigationController = new YACReaderNavigationController(this, contentViewsManager);

    createConnections();

    setWindowTitle(tr("YACReader Library"));

    setMinimumSize(800, 480);

    // restore
    if (settings->contains(MAIN_WINDOW_GEOMETRY))
        restoreGeometry(settings->value(MAIN_WINDOW_GEOMETRY).toByteArray());
    else
        // if(settings->value(USE_OPEN_GL).toBool() == false)
        showMaximized();

    trayIconController = new TrayIconController(settings, this);
}

void LibraryWindow::doLayout()
{
    // LAYOUT ELEMENTS------------------------------------------------------------
    auto sHorizontal = new QSplitter(Qt::Horizontal); // spliter principal
#ifdef Y_MAC_UI
    sHorizontal->setStyleSheet("QSplitter::handle{image:none;background-color:#B8B8B8;} QSplitter::handle:vertical {height:1px;}");
#else
    sHorizontal->setStyleSheet("QSplitter::handle:vertical {height:4px;}");
#endif

    // TOOLBARS-------------------------------------------------------------------
    //---------------------------------------------------------------------------
    editInfoToolBar = new QToolBar();
    editInfoToolBar->setStyleSheet("QToolBar {border: none;}");

#ifdef Y_MAC_UI
    libraryToolBar = new YACReaderMacOSXToolbar(this);
#else
    libraryToolBar = new YACReaderMainToolBar(this);
#endif

    // FOLDERS FILTER-------------------------------------------------------------
    //---------------------------------------------------------------------------
#ifndef Y_MAC_UI
    // in MacOSX the searchEdit is created using the toolbar wrapper
    searchEdit = new YACReaderSearchLineEdit();
#endif

    // SIDEBAR--------------------------------------------------------------------
    //---------------------------------------------------------------------------
    sideBar = new YACReaderSideBar;

    foldersView = sideBar->foldersView;
    listsView = sideBar->readingListsView;
    selectedLibrary = sideBar->selectedLibrary;

    YACReaderTitledToolBar *librariesTitle = sideBar->librariesTitle;
    YACReaderTitledToolBar *foldersTitle = sideBar->foldersTitle;
    YACReaderTitledToolBar *readingListsTitle = sideBar->readingListsTitle;

    librariesTitle->addAction(actions.createLibraryAction);
    librariesTitle->addAction(actions.openLibraryAction);
    librariesTitle->addSpacing(3);

    foldersTitle->addAction(actions.addFolderAction);
    foldersTitle->addAction(actions.deleteFolderAction);
    foldersTitle->addSepartor();
    foldersTitle->addAction(actions.setRootIndexAction);
    foldersTitle->addAction(actions.expandAllNodesAction);
    foldersTitle->addAction(actions.colapseAllNodesAction);

    readingListsTitle->addAction(actions.addReadingListAction);
    // readingListsTitle->addSepartor();
    readingListsTitle->addAction(actions.addLabelAction);
    // readingListsTitle->addSepartor();
    readingListsTitle->addAction(actions.renameListAction);
    readingListsTitle->addAction(actions.deleteReadingListAction);
    readingListsTitle->addSpacing(3);

    // FINAL LAYOUT-------------------------------------------------------------

    contentViewsManager = new YACReaderContentViewsManager(settings, this);

    sHorizontal->addWidget(sideBar);
#ifndef Y_MAC_UI
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(libraryToolBar);
    rightLayout->addWidget(contentViewsManager->containerWidget());

    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    QWidget *rightWidget = new QWidget();
    rightWidget->setLayout(rightLayout);

    sHorizontal->addWidget(rightWidget);
#else
    sHorizontal->addWidget(contentViewsManager->containerWidget());
#endif

    sHorizontal->setStretchFactor(0, 0);
    sHorizontal->setStretchFactor(1, 1);
    mainWidget = new QStackedWidget(this);
    mainWidget->addWidget(sHorizontal);
    setCentralWidget(mainWidget);
    // FINAL LAYOUT-------------------------------------------------------------

    // OTHER----------------------------------------------------------------------
    //---------------------------------------------------------------------------
    noLibrariesWidget = new NoLibrariesWidget();
    mainWidget->addWidget(noLibrariesWidget);

    importWidget = new ImportWidget();
    mainWidget->addWidget(importWidget);

    connect(noLibrariesWidget, &NoLibrariesWidget::createNewLibrary, this, &LibraryWindow::createLibrary);
    connect(noLibrariesWidget, &NoLibrariesWidget::addExistingLibrary, this, &LibraryWindow::showAddLibrary);

    // collapsible disabled in macosx (only temporaly)
#ifdef Y_MAC_UI
    sHorizontal->setCollapsible(0, false);
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
    actions.setUpShortcutsManagement(editShortcutsDialog);

#ifdef SERVER_RELEASE
    serverConfigDialog = new ServerConfigDialog(this);
#endif

    had = new HelpAboutDialog(this); // TODO load data.
    QString sufix = QLocale::system().name();
    if (QFile(":/files/about_" + sufix + ".html").exists())
        had->loadAboutInformation(":/files/about_" + sufix + ".html");
    else
        had->loadAboutInformation(":/files/about.html");

    if (QFile(":/files/helpYACReaderLibrary_" + sufix + ".html").exists())
        had->loadHelp(":/files/helpYACReaderLibrary_" + sufix + ".html");
    else
        had->loadHelp(":/files/helpYACReaderLibrary.html");
}

void LibraryWindow::doModels()
{
    // folders
    foldersModel = new FolderModel(this);
    foldersModelProxy = new FolderModelProxy(this);
    folderQueryResultProcessor.reset(new FolderQueryResultProcessor(foldersModel));
    // foldersModelProxy->setSourceModel(foldersModel);
    // comics
    comicsModel = new ComicModel(this);
    // lists
    listsModel = new ReadingListModel(this);
    listsModelProxy = new ReadingListModelProxy(this);
}

void LibraryWindow::setupCoordinators()
{
    recentVisibilityCoordinator = new RecentVisibilityCoordinator(settings, foldersModel, contentViewsManager->folderContentView, comicsModel);

    auto canStartUpdateProvider = [this]() {
        return comicVineDialog->isVisible() == false &&
                propertiesDialog->isVisible() == false;
    };
    librariesUpdateCoordinator = new LibrariesUpdateCoordinator(settings, libraries, canStartUpdateProvider, this);

    connect(librariesUpdateCoordinator, &LibrariesUpdateCoordinator::updateStarted, sideBar->librariesTitle, &YACReaderTitledToolBar::showBusyIndicator);
    connect(librariesUpdateCoordinator, &LibrariesUpdateCoordinator::updateEnded, sideBar->librariesTitle, &YACReaderTitledToolBar::hideBusyIndicator);

    connect(librariesUpdateCoordinator, &LibrariesUpdateCoordinator::updateStarted, this, [=]() {
        actions.disableAllActions();
    });
    connect(librariesUpdateCoordinator, &LibrariesUpdateCoordinator::updateEnded, this, &LibraryWindow::reloadCurrentLibrary);

    librariesUpdateCoordinator->init();

    connect(sideBar->librariesTitle, &YACReaderTitledToolBar::cancelOperationRequested, librariesUpdateCoordinator, &LibrariesUpdateCoordinator::cancel);
}

void LibraryWindow::createToolBars()
{

#ifdef Y_MAC_UI
    // libraryToolBar->setIconSize(QSize(16,16)); //TODO make icon size dynamic

    libraryToolBar->addAction(actions.backAction);
    libraryToolBar->addAction(actions.forwardAction);

    libraryToolBar->addSpace(10);

#ifdef SERVER_RELEASE
    libraryToolBar->addAction(actions.serverConfigAction);
#endif
    libraryToolBar->addAction(actions.optionsAction);
    libraryToolBar->addAction(actions.helpAboutAction);

    libraryToolBar->addSpace(10);

    libraryToolBar->addAction(actions.toggleComicsViewAction);

    libraryToolBar->addStretch();

    // Native toolbar search edit
    // libraryToolBar->addWidget(searchEdit);
    searchEdit = libraryToolBar->addSearchEdit();
    // connect(libraryToolBar,SIGNAL(searchTextChanged(YACReader::SearchModifiers,QString)),this,SLOT(setSearchFilter(YACReader::SearchModifiers, QString)));

    // libraryToolBar->setMovable(false);

    libraryToolBar->attachToWindow(this);

#else
    libraryToolBar->backButton->setDefaultAction(actions.backAction);
    libraryToolBar->forwardButton->setDefaultAction(actions.forwardAction);
    libraryToolBar->settingsButton->setDefaultAction(actions.optionsAction);
    libraryToolBar->serverButton->setDefaultAction(actions.serverConfigAction);
    libraryToolBar->helpButton->setDefaultAction(actions.helpAboutAction);
    libraryToolBar->toggleComicsViewButton->setDefaultAction(actions.toggleComicsViewAction);
#ifndef Q_OS_MACOS
    libraryToolBar->fullscreenButton->setDefaultAction(actions.toggleFullScreenAction);
#endif
    libraryToolBar->setSearchWidget(searchEdit);
#endif

    editInfoToolBar->setIconSize(QSize(18, 18));
    editInfoToolBar->addAction(actions.openComicAction);
    editInfoToolBar->addSeparator();
    editInfoToolBar->addAction(actions.editSelectedComicsAction);
    editInfoToolBar->addAction(actions.getInfoAction);
    editInfoToolBar->addAction(actions.asignOrderAction);

    editInfoToolBar->addSeparator();

    editInfoToolBar->addAction(actions.selectAllComicsAction);

    editInfoToolBar->addSeparator();

    editInfoToolBar->addAction(actions.setAsReadAction);
    editInfoToolBar->addAction(actions.setAsNonReadAction);

    editInfoToolBar->addAction(actions.showHideMarksAction);

    editInfoToolBar->addSeparator();

    auto setTypeToolButton = new QToolButton();
    setTypeToolButton->addAction(actions.setNormalAction);
    setTypeToolButton->addAction(actions.setMangaAction);
    setTypeToolButton->addAction(actions.setWesternMangaAction);
    setTypeToolButton->addAction(actions.setWebComicAction);
    setTypeToolButton->addAction(actions.setYonkomaAction);
    setTypeToolButton->setPopupMode(QToolButton::InstantPopup);
    setTypeToolButton->setDefaultAction(actions.setNormalAction);
    editInfoToolBar->addWidget(setTypeToolButton);

    editInfoToolBar->addSeparator();

    editInfoToolBar->addAction(actions.deleteComicsAction);

    auto toolBarStretch = new YACReaderToolBarStretch(this);
    editInfoToolBar->addWidget(toolBarStretch);

    editInfoToolBar->addAction(actions.toogleShowRecentIndicatorAction);

    contentViewsManager->comicsView->setToolBar(editInfoToolBar);
}

void LibraryWindow::createMenus()
{
    foldersView->addAction(actions.addFolderAction);
    foldersView->addAction(actions.deleteFolderAction);
    YACReader::addSperator(foldersView);

    foldersView->addAction(actions.openContainingFolderAction);
    foldersView->addAction(actions.updateFolderAction);
    YACReader::addSperator(foldersView);

    foldersView->addAction(actions.setFolderAsNotCompletedAction);
    foldersView->addAction(actions.setFolderAsCompletedAction);
    YACReader::addSperator(foldersView);

    foldersView->addAction(actions.setFolderAsReadAction);
    foldersView->addAction(actions.setFolderAsUnreadAction);
    YACReader::addSperator(foldersView);

    foldersView->addAction(actions.setFolderAsNormalAction);
    foldersView->addAction(actions.setFolderAsMangaAction);
    foldersView->addAction(actions.setFolderAsWesternMangaAction);
    foldersView->addAction(actions.setFolderAsWebComicAction);
    foldersView->addAction(actions.setFolderAsYonkomaAction);

    selectedLibrary->addAction(actions.updateLibraryAction);
    selectedLibrary->addAction(actions.renameLibraryAction);
    selectedLibrary->addAction(actions.removeLibraryAction);
    YACReader::addSperator(selectedLibrary);

    auto setNormalAction = new QAction();
    setNormalAction->setText(tr("comic"));

    auto setMangaAction = new QAction();
    setMangaAction->setText(tr("manga"));

    auto setWesternMangaAction = new QAction();
    setWesternMangaAction->setText(tr("western manga (left to right)"));

    auto setWebComicAction = new QAction();
    setWebComicAction->setText(tr("web comic"));

    auto setYonkomaAction = new QAction();
    setYonkomaAction->setText(tr("4koma (top to botom)"));

    setNormalAction->setCheckable(true);
    setMangaAction->setCheckable(true);
    setWesternMangaAction->setCheckable(true);
    setWebComicAction->setCheckable(true);
    setYonkomaAction->setCheckable(true);

    auto setupActions = [=](FileType type) {
        setNormalAction->setChecked(false);
        setMangaAction->setChecked(false);
        setWesternMangaAction->setChecked(false);
        setWebComicAction->setChecked(false);
        setYonkomaAction->setChecked(false);

        switch (type) {
        case YACReader::FileType::Comic:
            setNormalAction->setChecked(true);
            break;
        case YACReader::FileType::Manga:
            setMangaAction->setChecked(true);
            break;
        case YACReader::FileType::WesternManga:
            setWesternMangaAction->setChecked(true);
            break;
        case YACReader::FileType::WebComic:
            setWebComicAction->setChecked(true);
            break;
        case YACReader::FileType::Yonkoma:
            setYonkomaAction->setChecked(true);
            break;
        }
    };

    connect(setNormalAction, &QAction::triggered, this, [=]() { setCurrentLibraryAs(FileType::Comic); });
    connect(setMangaAction, &QAction::triggered, this, [=]() { setCurrentLibraryAs(FileType::Manga); });
    connect(setWesternMangaAction, &QAction::triggered, this, [=]() { setCurrentLibraryAs(FileType::WesternManga); });
    connect(setWebComicAction, &QAction::triggered, this, [=]() { setCurrentLibraryAs(FileType::WebComic); });
    connect(setYonkomaAction, &QAction::triggered, this, [=]() { setCurrentLibraryAs(FileType::Yonkoma); });

    auto typeMenu = new QMenu(tr("Set type"), selectedLibrary);

    connect(typeMenu, &QMenu::aboutToShow, this, [=]() {
        auto rootIndex = foldersModel->index(0, 0);
        auto folder = foldersModel->getFolder(rootIndex);
        setupActions(folder.type);
    });

    selectedLibrary->addAction(typeMenu->menuAction());
    YACReader::addSperator(selectedLibrary);
    typeMenu->addAction(setNormalAction);
    typeMenu->addAction(setMangaAction);
    typeMenu->addAction(setWesternMangaAction);
    typeMenu->addAction(setWebComicAction);
    typeMenu->addAction(setYonkomaAction);

    selectedLibrary->addAction(actions.rescanLibraryForXMLInfoAction);
    YACReader::addSperator(selectedLibrary);

    selectedLibrary->addAction(actions.exportComicsInfoAction);
    selectedLibrary->addAction(actions.importComicsInfoAction);
    YACReader::addSperator(selectedLibrary);

    selectedLibrary->addAction(actions.exportLibraryAction);
    selectedLibrary->addAction(actions.importLibraryAction);
    YACReader::addSperator(selectedLibrary);

    selectedLibrary->addAction(actions.showLibraryInfo);

// MacOSX app menus
#ifdef Q_OS_MACOS
    QMenuBar *menu = this->menuBar();
    // about / preferences
    // TODO

    // library
    QMenu *libraryMenu = new QMenu(tr("Library"));

    libraryMenu->addAction(actions.updateLibraryAction);
    libraryMenu->addAction(actions.renameLibraryAction);
    libraryMenu->addAction(actions.removeLibraryAction);
    libraryMenu->addSeparator();

    libraryMenu->addMenu(typeMenu);
    libraryMenu->addSeparator();

    libraryMenu->addAction(actions.rescanLibraryForXMLInfoAction);
    libraryMenu->addSeparator();

    libraryMenu->addAction(actions.exportComicsInfoAction);
    libraryMenu->addAction(actions.importComicsInfoAction);

    libraryMenu->addSeparator();

    libraryMenu->addAction(actions.exportLibraryAction);
    libraryMenu->addAction(actions.importLibraryAction);

    libraryMenu->addSeparator();

    libraryMenu->addAction(actions.showLibraryInfo);

    // folder
    QMenu *folderMenu = new QMenu(tr("Folder"));
    folderMenu->addAction(actions.openContainingFolderAction);
    folderMenu->addAction(actions.updateFolderAction);
    folderMenu->addSeparator();
    folderMenu->addAction(actions.rescanXMLFromCurrentFolderAction);
    folderMenu->addSeparator();
    folderMenu->addAction(actions.setFolderAsNotCompletedAction);
    folderMenu->addAction(actions.setFolderAsCompletedAction);
    folderMenu->addSeparator();
    folderMenu->addAction(actions.setFolderAsReadAction);
    folderMenu->addAction(actions.setFolderAsUnreadAction);
    folderMenu->addSeparator();
    foldersView->addAction(actions.setFolderAsNormalAction);
    foldersView->addAction(actions.setFolderAsMangaAction);
    foldersView->addAction(actions.setFolderAsWesternMangaAction);
    foldersView->addAction(actions.setFolderAsWebComicAction);
    foldersView->addAction(actions.setFolderAsYonkomaAction);

    // comic
    QMenu *comicMenu = new QMenu(tr("Comic"));
    comicMenu->addAction(actions.openContainingFolderComicAction);
    comicMenu->addSeparator();
    comicMenu->addAction(actions.resetComicRatingAction);

    menu->addMenu(libraryMenu);
    menu->addMenu(folderMenu);
    menu->addMenu(comicMenu);
#endif
}

void LibraryWindow::createConnections()
{
    actions.createConnections(
            historyController,
            this,
            had,
            exportLibraryDialog,
            contentViewsManager,
            editShortcutsDialog,
            foldersView,
            optionsDialog,
            serverConfigDialog,
            recentVisibilityCoordinator);
    QObject::connect(actions.focusSearchLineAction, &QAction::triggered, searchEdit, [this] { searchEdit->setFocus(Qt::ShortcutFocusReason); });

    // libraryCreator connections
    connect(createLibraryDialog, &CreateLibraryDialog::createLibrary, this, QOverload<QString, QString, QString>::of(&LibraryWindow::create));
    connect(createLibraryDialog, &CreateLibraryDialog::libraryExists, this, &LibraryWindow::libraryAlreadyExists);
    connect(importComicsInfoDialog, &QDialog::finished, this, &LibraryWindow::reloadCurrentLibrary);

    connect(libraryCreator, &LibraryCreator::finished, this, &LibraryWindow::showRootWidget);
    connect(libraryCreator, &LibraryCreator::updated, this, &LibraryWindow::reloadCurrentLibrary);
    connect(libraryCreator, &LibraryCreator::created, this, &LibraryWindow::openLastCreated);
    connect(libraryCreator, &LibraryCreator::updatedCurrentFolder, this, &LibraryWindow::reloadAfterCopyMove);
    connect(libraryCreator, &LibraryCreator::comicAdded, importWidget, &ImportWidget::newComic);
    // libraryCreator errors
    connect(libraryCreator, &LibraryCreator::failedCreatingDB, this, &LibraryWindow::manageCreatingError);
    // connect(libraryCreator, SIGNAL(failedUpdatingDB(QString)), this, SLOT(manageUpdatingError(QString))); // TODO: implement failedUpdatingDB

    connect(xmlInfoLibraryScanner, &QThread::finished, this, &LibraryWindow::showRootWidget);
    connect(xmlInfoLibraryScanner, &QThread::finished, this, &LibraryWindow::reloadCurrentFolderComicsContent);
    connect(xmlInfoLibraryScanner, &XMLInfoLibraryScanner::comicScanned, importWidget, &ImportWidget::newComic);

    // new import widget
    connect(importWidget, &ImportWidget::stop, this, &LibraryWindow::stopLibraryCreator);
    connect(importWidget, &ImportWidget::stop, this, &LibraryWindow::stopXMLScanning);

    // packageManager connections
    connect(exportLibraryDialog, &ExportLibraryDialog::exportPath, this, &LibraryWindow::exportLibrary);
    connect(exportLibraryDialog, &QDialog::rejected, packageManager, &PackageManager::cancel);
    connect(packageManager, &PackageManager::exported, exportLibraryDialog, &ExportLibraryDialog::close);
    connect(importLibraryDialog, &ImportLibraryDialog::unpackCLC, this, &LibraryWindow::importLibrary);
    connect(importLibraryDialog, &QDialog::rejected, packageManager, &PackageManager::cancel);
    connect(importLibraryDialog, &QDialog::rejected, this, &LibraryWindow::deleteCurrentLibrary);
    connect(importLibraryDialog, &ImportLibraryDialog::libraryExists, this, &LibraryWindow::libraryAlreadyExists);
    connect(packageManager, &PackageManager::imported, importLibraryDialog, &QWidget::hide);
    connect(packageManager, &PackageManager::imported, this, &LibraryWindow::openLastCreated);

    // create and update dialogs
    connect(createLibraryDialog, &CreateLibraryDialog::cancelCreate, this, &LibraryWindow::cancelCreating);

    // open existing library from dialog.
    connect(addLibraryDialog, &AddLibraryDialog::addLibrary, this, &LibraryWindow::openLibrary);

    // load library when selected library changes
    connect(selectedLibrary, &YACReaderLibraryListWidget::currentIndexChanged, this, &LibraryWindow::loadLibrary);

    // rename library dialog
    connect(renameLibraryDialog, &RenameLibraryDialog::renameLibrary, this, &LibraryWindow::rename);

    // navigations between view modes (tree,list and flow)
    // TODO connect(foldersView, SIGNAL(pressed(QModelIndex)), this, SLOT(updateFoldersViewConextMenu(QModelIndex)));
    // connect(foldersView, SIGNAL(clicked(QModelIndex)), this, SLOT(loadCovers(QModelIndex)));

    // drops in folders view
    connect(foldersView, QOverload<QList<QPair<QString, QString>>, QModelIndex>::of(&YACReaderFoldersView::copyComicsToFolder),
            this, &LibraryWindow::copyAndImportComicsToFolder);
    connect(foldersView, QOverload<QList<QPair<QString, QString>>, QModelIndex>::of(&YACReaderFoldersView::moveComicsToFolder),
            this, &LibraryWindow::moveAndImportComicsToFolder);
    connect(foldersView, &QWidget::customContextMenuRequested, this, &LibraryWindow::showFoldersContextMenu);

    // properties & config
    connect(propertiesDialog, &QDialog::accepted, contentViewsManager, &YACReaderContentViewsManager::updateCurrentContentView);
    connect(propertiesDialog, &PropertiesDialog::coverChangedSignal, this, [=](const ComicDB &comic) {
        comicsModel->notifyCoverChange(comic);
    });

    // comic vine
    connect(comicVineDialog, &QDialog::accepted, contentViewsManager, &YACReaderContentViewsManager::updateCurrentContentView, Qt::QueuedConnection);

    connect(optionsDialog, &YACReaderOptionsDialog::optionsChanged, this, &LibraryWindow::reloadOptions);
    connect(optionsDialog, &YACReaderOptionsDialog::editShortcuts, editShortcutsDialog, &QWidget::show);

    auto searchDebouncer = new KDToolBox::KDSignalDebouncer(this);
    searchDebouncer->setTimeout(400);

// Search filter
#ifdef Y_MAC_UI
    connect(searchEdit, &YACReaderMacOSXSearchLineEdit::filterChanged, searchDebouncer, &KDToolBox::KDSignalThrottler::throttle);
    connect(searchDebouncer, &KDToolBox::KDSignalThrottler::triggered, this, [=] {
        setSearchFilter(searchEdit->text());
    });
#else
    connect(searchEdit, &YACReaderSearchLineEdit::filterChanged, searchDebouncer, &KDToolBox::KDSignalThrottler::throttle);
    connect(searchDebouncer, &KDToolBox::KDSignalThrottler::triggered, this, [=] {
        setSearchFilter(searchEdit->text());
    });
#endif
    connect(&comicQueryResultProcessor, &ComicQueryResultProcessor::newData, this, &LibraryWindow::setComicSearchFilterData);
    qRegisterMetaType<FolderItem *>("FolderItem *");
    qRegisterMetaType<QMap<unsigned long long int, FolderItem *> *>("QMap<unsigned long long int, FolderItem *> *");
    connect(folderQueryResultProcessor.get(), &FolderQueryResultProcessor::newData, this, &LibraryWindow::setFolderSearchFilterData);

    connect(listsModel, &ReadingListModel::addComicsToFavorites, comicsModel, QOverload<const QList<qulonglong> &>::of(&ComicModel::addComicsToFavorites));
    connect(listsModel, &ReadingListModel::addComicsToLabel, comicsModel, QOverload<const QList<qulonglong> &, qulonglong>::of(&ComicModel::addComicsToLabel));
    connect(listsModel, &ReadingListModel::addComicsToReadingList, comicsModel, QOverload<const QList<qulonglong> &, qulonglong>::of(&ComicModel::addComicsToReadingList));
    //--

    // upgrade library
    connect(this, &LibraryWindow::libraryUpgraded, this, &LibraryWindow::loadLibrary, Qt::QueuedConnection);
    connect(this, &LibraryWindow::errorUpgradingLibrary, this, &LibraryWindow::showErrorUpgradingLibrary, Qt::QueuedConnection);
}

void LibraryWindow::showErrorUpgradingLibrary(const QString &path)
{
    QMessageBox::critical(this, tr("Upgrade failed"), tr("There were errors during library upgrade in: ") + path + "/library.ydb");
}

void LibraryWindow::setCurrentLibraryAs(FileType fileType)
{
    foldersModel->updateTreeType(fileType);
}

void LibraryWindow::loadLibrary(const QString &name)
{
    if (!libraries.isEmpty()) // si hay bibliotecas...
    {
        historyController->clear();

        showRootWidget();
        QString path = libraries.getPath(name) + "/.yacreaderlibrary";
        QDir d; // TODO change this by static methods (utils class?? with delTree for example)
        QString dbVersion;
        if (d.exists(path) && d.exists(path + "/library.ydb") && (dbVersion = DataBaseManagement::checkValidDB(path + "/library.ydb")) != "") // si existe en disco la biblioteca seleccionada, y es válida..
        {
            int comparation = DataBaseManagement::compareVersions(dbVersion, DB_VERSION);

            if (comparation < 0) {
                int ret = QMessageBox::question(this, tr("Update needed"), tr("This library was created with a previous version of YACReaderLibrary. It needs to be updated. Update now?"), QMessageBox::Yes, QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    importWidget->setUpgradeLook();
                    showImportingWidget();

                    upgradeLibraryFuture = std::async(std::launch::async, [this, name, path] {
                        bool updated = DataBaseManagement::updateToCurrentVersion(path);

                        if (!updated)
                            emit errorUpgradingLibrary(path);

                        emit libraryUpgraded(name);
                    });

                    return;
                } else {
                    contentViewsManager->comicsView->setModel(NULL);
                    foldersView->setModel(NULL);
                    listsView->setModel(NULL);
                    actions.disableAllActions(); // TODO comprobar que se deben deshabilitar
                    // será possible renombrar y borrar estas bibliotecas
                    actions.renameLibraryAction->setEnabled(true);
                    actions.removeLibraryAction->setEnabled(true);
                }
            }

            if (comparation == 0) // en caso de que la versión se igual que la actual
            {
                foldersModel->setupModelData(path);
                foldersModelProxy->setSourceModel(foldersModel);
                foldersView->setModel(foldersModelProxy);
                foldersView->setCurrentIndex(QModelIndex()); // why is this necesary?? by default it seems that returns an arbitrary index.

                listsModel->setupReadingListsData(path);
                listsModelProxy->setSourceModel(listsModel);
                listsView->setModel(listsModelProxy);

                if (foldersModel->rowCount(QModelIndex()) > 0)
                    actions.disableFoldersActions(false);
                else
                    actions.disableFoldersActions(true);

                d.setCurrent(libraries.getPath(name));
                d.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
                if (d.count() <= 1) // read only library
                {
                    actions.disableLibrariesActions(false);
                    actions.updateLibraryAction->setDisabled(true);
                    actions.openContainingFolderAction->setDisabled(true);
                    actions.rescanLibraryForXMLInfoAction->setDisabled(true);

                    disableComicsActions(true);
#ifndef Q_OS_MACOS
                    actions.toggleFullScreenAction->setEnabled(true);
#endif

                    importedCovers = true;
                } else // librería normal abierta
                {
                    actions.disableLibrariesActions(false);
                    importedCovers = false;
                }

                setRootIndex();

                searchEdit->clear();
            } else if (comparation > 0) {
                int ret = QMessageBox::question(this, tr("Download new version"), tr("This library was created with a newer version of YACReaderLibrary. Download the new version now?"), QMessageBox::Yes, QMessageBox::No);
                if (ret == QMessageBox::Yes)
                    QDesktopServices::openUrl(QUrl("http://www.yacreader.com"));

                contentViewsManager->comicsView->setModel(NULL);
                foldersView->setModel(NULL);
                listsView->setModel(NULL);
                actions.disableAllActions(); // TODO comprobar que se deben deshabilitar
                // será possible renombrar y borrar estas bibliotecas
                actions.renameLibraryAction->setEnabled(true);
                actions.removeLibraryAction->setEnabled(true);
            }
        } else {
            contentViewsManager->comicsView->setModel(NULL);
            foldersView->setModel(NULL);
            listsView->setModel(NULL);
            actions.disableAllActions(); // TODO comprobar que se deben deshabilitar

            // si la librería no existe en disco, se ofrece al usuario la posibiliad de eliminarla
            if (!d.exists(path)) {
                QString currentLibrary = selectedLibrary->currentText() + " -> " + libraries.getPath(name);
                if (QMessageBox::question(this, tr("Library not available"), tr("Library '%1' is no longer available. Do you want to remove it?").arg(currentLibrary), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
                    deleteCurrentLibrary();
                }
                // será possible renombrar y borrar estas bibliotecas
                actions.renameLibraryAction->setEnabled(true);
                actions.removeLibraryAction->setEnabled(true);

            } else // si existe el path, puede ser que la librería sea alguna versión pre-5.0 ó que esté corrupta o que no haya drivers sql
            {

                if (d.exists(path + "/library.ydb")) {
                    QSqlDatabase db = DataBaseManagement::loadDatabase(path);
                    manageOpeningLibraryError(db.lastError().databaseText() + "-" + db.lastError().driverText());
                    // será possible renombrar y borrar estas bibliotecas
                    actions.renameLibraryAction->setEnabled(true);
                    actions.removeLibraryAction->setEnabled(true);
                } else {
                    QString currentLibrary = selectedLibrary->currentText();
                    QString path = libraries.getPath(selectedLibrary->currentText());
                    if (QMessageBox::question(this, tr("Old library"), tr("Library '%1' has been created with an older version of YACReaderLibrary. It must be created again. Do you want to create the library now?").arg(currentLibrary), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
                        QDir d(path + "/.yacreaderlibrary");
                        d.removeRecursively();
                        // d.rmdir(path+"/.yacreaderlibrary");
                        createLibraryDialog->setDataAndStart(currentLibrary, path);
                        // create(path,path+"/.yacreaderlibrary",currentLibrary);
                    }
                    // será possible renombrar y borrar estas bibliotecas
                    actions.renameLibraryAction->setEnabled(true);
                    actions.removeLibraryAction->setEnabled(true);
                }
            }
        }
    } else // en caso de que no exista ninguna biblioteca se desactivan los botones pertinentes
    {
        actions.disableAllActions();
        showNoLibrariesWidget();
    }
}

void LibraryWindow::loadCoversFromCurrentModel()
{
    contentViewsManager->comicsView->setModel(comicsModel);
}

void LibraryWindow::copyAndImportComicsToCurrentFolder(const QList<QPair<QString, QString>> &comics)
{
    QLOG_DEBUG() << "-copyAndImportComicsToCurrentFolder-";
    if (comics.size() > 0) {
        QString destFolderPath = currentFolderPath();

        QModelIndex folderDestination = getCurrentFolderIndex();

        QProgressDialog *progressDialog = newProgressDialog(tr("Copying comics..."), comics.size());

        auto comicFilesManager = new ComicFilesManager();
        comicFilesManager->copyComicsTo(comics, destFolderPath, folderDestination);

        processComicFiles(comicFilesManager, progressDialog);
    }
}

void LibraryWindow::moveAndImportComicsToCurrentFolder(const QList<QPair<QString, QString>> &comics)
{
    QLOG_DEBUG() << "-moveAndImportComicsToCurrentFolder-";
    if (comics.size() > 0) {
        QString destFolderPath = currentFolderPath();

        QModelIndex folderDestination = getCurrentFolderIndex();

        QProgressDialog *progressDialog = newProgressDialog(tr("Moving comics..."), comics.size());

        auto comicFilesManager = new ComicFilesManager();
        comicFilesManager->moveComicsTo(comics, destFolderPath, folderDestination);

        processComicFiles(comicFilesManager, progressDialog);
    }
}

void LibraryWindow::copyAndImportComicsToFolder(const QList<QPair<QString, QString>> &comics, const QModelIndex &miFolder)
{
    QLOG_DEBUG() << "-copyAndImportComicsToFolder-";
    if (comics.size() > 0) {
        QModelIndex folderDestination = foldersModelProxy->mapToSource(miFolder);

        QString destFolderPath = QDir::cleanPath(currentPath() + foldersModel->getFolderPath(folderDestination));

        QLOG_DEBUG() << "Coping to " << destFolderPath;

        QProgressDialog *progressDialog = newProgressDialog(tr("Copying comics..."), comics.size());

        auto comicFilesManager = new ComicFilesManager();
        comicFilesManager->copyComicsTo(comics, destFolderPath, folderDestination);

        processComicFiles(comicFilesManager, progressDialog);
    }
}

void LibraryWindow::moveAndImportComicsToFolder(const QList<QPair<QString, QString>> &comics, const QModelIndex &miFolder)
{
    QLOG_DEBUG() << "-moveAndImportComicsToFolder-";
    if (comics.size() > 0) {
        QModelIndex folderDestination = foldersModelProxy->mapToSource(miFolder);

        QString destFolderPath = QDir::cleanPath(currentPath() + foldersModel->getFolderPath(folderDestination));

        QLOG_DEBUG() << "Moving to " << destFolderPath;

        QProgressDialog *progressDialog = newProgressDialog(tr("Moving comics..."), comics.size());

        auto comicFilesManager = new ComicFilesManager();
        comicFilesManager->moveComicsTo(comics, destFolderPath, folderDestination);

        processComicFiles(comicFilesManager, progressDialog);
    }
}

void LibraryWindow::processComicFiles(ComicFilesManager *comicFilesManager, QProgressDialog *progressDialog)
{
    connect(comicFilesManager, &ComicFilesManager::progress, progressDialog, &QProgressDialog::setValue);

    QThread *thread = NULL;

    thread = new QThread();

    comicFilesManager->moveToThread(thread);

    connect(progressDialog, &QProgressDialog::canceled, comicFilesManager, &ComicFilesManager::cancel, Qt::DirectConnection);

    connect(thread, &QThread::started, comicFilesManager, &ComicFilesManager::process);
    connect(comicFilesManager, &ComicFilesManager::success, this, &LibraryWindow::updateCopyMoveFolderDestination);
    connect(comicFilesManager, &ComicFilesManager::finished, thread, &QThread::quit);
    connect(comicFilesManager, &ComicFilesManager::finished, comicFilesManager, &QObject::deleteLater);
    connect(comicFilesManager, &ComicFilesManager::finished, progressDialog, &QWidget::close);
    connect(comicFilesManager, &ComicFilesManager::finished, progressDialog, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    if (thread != NULL)
        thread->start();
}

void LibraryWindow::updateCopyMoveFolderDestination(const QModelIndex &mi)
{
    updateFolder(mi);
}

void LibraryWindow::updateCurrentFolder()
{
    updateFolder(getCurrentFolderIndex());
}

void LibraryWindow::updateFolder(const QModelIndex &miFolder)
{
    QLOG_DEBUG() << "UPDATE FOLDER!!!!";

    importWidget->setUpdateLook();
    showImportingWidget();

    QString currentLibrary = selectedLibrary->currentText();
    QString path = libraries.getPath(currentLibrary);
    _lastAdded = currentLibrary;
    libraryCreator->updateFolder(QDir::cleanPath(path), QDir::cleanPath(path + "/.yacreaderlibrary"), QDir::cleanPath(currentPath() + foldersModel->getFolderPath(miFolder)), miFolder);
    libraryCreator->start();
}

QProgressDialog *LibraryWindow::newProgressDialog(const QString &label, int maxValue)
{
    QProgressDialog *progressDialog = new QProgressDialog(label, "Cancel", 0, maxValue, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumWidth(350);
    progressDialog->show();
    return progressDialog;
}

void LibraryWindow::reloadCurrentFolderComicsContent()
{
    navigationController->loadFolderInfo(getCurrentFolderIndex());

    enableNeededActions();
}

void LibraryWindow::reloadAfterCopyMove(const QModelIndex &mi)
{
    if (getCurrentFolderIndex() == mi) {
        auto item = static_cast<FolderItem *>(mi.internalPointer());

        if (item == nullptr) {
            foldersModel->reload();
        } else {
            foldersModel->reload(mi);
        }

        contentViewsManager->updateCurrentContentView();
    }

    enableNeededActions();
}

QModelIndex LibraryWindow::getCurrentFolderIndex()
{
    if (foldersView->selectionModel()->selectedRows().length() > 0)
        return foldersModelProxy->mapToSource(foldersView->currentIndex());
    else
        return QModelIndex();
}

void LibraryWindow::enableNeededActions()
{
    if (foldersModel->rowCount(QModelIndex()) > 0)
        actions.disableFoldersActions(false);

    if (comicsModel->rowCount() > 0)
        disableComicsActions(false);

    actions.disableLibrariesActions(false);
}

void LibraryWindow::disableComicsActions(bool disabled)
{
    if (!disabled && librariesUpdateCoordinator->isRunning()) {
        disableComicsActions(true);
        return;
    }

    actions.disableComicsActions(disabled);
}

void LibraryWindow::addFolderToCurrentIndex()
{
    exitSearchMode(); // Creating a folder in search mode is broken => exit it.

    QModelIndex currentIndex = getCurrentFolderIndex();

    bool ok;
    QString newFolderName = QInputDialog::getText(this, tr("Add new folder"),
                                                  tr("Folder name:"), QLineEdit::Normal,
                                                  "", &ok);

    // chars not supported in a folder's name: / \ : * ? " < > |
    QRegularExpression invalidChars("\\/\\:\\*\\?\\\"\\<\\>\\|\\\\"); // TODO this regexp is not properly written
    bool isValid = !newFolderName.contains(invalidChars);

    if (ok && !newFolderName.isEmpty() && isValid) {
        QString parentPath = QDir::cleanPath(currentPath() + foldersModel->getFolderPath(currentIndex));
        QDir parentDir(parentPath);
        QDir newFolder(parentPath + "/" + newFolderName);
        if (parentDir.mkdir(newFolderName) || newFolder.exists()) {
            QModelIndex newIndex = foldersModel->addFolderAtParent(newFolderName, currentIndex);
            foldersView->setCurrentIndex(foldersModelProxy->mapFromSource(newIndex));
            navigationController->loadFolderInfo(newIndex);
            historyController->updateHistory(YACReaderLibrarySourceContainer(newIndex, YACReaderLibrarySourceContainer::Folder));
            // a new folder is always an empty folder
            contentViewsManager->showFolderContentView();
        }
    }
}

void LibraryWindow::deleteSelectedFolder()
{
    QModelIndex currentIndex = getCurrentFolderIndex();
    QString relativePath = foldersModel->getFolderPath(currentIndex);
    QString folderPath = QDir::cleanPath(currentPath() + relativePath);

    if (!currentIndex.isValid())
        QMessageBox::information(this, tr("No folder selected"), tr("Please, select a folder first"));
    else {
        QString libraryPath = QDir::cleanPath(currentPath());
        if ((libraryPath == folderPath) || relativePath.isEmpty() || relativePath == "/")
            QMessageBox::critical(this, tr("Error in path"), tr("There was an error accessing the folder's path"));
        else {
            int ret = QMessageBox::question(this, tr("Delete folder"), tr("The selected folder and all its contents will be deleted from your disk. Are you sure?") + "\n\nFolder : " + folderPath, QMessageBox::Yes, QMessageBox::No);

            if (ret == QMessageBox::Yes) {
                // no folders multiselection by now
                QModelIndexList indexList;
                indexList << currentIndex;

                QList<QString> paths;
                paths << folderPath;

                auto remover = new FoldersRemover(indexList, paths);
                const auto thread = new QThread(this);
                moveAndConnectRemoverToThread(remover, thread);

                connect(remover, &FoldersRemover::remove, foldersModel, &FolderModel::deleteFolder);
                connect(remover, &FoldersRemover::removeError, this, &LibraryWindow::errorDeletingFolder);
                connect(remover, &FoldersRemover::finished, navigationController, &YACReaderNavigationController::reselectCurrentFolder);

                thread->start();
            }
        }
    }
}

void LibraryWindow::errorDeletingFolder()
{
    QMessageBox::critical(this, tr("Unable to delete"), tr("There was an issue trying to delete the selected folders. Please, check for write permissions and be sure that any applications are using these folders or any of the contained files."));
}

void LibraryWindow::addNewReadingList()
{
    QModelIndexList selectedLists = listsView->selectionModel()->selectedIndexes();
    QModelIndex sourceMI;
    if (!selectedLists.isEmpty())
        sourceMI = listsModelProxy->mapToSource(selectedLists.at(0));

    if (selectedLists.isEmpty() || !listsModel->isReadingSubList(sourceMI)) {
        bool ok;
        QString newListName = QInputDialog::getText(this, tr("Add new reading lists"),
                                                    tr("List name:"), QLineEdit::Normal,
                                                    "", &ok);
        if (ok) {
            if (selectedLists.isEmpty() || !listsModel->isReadingList(sourceMI))
                listsModel->addReadingList(newListName); // top level
            else {
                listsModel->addReadingListAt(newListName, sourceMI); // sublist
            }
        }
    }
}

void LibraryWindow::deleteSelectedReadingList()
{
    QModelIndexList selectedLists = listsView->selectionModel()->selectedIndexes();
    if (!selectedLists.isEmpty()) {
        QModelIndex mi = listsModelProxy->mapToSource(selectedLists.at(0));
        if (listsModel->isEditable(mi)) {
            int ret = QMessageBox::question(this, tr("Delete list/label"), tr("The selected item will be deleted, your comics or folders will NOT be deleted from your disk. Are you sure?"), QMessageBox::Yes, QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                listsModel->deleteItem(mi);
                navigationController->reselectCurrentList();
            }
        }
    }
}

void LibraryWindow::showAddNewLabelDialog()
{
    auto dialog = new AddLabelDialog();
    int ret = dialog->exec();

    if (ret == QDialog::Accepted) {
        YACReader::LabelColors color = dialog->selectedColor();
        QString name = dialog->name();

        listsModel->addNewLabel(name, color);
    }
}

// TODO implement editors in treeview
void LibraryWindow::showRenameCurrentList()
{
    QModelIndexList selectedLists = listsView->selectionModel()->selectedIndexes();
    if (!selectedLists.isEmpty()) {
        QModelIndex mi = listsModelProxy->mapToSource(selectedLists.at(0));
        if (listsModel->isEditable(mi)) {
            bool ok;
            QString newListName = QInputDialog::getText(this, tr("Rename list name"),
                                                        tr("List name:"), QLineEdit::Normal,
                                                        listsModel->name(mi), &ok);

            if (ok)
                listsModel->rename(mi, newListName);
        }
    }
}

void LibraryWindow::addSelectedComicsToFavorites()
{
    QModelIndexList indexList = getSelectedComics();
    comicsModel->addComicsToFavorites(indexList);
}

void LibraryWindow::showComicsViewContextMenu(const QPoint &point)
{
    showComicsContextMenu(point, true);
}

void LibraryWindow::showComicsItemContextMenu(const QPoint &point)
{
    showComicsContextMenu(point, false);
}

void LibraryWindow::showComicsContextMenu(const QPoint &point, bool showFullScreenAction)
{
    auto selection = this->getSelectedComics();

    auto setNormalAction = new QAction();
    setNormalAction->setText(tr("comic"));

    auto setMangaAction = new QAction();
    setMangaAction->setText(tr("manga"));

    auto setWesternMangaAction = new QAction();
    setWesternMangaAction->setText(tr("western manga (left to right)"));

    auto setWebComicAction = new QAction();
    setWebComicAction->setText(tr("web comic"));

    auto setYonkomaAction = new QAction();
    setYonkomaAction->setText(tr("4koma (top to botom)"));

    setNormalAction->setCheckable(true);
    setMangaAction->setCheckable(true);
    setWesternMangaAction->setCheckable(true);
    setWebComicAction->setCheckable(true);
    setYonkomaAction->setCheckable(true);

    connect(setNormalAction, &QAction::triggered, actions.setNormalAction, &QAction::trigger);
    connect(setMangaAction, &QAction::triggered, actions.setMangaAction, &QAction::trigger);
    connect(setWesternMangaAction, &QAction::triggered, actions.setWesternMangaAction, &QAction::trigger);
    connect(setWebComicAction, &QAction::triggered, actions.setWebComicAction, &QAction::trigger);
    connect(setYonkomaAction, &QAction::triggered, actions.setYonkomaAction, &QAction::trigger);

    auto setupActions = [=](FileType type) {
        switch (type) {
        case YACReader::FileType::Comic:
            setNormalAction->setChecked(true);
            break;
        case YACReader::FileType::Manga:
            setMangaAction->setChecked(true);
            break;
        case YACReader::FileType::WesternManga:
            setWesternMangaAction->setChecked(true);
            break;
        case YACReader::FileType::WebComic:
            setWebComicAction->setChecked(true);
            break;
        case YACReader::FileType::Yonkoma:
            setYonkomaAction->setChecked(true);
            break;
        }
    };

    if (selection.size() == 1) {
        QModelIndex index = selection.at(0);
        auto type = index.data(ComicModel::TypeRole).value<YACReader::FileType>();
        setupActions(type);
    }

    QMenu menu;

    menu.addAction(actions.openComicAction);
    menu.addAction(actions.saveCoversToAction);
    menu.addSeparator();
    menu.addAction(actions.openContainingFolderComicAction);
    menu.addAction(actions.updateCurrentFolderAction);
    menu.addSeparator();
    menu.addAction(actions.resetComicRatingAction);
    menu.addSeparator();
    menu.addAction(actions.editSelectedComicsAction);
    menu.addAction(actions.getInfoAction);
    menu.addAction(actions.asignOrderAction);
    menu.addSeparator();
    menu.addAction(actions.selectAllComicsAction);
    menu.addSeparator();
    menu.addAction(actions.setAsReadAction);
    menu.addAction(actions.setAsNonReadAction);
    menu.addSeparator();
    auto typeMenu = new QMenu(tr("Set type"));
    menu.addMenu(typeMenu);
    typeMenu->addAction(setNormalAction);
    typeMenu->addAction(setMangaAction);
    typeMenu->addAction(setWesternMangaAction);
    typeMenu->addAction(setWebComicAction);
    typeMenu->addAction(setYonkomaAction);
    menu.addSeparator();
    menu.addAction(actions.deleteMetadataAction);
    menu.addSeparator();
    menu.addAction(actions.deleteComicsAction);
    menu.addSeparator();
    menu.addAction(actions.addToMenuAction);
    QMenu subMenu;
    setupAddToSubmenu(subMenu);

#ifndef Q_OS_MACOS
    if (showFullScreenAction) {
        menu.addSeparator();
        menu.addAction(actions.toggleFullScreenAction);
    }
#endif

    menu.exec(contentViewsManager->comicsView->mapToGlobal(point));
}

void LibraryWindow::showGridFoldersContextMenu(QPoint point, Folder folder)
{
    QMenu menu;

    auto openContainingFolderAction = new QAction();
    openContainingFolderAction->setText(tr("Open folder..."));
    openContainingFolderAction->setIcon(QIcon(":/images/menus_icons/open_containing_folder.svg"));

    auto updateFolderAction = new QAction(tr("Update folder"), this);
    updateFolderAction->setIcon(QIcon(":/images/menus_icons/update_current_folder.svg"));

    auto rescanLibraryForXMLInfoAction = new QAction(tr("Rescan library for XML info"), this);

    auto setFolderAsNotCompletedAction = new QAction();
    setFolderAsNotCompletedAction->setText(tr("Set as uncompleted"));

    auto setFolderAsCompletedAction = new QAction();
    setFolderAsCompletedAction->setText(tr("Set as completed"));

    auto setFolderAsReadAction = new QAction();
    setFolderAsReadAction->setText(tr("Set as read"));

    auto setFolderAsUnreadAction = new QAction();
    setFolderAsUnreadAction->setText(tr("Set as unread"));

    auto setFolderAsMangaAction = new QAction();
    setFolderAsMangaAction->setText(tr("manga"));

    auto setFolderAsNormalAction = new QAction();
    setFolderAsNormalAction->setText(tr("comic"));

    auto setFolderAsWesternMangaAction = new QAction();
    setFolderAsWesternMangaAction->setText(tr("western manga (left to right)"));

    auto setFolderAsWebComicAction = new QAction();
    setFolderAsWebComicAction->setText(tr("web comic"));

    auto setFolderAs4KomaAction = new QAction();
    setFolderAs4KomaAction->setText(tr("4koma (top to botom)"));

    menu.addAction(openContainingFolderAction);
    menu.addAction(updateFolderAction);
    menu.addSeparator();
    menu.addAction(rescanLibraryForXMLInfoAction);
    menu.addSeparator();
    if (folder.completed)
        menu.addAction(setFolderAsNotCompletedAction);
    else
        menu.addAction(setFolderAsCompletedAction);
    menu.addSeparator();
    if (folder.finished)
        menu.addAction(setFolderAsUnreadAction);
    else
        menu.addAction(setFolderAsReadAction);
    menu.addSeparator();

    setFolderAsNormalAction->setCheckable(true);
    setFolderAsMangaAction->setCheckable(true);
    setFolderAsWesternMangaAction->setCheckable(true);
    setFolderAsWebComicAction->setCheckable(true);
    setFolderAs4KomaAction->setCheckable(true);

    switch (folder.type) {
    case FileType::Comic:
        setFolderAsNormalAction->setChecked(true);
        break;
    case FileType::Manga:
        setFolderAsMangaAction->setChecked(true);
        break;
    case FileType::WesternManga:
        setFolderAsWesternMangaAction->setChecked(true);
        break;
    case FileType::WebComic:
        setFolderAsWebComicAction->setChecked(true);
        break;
    case FileType::Yonkoma:
        setFolderAs4KomaAction->setChecked(true);
        break;
    }

    auto typeMenu = new QMenu(tr("Set type"));
    menu.addMenu(typeMenu);
    typeMenu->addAction(setFolderAsNormalAction);
    typeMenu->addAction(setFolderAsMangaAction);
    typeMenu->addAction(setFolderAsWesternMangaAction);
    typeMenu->addAction(setFolderAsWebComicAction);
    typeMenu->addAction(setFolderAs4KomaAction);

    auto subfolderModel = contentViewsManager->folderContentView->currentFolderModel();

    connect(openContainingFolderAction, &QAction::triggered, this, [=]() {
        QDesktopServices::openUrl(QUrl("file:///" + QDir::cleanPath(currentPath() + "/" + folder.path), QUrl::TolerantMode));
    });
    connect(updateFolderAction, &QAction::triggered, this, [=]() {
        updateFolder(foldersModel->getIndexFromFolder(folder));
    });
    connect(rescanLibraryForXMLInfoAction, &QAction::triggered, this, [=]() {
        rescanFolderForXMLInfo(foldersModel->getIndexFromFolder(folder));
    });
    connect(setFolderAsNotCompletedAction, &QAction::triggered, this, [=]() {
        foldersModel->updateFolderCompletedStatus(QModelIndexList() << foldersModel->getIndexFromFolder(folder), false);
        subfolderModel->updateFolderCompletedStatus(QModelIndexList() << subfolderModel->getIndexFromFolder(folder), false);
    });
    connect(setFolderAsCompletedAction, &QAction::triggered, this, [=]() {
        foldersModel->updateFolderCompletedStatus(QModelIndexList() << foldersModel->getIndexFromFolder(folder), true);
        subfolderModel->updateFolderCompletedStatus(QModelIndexList() << subfolderModel->getIndexFromFolder(folder), true);
    });
    connect(setFolderAsReadAction, &QAction::triggered, this, [=]() {
        foldersModel->updateFolderFinishedStatus(QModelIndexList() << foldersModel->getIndexFromFolder(folder), true);
        subfolderModel->updateFolderFinishedStatus(QModelIndexList() << subfolderModel->getIndexFromFolder(folder), true);
    });
    connect(setFolderAsUnreadAction, &QAction::triggered, this, [=]() {
        foldersModel->updateFolderFinishedStatus(QModelIndexList() << foldersModel->getIndexFromFolder(folder), false);
        subfolderModel->updateFolderFinishedStatus(QModelIndexList() << subfolderModel->getIndexFromFolder(folder), false);
    });
    connect(setFolderAsMangaAction, &QAction::triggered, this, [=]() {
        foldersModel->updateFolderType(QModelIndexList() << foldersModel->getIndexFromFolder(folder), FileType::Manga);
        subfolderModel->updateFolderType(QModelIndexList() << foldersModel->getIndexFromFolder(folder), FileType::Manga);
    });
    connect(setFolderAsNormalAction, &QAction::triggered, this, [=]() {
        foldersModel->updateFolderType(QModelIndexList() << foldersModel->getIndexFromFolder(folder), FileType::Comic);
        subfolderModel->updateFolderType(QModelIndexList() << foldersModel->getIndexFromFolder(folder), FileType::Comic);
    });
    connect(setFolderAsWesternMangaAction, &QAction::triggered, this, [=]() {
        foldersModel->updateFolderType(QModelIndexList() << foldersModel->getIndexFromFolder(folder), FileType::WesternManga);
        subfolderModel->updateFolderType(QModelIndexList() << foldersModel->getIndexFromFolder(folder), FileType::WesternManga);
    });
    connect(setFolderAsWebComicAction, &QAction::triggered, this, [=]() {
        foldersModel->updateFolderType(QModelIndexList() << foldersModel->getIndexFromFolder(folder), FileType::WebComic);
        subfolderModel->updateFolderType(QModelIndexList() << foldersModel->getIndexFromFolder(folder), FileType::WebComic);
    });
    connect(setFolderAs4KomaAction, &QAction::triggered, this, [=]() {
        foldersModel->updateFolderType(QModelIndexList() << foldersModel->getIndexFromFolder(folder), FileType::Yonkoma);
        subfolderModel->updateFolderType(QModelIndexList() << foldersModel->getIndexFromFolder(folder), FileType::Yonkoma);
    });

    menu.exec(contentViewsManager->folderContentView->mapToGlobal(point));
}

void LibraryWindow::showContinueReadingContextMenu(QPoint point, ComicDB comic)
{
    QMenu menu;

    auto setAsUnReadAction = new QAction();
    setAsUnReadAction->setText(tr("Set as unread"));
    setAsUnReadAction->setIcon(QIcon(":/images/comics_view_toolbar/setUnread.svg"));

    menu.addAction(setAsUnReadAction);

    connect(setAsUnReadAction, &QAction::triggered, this, [=]() {
        auto libraryId = libraries.getId(selectedLibrary->currentText());
        auto info = comic.info;
        info.setRead(false);
        info.currentPage = 1;
        info.hasBeenOpened = false;
        info.lastTimeOpened = QVariant();
        DBHelper::update(libraryId, info);

        contentViewsManager->folderContentView->reloadContinueReadingModel();
    });

    menu.exec(contentViewsManager->folderContentView->mapToGlobal(point));
}

void LibraryWindow::setupAddToSubmenu(QMenu &menu)
{
    menu.addAction(actions.addToFavoritesAction);
    actions.addToMenuAction->setMenu(&menu);

    const QList<LabelItem *> labels = listsModel->getLabels();
    if (labels.count() > 0)
        menu.addSeparator();
    foreach (LabelItem *label, labels) {
        auto action = new QAction(this);
        action->setIcon(label->getIcon());
        action->setText(label->name());

        action->setData(label->getId());

        menu.addAction(action);

        connect(action, &QAction::triggered, this, &LibraryWindow::onAddComicsToLabel);
    }
}

void LibraryWindow::onAddComicsToLabel()
{
    auto action = static_cast<QAction *>(sender());

    qulonglong labelId = action->data().toULongLong();

    QModelIndexList comics = getSelectedComics();

    comicsModel->addComicsToLabel(comics, labelId);
}

void LibraryWindow::setToolbarTitle(const QModelIndex &modelIndex)
{
#ifndef Y_MAC_UI
    if (!modelIndex.isValid())
        libraryToolBar->setCurrentFolderName(selectedLibrary->currentText());
    else
        libraryToolBar->setCurrentFolderName(modelIndex.data().toString());
#endif
}

void LibraryWindow::saveSelectedCoversTo()
{
    QFileDialog saveDialog;
    QString folderPath = saveDialog.getExistingDirectory(this, tr("Save covers"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    if (!folderPath.isEmpty()) {
        QModelIndexList comics = getSelectedComics();
        foreach (QModelIndex comic, comics) {
            QString origin = comic.data(ComicModel::CoverPathRole).toString().remove("file:///").remove("file:");
            QString destination = QDir(folderPath).filePath(comic.data(ComicModel::FileNameRole).toString() + ".jpg");

            QLOG_DEBUG() << "From : " << origin;
            QLOG_DEBUG() << "To : " << destination;

            QFile::copy(origin, destination);
        }
    }
}

void LibraryWindow::checkMaxNumLibraries()
{
    int numLibraries = libraries.getNames().length();
    if (numLibraries >= MAX_LIBRARIES_WARNING_NUM) {
        QMessageBox::warning(this, tr("You are adding too many libraries."), tr("You are adding too many libraries.\n\nYou probably only need one library in your top level comics folder, you can browse any subfolders using the folders section in the left sidebar.\n\nYACReaderLibrary will not stop you from creating more libraries but you should keep the number of libraries low."));
    }
}

void LibraryWindow::selectSubfolder(const QModelIndex &mi, int child)
{
    QModelIndex dest = foldersModel->index(child, 0, mi);
    foldersView->setCurrentIndex(dest);
    navigationController->selectedFolder(dest);
}

// this methods is only using after deleting comics
// TODO broken window :)
void LibraryWindow::checkEmptyFolder()
{
    if (comicsModel->rowCount() > 0 && !importedCovers) {
        disableComicsActions(false);
    } else {
        disableComicsActions(true);
#ifndef Q_OS_MACOS
        if (comicsModel->rowCount() > 0)
            actions.toggleFullScreenAction->setEnabled(true);
#endif
        if (comicsModel->rowCount() == 0)
            navigationController->reselectCurrentFolder();
    }
}

void LibraryWindow::openComic()
{
    if (!importedCovers) {

        auto comic = comicsModel->getComic(contentViewsManager->comicsView->currentIndex());
        auto mode = comicsModel->getMode();

        openComic(comic, mode);
    }
}

void LibraryWindow::openComic(const ComicDB &comic, const ComicModel::Mode mode)
{
    auto libraryId = libraries.getId(selectedLibrary->currentText());

    OpenComicSource::Source source;

    if (mode == ComicModel::ReadingList) {
        source = OpenComicSource::Source::ReadingList;
    } else if (mode == ComicModel::Reading) {
        // TODO check where the comic was opened from the last time it was read
        source = OpenComicSource::Source::Folder;
    } else {
        source = OpenComicSource::Source::Folder;
    }

    auto thirdPartyReaderCommand = settings->value(THIRD_PARTY_READER_COMMAND, "").toString();
    if (thirdPartyReaderCommand.isEmpty()) {
        auto yacreaderFound = YACReader::openComic(comic, libraryId, currentPath(), OpenComicSource { source, comicsModel->getSourceId() });

        if (!yacreaderFound) {
#ifdef Q_OS_WIN
            QMessageBox::critical(this, tr("YACReader not found"), tr("YACReader not found. YACReader should be installed in the same folder as YACReaderLibrary."));
#else
            QMessageBox::critical(this, tr("YACReader not found"), tr("YACReader not found. There might be a problem with your YACReader installation."));
#endif
        }
    } else {
        auto exec = YACReader::openComicInThirdPartyApp(thirdPartyReaderCommand, QDir::cleanPath(currentPath() + comic.path));

        if (!exec) {
            QMessageBox::critical(this, tr("Error"), tr("Error opening comic with third party reader."));
        }
    }
}

void LibraryWindow::setCurrentComicsStatusReaded(YACReaderComicReadStatus readStatus)
{
    comicsModel->setComicsRead(getSelectedComics(), readStatus);
    contentViewsManager->updateCurrentComicView();
}

void LibraryWindow::setCurrentComicReaded()
{
    this->setCurrentComicsStatusReaded(YACReader::Read);
}

void LibraryWindow::setCurrentComicUnreaded()
{
    this->setCurrentComicsStatusReaded(YACReader::Unread);
}

void LibraryWindow::setSelectedComicsType(FileType type)
{
    comicsModel->setComicsType(getSelectedComics(), type);
}

void LibraryWindow::createLibrary()
{
    checkMaxNumLibraries();
    createLibraryDialog->open(libraries);
}

void LibraryWindow::create(QString source, QString dest, QString name)
{
    QLOG_INFO() << QString("About to create a library from '%1' to '%2' with name '%3'").arg(source, dest, name);
    libraryCreator->createLibrary(source, dest);
    libraryCreator->start();
    _lastAdded = name;
    _sourceLastAdded = source;

    importWidget->setImportLook();
    showImportingWidget();
}

void LibraryWindow::reloadCurrentLibrary()
{
    foldersModel->reload();
    contentViewsManager->updateCurrentContentView();

    enableNeededActions();
}

void LibraryWindow::openLastCreated()
{

    selectedLibrary->disconnect();

    selectedLibrary->setCurrentIndex(selectedLibrary->findText(_lastAdded));
    libraries.addLibrary(_lastAdded, _sourceLastAdded);
    selectedLibrary->addItem(_lastAdded, _sourceLastAdded);
    selectedLibrary->setCurrentIndex(selectedLibrary->findText(_lastAdded));
    libraries.save();

    connect(selectedLibrary, &YACReaderLibraryListWidget::currentIndexChanged, this, &LibraryWindow::loadLibrary);

    loadLibrary(_lastAdded);
}

void LibraryWindow::showAddLibrary()
{
    checkMaxNumLibraries();
    addLibraryDialog->open();
}

void LibraryWindow::openLibrary(QString path, QString name)
{
    if (!libraries.contains(name)) {
        // TODO: fix bug, /a/b/c/.yacreaderlibrary/d/e
        path.remove("/.yacreaderlibrary");
        QDir d; // TODO change this by static methods (utils class?? with delTree for example)
        if (d.exists(path + "/.yacreaderlibrary")) {
            _lastAdded = name;
            _sourceLastAdded = path;
            openLastCreated();
            addLibraryDialog->close();
        } else
            QMessageBox::warning(this, tr("Library not found"), tr("The selected folder doesn't contain any library."));
    } else {
        libraryAlreadyExists(name);
    }
}

void LibraryWindow::loadLibraries()
{
    libraries.load();
    foreach (QString name, libraries.getNames())
        selectedLibrary->addItem(name, libraries.getPath(name));
}

void LibraryWindow::saveLibraries()
{
    libraries.save();
}

void LibraryWindow::updateLibrary()
{
    importWidget->setUpdateLook();
    showImportingWidget();

    QString currentLibrary = selectedLibrary->currentText();
    QString path = libraries.getPath(currentLibrary);
    _lastAdded = currentLibrary;
    libraryCreator->updateLibrary(path, path + "/.yacreaderlibrary");
    libraryCreator->start();
}

void LibraryWindow::deleteCurrentLibrary()
{
    QString path = libraries.getPath(selectedLibrary->currentText());
    libraries.remove(selectedLibrary->currentText());
    selectedLibrary->removeItem(selectedLibrary->currentIndex());
    // selectedLibrary->setCurrentIndex(0);
    path = path + "/.yacreaderlibrary";

    QDir d(path);
    d.removeRecursively();
    if (libraries.isEmpty()) // no more libraries available.
    {
        contentViewsManager->comicsView->setModel(NULL);
        foldersView->setModel(NULL);
        listsView->setModel(NULL);

        actions.disableAllActions();
        showNoLibrariesWidget();
    }
    libraries.save();
}

void LibraryWindow::removeLibrary()
{
    QString currentLibrary = selectedLibrary->currentText();
    QMessageBox *messageBox = new QMessageBox(tr("Are you sure?"), tr("Do you want remove ") + currentLibrary + tr(" library?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::YesToAll, QMessageBox::No);
    messageBox->button(QMessageBox::YesToAll)->setText(tr("Remove and delete metadata"));
    messageBox->setParent(this);
    messageBox->setWindowModality(Qt::WindowModal);
    int ret = messageBox->exec();
    if (ret == QMessageBox::Yes) {
        libraries.remove(currentLibrary);
        selectedLibrary->removeItem(selectedLibrary->currentIndex());
        // selectedLibrary->setCurrentIndex(0);
        if (libraries.isEmpty()) // no more libraries available.
        {
            contentViewsManager->comicsView->setModel(NULL);
            foldersView->setModel(NULL);
            listsView->setModel(NULL);

            actions.disableAllActions();
            showNoLibrariesWidget();
        }
        libraries.save();
    } else if (ret == QMessageBox::YesToAll) {
        deleteCurrentLibrary();
    }
}

void LibraryWindow::renameLibrary()
{
    renameLibraryDialog->open();
}

void LibraryWindow::rename(QString newName) // TODO replace
{
    QString currentLibrary = selectedLibrary->currentText();
    if (newName != currentLibrary) {
        if (!libraries.contains(newName)) {
            libraries.rename(currentLibrary, newName);
            // selectedLibrary->removeItem(selectedLibrary->currentIndex());
            // libraries.addLibrary(newName,path);
            selectedLibrary->renameCurrentLibrary(newName);
            libraries.save();
            renameLibraryDialog->close();
#ifndef Y_MAC_UI
            if (!foldersModelProxy->mapToSource(foldersView->currentIndex()).isValid())
                libraryToolBar->setCurrentFolderName(selectedLibrary->currentText());
#endif
        } else {
            libraryAlreadyExists(newName);
        }
    } else
        renameLibraryDialog->close();
    // selectedLibrary->setCurrentIndex(selectedLibrary->findText(newName));
}

void LibraryWindow::rescanLibraryForXMLInfo()
{
    importWidget->setXMLScanLook();
    showImportingWidget();

    QString currentLibrary = selectedLibrary->currentText();
    QString path = libraries.getPath(currentLibrary);
    _lastAdded = currentLibrary;

    xmlInfoLibraryScanner->scanLibrary(path, path + "/.yacreaderlibrary");
}

void LibraryWindow::showLibraryInfo()
{
    auto id = libraries.getUuid(selectedLibrary->currentText());
    auto info = DBHelper::getLibraryInfo(id);

    // TODO: use something nicer than a QMessageBox
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Library info"));
    msgBox.setText(info);
    QSpacerItem *horizontalSpacer = new QSpacerItem(420, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout *layout = (QGridLayout *)msgBox.layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.setDefaultButton(QMessageBox::Close);
    msgBox.exec();
}

void LibraryWindow::rescanCurrentFolderForXMLInfo()
{
    rescanFolderForXMLInfo(getCurrentFolderIndex());
}

void LibraryWindow::rescanFolderForXMLInfo(QModelIndex modelIndex)
{
    importWidget->setXMLScanLook();
    showImportingWidget();

    QString currentLibrary = selectedLibrary->currentText();
    QString path = libraries.getPath(currentLibrary);
    _lastAdded = currentLibrary;

    xmlInfoLibraryScanner->scanFolder(path, path + "/.yacreaderlibrary", QDir::cleanPath(currentPath() + foldersModel->getFolderPath(modelIndex)), modelIndex);
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

void LibraryWindow::stopXMLScanning()
{
    xmlInfoLibraryScanner->stop();
    xmlInfoLibraryScanner->wait();
}

void LibraryWindow::setRootIndex()
{
    if (!libraries.isEmpty()) {
        QString path = libraries.getPath(selectedLibrary->currentText()) + "/.yacreaderlibrary";
        QDir d; // TODO change this by static methods (utils class?? with delTree for example)
        if (d.exists(path)) {
            navigationController->selectedFolder(QModelIndex());
        } else {
            contentViewsManager->comicsView->setModel(NULL);
        }

        foldersView->selectionModel()->clear();
    }
}

void LibraryWindow::toggleFullScreen()
{
    fullscreen ? toNormal() : toFullScreen();
    fullscreen = !fullscreen;
}

#ifdef Q_OS_WIN // fullscreen mode in Windows for preventing this bug: QTBUG-41309 https://bugreports.qt.io/browse/QTBUG-41309
void LibraryWindow::toFullScreen()
{
    fromMaximized = this->isMaximized();

    sideBar->hide();
    libraryToolBar->hide();

    previousWindowFlags = windowFlags();
    previousPos = pos();
    previousSize = size();

    showNormal();
    setWindowFlags(previousWindowFlags | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    QRect r = windowHandle()->screen()->geometry();

    r.setHeight(r.height() + 1);

    setGeometry(r);
    show();

    contentViewsManager->toFullscreen();
}

void LibraryWindow::toNormal()
{
    sideBar->show();
    libraryToolBar->show();

    setWindowFlags(previousWindowFlags);
    move(previousPos);
    resize(previousSize);
    show();

    if (fromMaximized)
        showMaximized();

    contentViewsManager->toNormal();
}

#else

void LibraryWindow::toFullScreen()
{
    fromMaximized = this->isMaximized();

    sideBar->hide();
    libraryToolBar->hide();

    contentViewsManager->toFullscreen();

    showFullScreen();
}

void LibraryWindow::toNormal()
{
    sideBar->show();

    contentViewsManager->toNormal();

    if (fromMaximized)
        showMaximized();
    else
        showNormal();

#ifdef Y_MAC_UI
    auto timer = new QTimer();
    timer->setSingleShot(true);
    timer->start();
    connect(timer, &QTimer::timeout, libraryToolBar, &YACReaderMacOSXToolbar::show);
    connect(timer, &QTimer::timeout, timer, &QTimer::deleteLater);
#else
    libraryToolBar->show();
#endif
}

#endif

void LibraryWindow::setSearchFilter(QString filter)
{
    if (!filter.isEmpty()) {
        folderQueryResultProcessor->createModelData(filter);
        comicQueryResultProcessor.createModelData(filter, foldersModel->getDatabase());
    } else if (status == LibraryWindow::Searching) { // if no searching, then ignore this
        clearSearchFilter();
        navigationController->loadPreviousStatus();
    }
}

void LibraryWindow::setComicSearchFilterData(QList<ComicItem *> *data, const QString &databasePath)
{
    status = LibraryWindow::Searching;

    comicsModel->setModelData(data, databasePath);
    contentViewsManager->comicsView->enableFilterMode(true);
    contentViewsManager->comicsView->setModel(comicsModel); // TODO, columns are messed up after ResetModel some times, this shouldn't be necesary

    if (comicsModel->rowCount() == 0) {
        contentViewsManager->showNoSearchResultsView();
        disableComicsActions(true);
    } else {
        contentViewsManager->showComicsView();
        disableComicsActions(false);
    }
}

void LibraryWindow::setFolderSearchFilterData(QMap<unsigned long long, FolderItem *> *filteredItems, FolderItem *root)
{
    foldersModelProxy->setFilterData(filteredItems, root);
    foldersView->expandAll();
}

void LibraryWindow::clearSearchFilter()
{
    foldersModelProxy->clear();
    contentViewsManager->comicsView->enableFilterMode(false);
    foldersView->collapseAll();
    status = LibraryWindow::Normal;
}

void LibraryWindow::showProperties()
{
    QModelIndexList indexList = getSelectedComics();

    QList<ComicDB> comics = comicsModel->getComics(indexList);
    ComicDB c = comics[0];
    _comicIdEdited = c.id; // static_cast<TableItem*>(indexList[0].internalPointer())->data(4).toULongLong();

    propertiesDialog->databasePath = foldersModel->getDatabase();
    propertiesDialog->basePath = currentPath();

    if (indexList.length() > 1) { // edit common properties
        propertiesDialog->setComics(comics);
    } else {
        auto allComics = comicsModel->getAllComics();
        int index = allComics.indexOf(c);
        propertiesDialog->setComicsForSequentialEditing(index, comicsModel->getAllComics());
    }

    propertiesDialog->show();
}

void LibraryWindow::showComicVineScraper()
{
    QSettings s(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat); // TODO unificar la creación del fichero de config con el servidor
    s.beginGroup("ComicVine");

    if (!s.contains(COMIC_VINE_API_KEY)) {
        ApiKeyDialog d;
        d.exec();
    }

    // check if the api key was inserted
    if (s.contains(COMIC_VINE_API_KEY)) {
        QModelIndexList indexList = getSelectedComics();

        QList<ComicDB> comics = comicsModel->getComics(indexList);
        ComicDB c = comics[0];
        _comicIdEdited = c.id; // static_cast<TableItem*>(indexList[0].internalPointer())->data(4).toULongLong();

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
    if (removeError) {
        QMessageBox::critical(this, tr("Unable to delete"), tr("There was an issue trying to delete the selected comics. Please, check for write permissions in the selected files or containing folder."));
    }
    removeError = false;
}

void LibraryWindow::resetComicRating()
{
    QModelIndexList indexList = getSelectedComics();

    comicsModel->startTransaction();
    for (auto &index : indexList) {
        comicsModel->resetComicRating(index);
    }
    comicsModel->finishTransaction();
}

void LibraryWindow::checkSearchNumResults(int numResults)
{
    if (numResults == 0)
        contentViewsManager->showNoSearchResultsView();
    else
        contentViewsManager->showComicsView();
}

void LibraryWindow::asignNumbers()
{
    QModelIndexList indexList = getSelectedComics();

    int startingNumber = indexList[0].row() + 1;
    if (indexList.count() > 1) {
        bool ok;
        int n = QInputDialog::getInt(this, tr("Assign comics numbers"),
                                     tr("Assign numbers starting in:"), startingNumber, 0, 2147483647, 1, &ok);
        if (ok)
            startingNumber = n;
        else
            return;
    }
    qint64 edited = comicsModel->asignNumbers(indexList, startingNumber);

    // TODO add resorting without reloading
    navigationController->loadFolderInfo(foldersModelProxy->mapToSource(foldersView->currentIndex()));

    const QModelIndex &mi = comicsModel->getIndexFromId(edited);
    if (mi.isValid()) {
        contentViewsManager->comicsView->scrollTo(mi, QAbstractItemView::PositionAtCenter);
        contentViewsManager->comicsView->setCurrentIndex(mi);
    }
}

void LibraryWindow::openContainingFolderComic()
{
    QModelIndex modelIndex = contentViewsManager->comicsView->currentIndex();
    QFileInfo file(QDir::cleanPath(currentPath() + comicsModel->getComicPath(modelIndex)));
#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    QString path = file.absolutePath();
    QDesktopServices::openUrl(QUrl("file:///" + path, QUrl::TolerantMode));
#endif

#ifdef Q_OS_MACOS
    QString filePath = file.absoluteFilePath();
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \"" + filePath + "\"";
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
    QModelIndex modelIndex = foldersModelProxy->mapToSource(foldersView->currentIndex());
    QString path;
    if (modelIndex.isValid())
        path = QDir::cleanPath(currentPath() + foldersModel->getFolderPath(modelIndex));
    else
        path = QDir::cleanPath(currentPath());
    QDesktopServices::openUrl(QUrl("file:///" + path, QUrl::TolerantMode));
}

void LibraryWindow::setFolderAsNotCompleted()
{
    // foldersModel->updateFolderCompletedStatus(foldersView->selectionModel()->selectedRows(),false);
    foldersModel->updateFolderCompletedStatus(QModelIndexList() << foldersModelProxy->mapToSource(foldersView->currentIndex()), false);
}

void LibraryWindow::setFolderAsCompleted()
{
    // foldersModel->updateFolderCompletedStatus(foldersView->selectionModel()->selectedRows(),true);
    foldersModel->updateFolderCompletedStatus(QModelIndexList() << foldersModelProxy->mapToSource(foldersView->currentIndex()), true);
}

void LibraryWindow::setFolderAsRead()
{
    // foldersModel->updateFolderFinishedStatus(foldersView->selectionModel()->selectedRows(),true);
    foldersModel->updateFolderFinishedStatus(QModelIndexList() << foldersModelProxy->mapToSource(foldersView->currentIndex()), true);
}

void LibraryWindow::setFolderAsUnread()
{
    // foldersModel->updateFolderFinishedStatus(foldersView->selectionModel()->selectedRows(),false);
    foldersModel->updateFolderFinishedStatus(QModelIndexList() << foldersModelProxy->mapToSource(foldersView->currentIndex()), false);
}

void LibraryWindow::setFolderType(FileType type)
{
    foldersModel->updateFolderType(QModelIndexList() << foldersModelProxy->mapToSource(foldersView->currentIndex()), type);
}

void LibraryWindow::exportLibrary(QString destPath)
{
    QString currentLibrary = selectedLibrary->currentText();
    QString path = libraries.getPath(currentLibrary) + "/.yacreaderlibrary";
    packageManager->createPackage(path, destPath + "/" + currentLibrary);
}

void LibraryWindow::importLibrary(QString clc, QString destPath, QString name)
{
    packageManager->extractPackage(clc, destPath + "/" + name);
    _lastAdded = name;
    _sourceLastAdded = destPath + "/" + name;
}

void LibraryWindow::reloadOptions()
{
    contentViewsManager->comicsView->updateConfig(settings);

    trayIconController->updateIconVisibility();

    recentVisibilityCoordinator->updateTimeRange();
}

QString LibraryWindow::currentPath()
{
    return libraries.getPath(selectedLibrary->currentText());
}

QString LibraryWindow::currentFolderPath()
{
    QString path;

    if (foldersView->selectionModel()->selectedRows().length() > 0)
        path = foldersModel->getFolderPath(foldersModelProxy->mapToSource(foldersView->currentIndex()));
    else
        path = foldersModel->getFolderPath(QModelIndex());

    QLOG_DEBUG() << "current folder path : " << QDir::cleanPath(currentPath() + path);

    return QDir::cleanPath(currentPath() + path);
}

void LibraryWindow::showExportComicsInfo()
{
    exportComicsInfoDialog->source = currentPath() + "/.yacreaderlibrary/library.ydb";
    exportComicsInfoDialog->open();
}

void LibraryWindow::showImportComicsInfo()
{
    importComicsInfoDialog->dest = currentPath() + "/.yacreaderlibrary/library.ydb";
    importComicsInfoDialog->open();
}

void LibraryWindow::closeEvent(QCloseEvent *event)
{
    if (!trayIconController->handleCloseToTrayIcon(event)) {
        event->accept();
        closeApp();
    }
}

void LibraryWindow::prepareToCloseApp()
{
    httpServer->stop();

    libraryCreator->stop();
    librariesUpdateCoordinator->stop();

    settings->setValue(MAIN_WINDOW_GEOMETRY, saveGeometry());

    contentViewsManager->comicsView->close();
    sideBar->close();

    QApplication::instance()->processEvents();
}

void LibraryWindow::closeApp()
{
    prepareToCloseApp();

    qApp->exit(0);
}

void LibraryWindow::showNoLibrariesWidget()
{
    actions.disableAllActions();
    searchEdit->setDisabled(true);
    mainWidget->setCurrentIndex(1);
}

void LibraryWindow::showRootWidget()
{
#ifndef Y_MAC_UI
    libraryToolBar->setDisabled(false);
#endif
    searchEdit->setEnabled(true);
    mainWidget->setCurrentIndex(0);
}

void LibraryWindow::showImportingWidget()
{
    actions.disableAllActions();
    importWidget->clear();
#ifndef Y_MAC_UI
    libraryToolBar->setDisabled(true);
#endif
    searchEdit->setDisabled(true);
    mainWidget->setCurrentIndex(2);
}

void LibraryWindow::manageCreatingError(const QString &error)
{
    QMessageBox::critical(this, tr("Error creating the library"), error);
}

void LibraryWindow::manageUpdatingError(const QString &error)
{
    QMessageBox::critical(this, tr("Error updating the library"), error);
}

void LibraryWindow::manageOpeningLibraryError(const QString &error)
{
    QMessageBox::critical(this, tr("Error opening the library"), error);
}

bool lessThanModelIndexRow(const QModelIndex &m1, const QModelIndex &m2)
{
    return m1.row() < m2.row();
}

QModelIndexList LibraryWindow::getSelectedComics()
{
    // se fuerza a que haya almenos una fila seleccionada TODO comprobar se se puede forzar a la tabla a que lo haga automáticamente
    // avoid selection.count()==0 forcing selection in comicsView
    QModelIndexList selection = contentViewsManager->comicsView->selectionModel()->selectedRows();
    QLOG_TRACE() << "selection count " << selection.length();
    std::sort(selection.begin(), selection.end(), lessThanModelIndexRow);

    if (selection.count() == 0) {
        contentViewsManager->comicsView->selectIndex(0);
        selection = contentViewsManager->comicsView->selectionModel()->selectedRows();
    }
    return selection;
}

void LibraryWindow::deleteMetadataFromSelectedComics()
{
    QModelIndexList indexList = getSelectedComics();
    QList<ComicDB> comics = comicsModel->getComics(indexList);

    for (auto &comic : comics) {
        comic.info.deleteMetadata();
    }

    DBHelper::updateComicsInfo(comics, foldersModel->getDatabase());

    comicsModel->reload();
}

void LibraryWindow::deleteComics()
{
    // TODO
    if (!listsView->selectionModel()->selectedRows().isEmpty()) {
        deleteComicsFromList();
    } else {
        deleteComicsFromDisk();
    }
}

void LibraryWindow::deleteComicsFromDisk()
{
    int ret = QMessageBox::question(this, tr("Delete comics"), tr("All the selected comics will be deleted from your disk. Are you sure?"), QMessageBox::Yes, QMessageBox::No);

    if (ret == QMessageBox::Yes) {

        QModelIndexList indexList = getSelectedComics();

        QList<ComicDB> comics = comicsModel->getComics(indexList);

        QList<QString> paths;
        QString libraryPath = currentPath();
        foreach (ComicDB comic, comics) {
            paths.append(libraryPath + comic.path);
            QLOG_TRACE() << comic.path;
            QLOG_TRACE() << comic.id;
            QLOG_TRACE() << comic.parentId;
        }

        auto remover = new ComicsRemover(indexList, paths, comics.at(0).parentId);
        const auto thread = new QThread(this);
        moveAndConnectRemoverToThread(remover, thread);

        comicsModel->startTransaction();

        connect(remover, &ComicsRemover::remove, comicsModel, &ComicModel::remove);
        connect(remover, &ComicsRemover::removeError, this, &LibraryWindow::setRemoveError);
        connect(remover, &ComicsRemover::finished, comicsModel, &ComicModel::finishTransaction);
        connect(remover, &ComicsRemover::removedItemsFromFolder, foldersModel, &FolderModel::updateFolderChildrenInfo);

        connect(remover, &ComicsRemover::finished, this, &LibraryWindow::checkEmptyFolder);
        connect(remover, &ComicsRemover::finished, this, &LibraryWindow::checkRemoveError);

        thread->start();
    }
}

void LibraryWindow::deleteComicsFromList()
{
    int ret = QMessageBox::question(this, tr("Remove comics"), tr("Comics will only be deleted from the current label/list. Are you sure?"), QMessageBox::Yes, QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QModelIndexList indexList = getSelectedComics();
        if (indexList.isEmpty())
            return;

        QModelIndex mi = listsModelProxy->mapToSource(listsView->currentIndex());

        ReadingListModel::TypeList typeList = (ReadingListModel::TypeList)mi.data(ReadingListModel::TypeListsRole).toInt();

        qulonglong id = mi.data(ReadingListModel::IDRole).toULongLong();
        switch (typeList) {
        case ReadingListModel::SpecialList:
            comicsModel->deleteComicsFromSpecialList(indexList, id);
            break;
        case ReadingListModel::Label:
            comicsModel->deleteComicsFromLabel(indexList, id);
            break;
        case ReadingListModel::ReadingList:
            comicsModel->deleteComicsFromReadingList(indexList, id);
            break;
        case ReadingListModel::Separator:
            break;
        }
    }
}

void LibraryWindow::showFoldersContextMenu(const QPoint &point)
{
    QModelIndex sourceMI = foldersModelProxy->mapToSource(foldersView->indexAt(point));

    bool isCompleted = sourceMI.data(FolderModel::CompletedRole).toBool();
    bool isRead = sourceMI.data(FolderModel::FinishedRole).toBool();
    auto type = sourceMI.data(FolderModel::TypeRole).value<YACReader::FileType>();

    actions.setFolderAsNormalAction->setCheckable(true);
    actions.setFolderAsMangaAction->setCheckable(true);
    actions.setFolderAsWesternMangaAction->setCheckable(true);
    actions.setFolderAsWebComicAction->setCheckable(true);
    actions.setFolderAsYonkomaAction->setCheckable(true);

    actions.setFolderAsNormalAction->setChecked(false);
    actions.setFolderAsMangaAction->setChecked(false);
    actions.setFolderAsWesternMangaAction->setChecked(false);
    actions.setFolderAsWebComicAction->setChecked(false);
    actions.setFolderAsYonkomaAction->setChecked(false);

    switch (type) {
    case FileType::Comic:
        actions.setFolderAsNormalAction->setChecked(true);
        break;
    case FileType::Manga:
        actions.setFolderAsMangaAction->setChecked(true);
        break;
    case FileType::WesternManga:
        actions.setFolderAsWesternMangaAction->setChecked(true);
        break;
    case FileType::WebComic:
        actions.setFolderAsWebComicAction->setChecked(true);
        break;
    case FileType::Yonkoma:
        actions.setFolderAsYonkomaAction->setChecked(true);
        break;
    }

    QMenu menu;

    menu.addAction(actions.openContainingFolderAction);
    menu.addAction(actions.updateFolderAction);
    menu.addSeparator(); //-------------------------------
    menu.addAction(actions.rescanXMLFromCurrentFolderAction);
    menu.addSeparator(); //-------------------------------
    if (isCompleted)
        menu.addAction(actions.setFolderAsNotCompletedAction);
    else
        menu.addAction(actions.setFolderAsCompletedAction);
    menu.addSeparator(); //-------------------------------
    if (isRead)
        menu.addAction(actions.setFolderAsUnreadAction);
    else
        menu.addAction(actions.setFolderAsReadAction);
    menu.addSeparator(); //-------------------------------
    auto typeMenu = new QMenu(tr("Set type"));
    menu.addMenu(typeMenu);
    typeMenu->addAction(actions.setFolderAsNormalAction);
    typeMenu->addAction(actions.setFolderAsMangaAction);
    typeMenu->addAction(actions.setFolderAsWesternMangaAction);
    typeMenu->addAction(actions.setFolderAsWebComicAction);
    typeMenu->addAction(actions.setFolderAsYonkomaAction);

    menu.exec(foldersView->mapToGlobal(point));
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

void LibraryWindow::libraryAlreadyExists(const QString &name)
{
    QMessageBox::information(this, tr("Library name already exists"), tr("There is another library with the name '%1'.").arg(name));
}

void LibraryWindow::importLibraryPackage()
{
    importLibraryDialog->open(libraries);
}

void LibraryWindow::updateViewsOnClientSync()
{
    comicsModel->reload();
    contentViewsManager->updateCurrentComicView();
    contentViewsManager->updateContinueReadingView();
}

void LibraryWindow::updateViewsOnComicUpdateWithId(quint64 libraryId, quint64 comicId)
{
    if (libraryId == (quint64)libraries.getId(selectedLibrary->currentText())) {
        auto path = libraries.getPath(libraryId);
        if (path.isEmpty()) {
            return;
        }
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(path + "/.yacreaderlibrary");
            bool found;
            auto comic = DBHelper::loadComic(comicId, db, found);
            if (found) {
                updateViewsOnComicUpdate(libraryId, comic);
            }

            qDebug() << db.lastError();
            connectionName = db.connectionName();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }
}

void LibraryWindow::updateViewsOnComicUpdate(quint64 libraryId, const ComicDB &comic)
{
    if (libraryId == (quint64)libraries.getId(selectedLibrary->currentText())) {
        comicsModel->reload(comic);
        contentViewsManager->updateCurrentComicView();
        contentViewsManager->updateContinueReadingView();
    }
}

bool LibraryWindow::exitSearchMode()
{
    if (status != LibraryWindow::Searching)
        return false;
    searchEdit->clearText();
    clearSearchFilter();
    return true;
}
