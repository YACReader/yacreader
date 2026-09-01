#include "library_window.h"

#include "QsLog.h"
#include "add_library_dialog.h"
#include "comic_db.h"
#include "comic_management_coordinator.h"
#include "comic_model.h"
#include "comic_vine_dialog.h"
#include "comics_view.h"
#include "create_library_dialog.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "edit_shortcuts_dialog.h"
#include "export_comics_info_dialog.h"
#include "export_library_dialog.h"
#include "feature_flags.h"
#include "folder_item.h"
#include "folder_management_coordinator.h"
#include "folder_model.h"
#include "grid_comics_view.h"
#include "help_about_dialog.h"
#include "import_comics_info_dialog.h"
#include "import_library_dialog.h"
#include "import_widget.h"
#include "library_database_maintenance_coordinator.h"
#include "library_management_coordinator.h"
#include "library_repair_coordinator.h"
#include "library_search_coordinator.h"
#include "library_window_menus.h"
#include "no_libraries_widget.h"
#include "options_dialog.h"
#include "organize_files_coordinator.h"
#include "properties_dialog.h"
#include "reading_list_management_coordinator.h"
#include "reading_list_model.h"
#include "recent_visibility_coordinator.h"
#include "rename_library_dialog.h"
#include "search_syntax_dialog.h"
#include "server_config_dialog.h"
#include "shortcuts_manager.h"
#include "static.h"
#include "trayicon_controller.h"
#include "whats_new_controller.h"
#include "yacreader_content_views_manager.h"
#include "yacreader_folders_view.h"
#include "yacreader_global.h"
#include "yacreader_global_gui.h"
#include "yacreader_history_controller.h"
#include "yacreader_http_server.h"
#include "yacreader_library_list_widget.h"
#include "yacreader_main_toolbar.h"
#include "yacreader_reading_lists_view.h"
#include "yacreader_search_line_edit.h"
#include "yacreader_sidebar.h"
#include "yacreader_titled_toolbar.h"
#include "yacreader_tool_bar_stretch.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileIconProvider>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QShowEvent>
#include <QSplitter>
#include <QSqlError>
#include <QStackedWidget>
#include <QToolBar>
#include <QToolButton>
#include <QtCore>

#include <algorithm>
#include <utility>
extern YACReaderHttpServer *httpServer;

#include <KDSignalThrottler.h>

using namespace YACReader;

LibraryWindow::LibraryWindow()
    : QMainWindow(), fullscreen(false), pendingAfterLaunchTasks(false)
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

    if (startsHiddenInTray()) {
        pendingAfterLaunchTasks = true;
    } else {
        afterLaunchTasks();
    }
}

void LibraryWindow::afterLaunchTasks()
{
    if (!libraries.isEmpty()) {
        WhatsNewController whatsNewController;
        whatsNewController.showWhatsNewIfNeeded(this);
    }
}

bool LibraryWindow::startsHiddenInTray() const
{
    return settings->value(START_TO_TRAY, false).toBool() && settings->value(CLOSE_TO_TRAY, false).toBool();
}

void LibraryWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    if (pendingAfterLaunchTasks) {
        pendingAfterLaunchTasks = false;
        afterLaunchTasks();
    }
}

bool LibraryWindow::eventFilter(QObject *object, QEvent *event)
{
    if (this->isActiveWindow()) {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
            auto mouseEvent = static_cast<QMouseEvent *>(event);

            if (mouseEvent->button() == Qt::ForwardButton) {
                if (event->type() == QEvent::MouseButtonRelease)
                    actions.forwardAction->trigger();
                event->accept();
                return true;
            }

            if (mouseEvent->button() == Qt::BackButton) {
                if (event->type() == QEvent::MouseButtonRelease)
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

        auto keyCombination = keySequence[0];

        if (keyCombination.keyboardModifiers() != Qt::NoModifier) {
            return QMainWindow::eventFilter(object, event);
        }

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

void LibraryWindow::setupUI()
{
    setUnifiedTitleAndToolBarOnMac(true);

    historyController = new YACReaderHistoryController(this);

    actions.createActions(this, settings);
    doModels();

    doDialogs();
    doLayout();
    createToolBars();

    librarySearchCoordinator = new LibrarySearchCoordinator(
            foldersModel,
            foldersModelProxy,
            comicsModel,
            foldersView,
            contentViewsManager,
            [this] { clearSearchInput(false); },
            this);
    navigationController = new YACReaderNavigationController(this, contentViewsManager, librarySearchCoordinator);
    connect(librarySearchCoordinator, &LibrarySearchCoordinator::previousNavigationStateRequested, navigationController, &YACReaderNavigationController::loadPreviousStatus);
    connect(librarySearchCoordinator, &LibrarySearchCoordinator::comicActionsDisabledChanged, this, &LibraryWindow::setComicActionsDisabled);

    setupCoordinators();

    menus = new LibraryWindowMenus(
            this,
            actions,
            selectedLibrary,
            foldersView,
            contentViewsManager,
            foldersModel,
            foldersModelProxy,
            listsModel,
            folderManagementCoordinator,
            comicManagementCoordinator,
            organizeFilesCoordinator,
            [this] { return getSelectedComics(); },
            [this] { return static_cast<qulonglong>(libraries.getId(selectedLibrary->currentText())); },
            [this] { return currentPath(); },
            [this]() -> const Theme & { return theme; });
    menus->setupMenus();
    contentViewsManager->setLibraryWindowMenus(menus);
    connect(menus, &LibraryWindowMenus::currentLibraryTypeChangeRequested, this, &LibraryWindow::setCurrentLibraryAs);
    connect(menus, &LibraryWindowMenus::folderUpdateRequested, libraryManagementCoordinator, &LibraryManagementCoordinator::updateFolder);
    connect(menus, &LibraryWindowMenus::folderXmlRescanRequested, libraryManagementCoordinator, &LibraryManagementCoordinator::rescanFolderForXMLInfo);

    createConnections();

    setWindowTitle(tr("YACReader Library"));

    setMinimumSize(800, 480);

    // restore
    if (settings->contains(MAIN_WINDOW_GEOMETRY)) {
        restoreGeometry(settings->value(MAIN_WINDOW_GEOMETRY).toByteArray());
        restoreState(settings->value(MAIN_WINDOW_STATE).toByteArray());
        // Guard against the window landing off-screen when a monitor is unplugged
        // between sessions. Qt 6 tries to remap the geometry to the primary screen
        // when the saved screen is gone, but the result can still be off-screen.
        const QRect restored = geometry();
        const auto availableScreens = QApplication::screens();
        const bool onScreen = std::any_of(
                availableScreens.cbegin(), availableScreens.cend(),
                [&restored](QScreen *s) { return s->availableGeometry().intersects(restored); });
        if (!onScreen) {
            const QRect avail = QApplication::primaryScreen()->availableGeometry();
            setGeometry(QRect(avail.center() - QPoint(width() / 2, height() / 2), size()));
        }
    } else if (startsHiddenInTray()) {
        setWindowState(windowState() | Qt::WindowMaximized);
    } else {
        // if(settings->value(USE_OPEN_GL).toBool() == false)
        showMaximized();
    }

    trayIconController = new TrayIconController(settings, this);

    initTheme(this);
}

void LibraryWindow::applyTheme(const Theme &theme)
{
    editInfoToolBar->setStyleSheet(theme.comicsViewToolbar.toolbarQSS);
    // Both menu buttons carry their own icon, because neither has a default action
    // to take one from. See createMenuToolButton().
    if (organizeToolButton != nullptr)
        organizeToolButton->setIcon(theme.comicsViewToolbar.organizeIcon);
    if (setTypeToolButton != nullptr)
        setTypeToolButton->setIcon(theme.comicsViewToolbar.setAsNormalIcon);
    mainSplitter->setStyleSheet(theme.contentSplitter.horizontalSplitterQSS);

    // Update main toolbar and comics view toolbar icons
    actions.updateTheme(theme);
}

void LibraryWindow::doLayout()
{
    // LAYOUT ELEMENTS------------------------------------------------------------
    mainSplitter = new QSplitter(Qt::Horizontal); // spliter principal
    auto sHorizontal = mainSplitter; // Keep local alias for existing code

    // TOOLBARS-------------------------------------------------------------------
    //---------------------------------------------------------------------------
    editInfoToolBar = new QToolBar();

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
    // foldersModelProxy->setSourceModel(foldersModel);
    // comics
    comicsModel = new ComicModel(this);
    // lists
    listsModel = new ReadingListModel(this);
    listsModelProxy = new ReadingListModelProxy(this);
}

void LibraryWindow::setupCoordinators()
{
    recentVisibilityCoordinator = new RecentVisibilityCoordinator(settings, foldersModel, comicsModel);
    organizeFilesCoordinator = new OrganizeFilesCoordinator(
            settings,
            this,
            comicsModel,
            foldersModel,
            [this] { return getSelectedComics(); },
            [this] {
                // A search shows comics from the whole library while the folder
                // tree keeps its old selection. That folder says nothing about the
                // results, so the base is the library root, forced.
                if (librarySearchCoordinator != nullptr && librarySearchCoordinator->isSearching())
                    return QModelIndex();
                return getCurrentFolderIndex();
            },
            [this] {
                const auto libraryName = selectedLibrary->currentText();
                return OrganizeFilesCoordinator::LibraryContext { static_cast<qulonglong>(libraries.getId(libraryName)), libraries.getPath(libraryName) };
            });
    connect(organizeFilesCoordinator, &OrganizeFilesCoordinator::libraryContentChanged, this, &LibraryWindow::reloadCurrentLibrary);
    comicManagementCoordinator = new ComicManagementCoordinator(
            this,
            settings,
            comicsModel,
            foldersModel,
            foldersModelProxy,
            propertiesDialog,
            comicVineDialog,
            [this] { return getSelectedComics(); },
            [this] {
                if (listsView->selectionModel() == nullptr || listsView->selectionModel()->selectedRows().isEmpty())
                    return QModelIndex();
                return listsModelProxy->mapToSource(listsView->currentIndex());
            },
            [this] { return getCurrentFolderIndex(); },
            [this] { return contentViewsManager->comicsView->currentIndex(); },
            [this] { return !importedCovers; },
            [this] { return static_cast<qulonglong>(libraries.getId(selectedLibrary->currentText())); },
            [this] { return currentPath(); });
    contentViewsManager->setComicManagementCoordinator(comicManagementCoordinator);
    connect(comicManagementCoordinator, &ComicManagementCoordinator::currentComicViewUpdateRequested, contentViewsManager, &YACReaderContentViewsManager::updateCurrentComicView);
    connect(comicManagementCoordinator, &ComicManagementCoordinator::currentSourceRefreshStarted, navigationController, &YACReaderNavigationController::beginCurrentSourceRefresh);
    connect(comicManagementCoordinator, &ComicManagementCoordinator::currentSourceRefreshAccepted, navigationController, &YACReaderNavigationController::refreshCurrentSource);
    connect(comicManagementCoordinator, &ComicManagementCoordinator::currentSourceRefreshCancelled, navigationController, &YACReaderNavigationController::cancelCurrentSourceRefresh);
    connect(comicManagementCoordinator, &ComicManagementCoordinator::comicNumbersAssigned, this, [this](qint64 editedComicId) {
        navigationController->loadFolderContent(foldersModelProxy->mapToSource(foldersView->currentIndex()));

        const auto editedComic = comicsModel->getIndexFromId(editedComicId);
        if (editedComic.isValid()) {
            contentViewsManager->comicsView->scrollTo(editedComic, QAbstractItemView::PositionAtCenter);
            contentViewsManager->comicsView->setCurrentIndex(editedComic);
        }
    });
    connect(comicManagementCoordinator, &ComicManagementCoordinator::comicDeletionFinished, this, &LibraryWindow::checkEmptyFolder);
    connect(comicManagementCoordinator, &ComicManagementCoordinator::rootContinueReadingReloadRequested, navigationController, &YACReaderNavigationController::reloadRootContinueReading);
    readingListManagementCoordinator = new ReadingListManagementCoordinator(
            this,
            listsModel,
            comicsModel,
            [this] {
                if (listsView->selectionModel() == nullptr)
                    return QModelIndex();
                const auto selectedLists = listsView->selectionModel()->selectedIndexes();
                return selectedLists.isEmpty() ? QModelIndex() : listsModelProxy->mapToSource(selectedLists.constFirst());
            });
    connect(readingListManagementCoordinator, &ReadingListManagementCoordinator::currentListReselectionRequested, navigationController, &YACReaderNavigationController::reselectCurrentList);
    folderManagementCoordinator = new FolderManagementCoordinator(
            foldersModel,
            this,
            [this] { return foldersModelProxy->mapToSource(foldersView->currentIndex()); },
            [this] { return getCurrentFolderIndex(); },
            [this] { return currentPath(); });
    connect(folderManagementCoordinator, &FolderManagementCoordinator::folderRenamed, navigationController, &YACReaderNavigationController::refreshCurrentSource);
    connect(folderManagementCoordinator, &FolderManagementCoordinator::folderCreationStarted, this, [this] { librarySearchCoordinator->exitSearchMode(); });
    connect(folderManagementCoordinator, &FolderManagementCoordinator::folderNavigationRequested, this, [this](const QModelIndex &folder) {
        foldersView->setCurrentIndex(foldersModelProxy->mapFromSource(folder));
        navigationController->loadFolderContent(folder);
        historyController->updateHistory(YACReaderLibrarySourceContainer(folder, YACReaderLibrarySourceContainer::Folder));
    });
    connect(folderManagementCoordinator, &FolderManagementCoordinator::folderAboutToBeDeleted, this, [this](const QModelIndex &parentFolder) {
        // The unified grid observes the main folder model directly. Move away
        // from the folder before removing its model index so the content view
        // never retains the index being deleted.
        if (parentFolder.isValid())
            foldersView->setCurrentIndex(foldersModelProxy->mapFromSource(parentFolder));
        else
            setRootIndex();
    });
    connect(folderManagementCoordinator, &FolderManagementCoordinator::folderDeletionFinished, navigationController, &YACReaderNavigationController::reselectCurrentFolder);
    connect(contentViewsManager->gridView(), &ComicsView::customFolderCoverRequested, folderManagementCoordinator, qOverload<qulonglong, const QString &>(&FolderManagementCoordinator::setCustomCover));
    libraryDatabaseMaintenanceCoordinator = new LibraryDatabaseMaintenanceCoordinator(
            libraries,
            this,
            [this] { return selectedLibrary->currentText(); });
    connect(libraryDatabaseMaintenanceCoordinator, &LibraryDatabaseMaintenanceCoordinator::backupAvailabilityChanged, actions.backupLibraryAction, &QAction::setEnabled);
    connect(libraryDatabaseMaintenanceCoordinator, &LibraryDatabaseMaintenanceCoordinator::maintenanceStarted, this, [this] {
        contentViewsManager->comicsView->setModel(nullptr);
        foldersView->setModel(nullptr);
        listsView->setModel(nullptr);
        actions.disableAllActions();
    });
    connect(libraryDatabaseMaintenanceCoordinator, &LibraryDatabaseMaintenanceCoordinator::invalidDatabaseRestoreCancelled, this, [this] {
        actions.renameLibraryAction->setEnabled(true);
        actions.removeLibraryAction->setEnabled(true);
        actions.restoreLibraryAction->setEnabled(true);
    });
    connect(libraryDatabaseMaintenanceCoordinator, &LibraryDatabaseMaintenanceCoordinator::databaseUnavailableAfterRestore, this, [this] {
        actions.restoreLibraryAction->setEnabled(true);
        actions.removeLibraryAction->setEnabled(true);
    });
    connect(libraryDatabaseMaintenanceCoordinator, &LibraryDatabaseMaintenanceCoordinator::databaseSalvageFailed, this, [this] {
        actions.restoreLibraryAction->setEnabled(true);
    });
    libraryRepairCoordinator = new LibraryRepairCoordinator(
            settings,
            libraries,
            this,
            [this] { return selectedLibrary->currentText(); });
    connect(libraryRepairCoordinator, &LibraryRepairCoordinator::repairStarted, importWidget, &ImportWidget::setRepairLook);
    connect(libraryRepairCoordinator, &LibraryRepairCoordinator::repairStarted, this, &LibraryWindow::showImportingWidget);
    connect(libraryRepairCoordinator, &LibraryRepairCoordinator::repairFinished, this, &LibraryWindow::showRootWidget);
    connect(libraryRepairCoordinator, &LibraryRepairCoordinator::repairFinished, this, &LibraryWindow::reloadCurrentLibrary);
    connect(libraryRepairCoordinator, &LibraryRepairCoordinator::comicProcessed, importWidget, &ImportWidget::newComic);
    libraryManagementCoordinator = new LibraryManagementCoordinator(
            settings,
            libraries,
            this,
            createLibraryDialog,
            addLibraryDialog,
            exportLibraryDialog,
            importLibraryDialog,
            foldersModel,
            [this] { return selectedLibrary->currentText(); },
            [this] { return getCurrentFolderIndex(); },
            tr("Library info"));
    connect(noLibrariesWidget, &NoLibrariesWidget::createNewLibrary, libraryManagementCoordinator, &LibraryManagementCoordinator::showCreateLibraryDialog);
    connect(noLibrariesWidget, &NoLibrariesWidget::addExistingLibrary, libraryManagementCoordinator, &LibraryManagementCoordinator::showAddLibraryDialog);
    connect(libraryDatabaseMaintenanceCoordinator, &LibraryDatabaseMaintenanceCoordinator::libraryReloadRequested, libraryManagementCoordinator, &LibraryManagementCoordinator::loadLibrary);
    connect(comicManagementCoordinator, &ComicManagementCoordinator::importRequested, libraryManagementCoordinator, [this](qulonglong folderId) {
        libraryManagementCoordinator->updateFolder(foldersModel->getIndexFromFolderId(folderId));
    });
    connect(contentViewsManager->gridView(), &GridComicsView::openLibraryFolderRequested, libraryManagementCoordinator, &LibraryManagementCoordinator::openCurrentLibraryFolder);
    connect(libraryDatabaseMaintenanceCoordinator, &LibraryDatabaseMaintenanceCoordinator::libraryUpdateRequested, libraryManagementCoordinator, &LibraryManagementCoordinator::updateCurrentLibrary);
    connect(libraryRepairCoordinator, &LibraryRepairCoordinator::databaseRecoveryRequested, libraryDatabaseMaintenanceCoordinator, [coordinator = libraryDatabaseMaintenanceCoordinator, restoreAction = actions.restoreLibraryAction](const QString &libraryName) {
        coordinator->offerDatabaseRecovery(libraryName, restoreAction->text());
    });
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::loadStarted, this, [this] {
        historyController->clear();
        showRootWidget();
    });
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::noLibrariesRequested, this, [this] {
        actions.disableAllActions();
        showNoLibrariesWidget();
    });
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::libraryReady, this, &LibraryWindow::applyLoadedLibrary);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::libraryManagementOnlyRequested, this, &LibraryWindow::showLibraryManagementOnly);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::databaseRecoveryRequested, libraryDatabaseMaintenanceCoordinator, [coordinator = libraryDatabaseMaintenanceCoordinator, restoreAction = actions.restoreLibraryAction](const QString &libraryName) {
        coordinator->offerDatabaseRecovery(libraryName, restoreAction->text());
    });
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::upgradeStarted, importWidget, &ImportWidget::setUpgradeLook);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::upgradeStarted, this, &LibraryWindow::showImportingWidget);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::creationStarted, importWidget, &ImportWidget::setImportLook);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::creationStarted, this, &LibraryWindow::showImportingWidget);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::updateStarted, importWidget, &ImportWidget::setUpdateLook);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::updateStarted, this, &LibraryWindow::showImportingWidget);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::operationUiResetRequested, this, &LibraryWindow::showRootWidget);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::operationFinished, this, &LibraryWindow::showRootWidget);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::currentLibraryReloadRequested, this, &LibraryWindow::reloadCurrentLibrary);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::libraryAdded, this, &LibraryWindow::addLibraryToSelector);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::libraryRemoved, this, &LibraryWindow::handleLibraryRemoved);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::libraryRenamed, this, [this](const QString &oldName, const QString &newName) {
        if (newName == oldName)
            return;

        selectedLibrary->renameCurrentLibrary(newName);
#ifndef Y_MAC_UI
        if (!foldersModelProxy->mapToSource(foldersView->currentIndex()).isValid())
            libraryToolBar->setCurrentFolderName(newName);
#endif
    });
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::folderUpdateFinished, this, [this](qulonglong folderId) {
        reloadAfterCopyMove(foldersModel->getIndexFromFolderId(folderId));
    });
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::comicAdded, importWidget, &ImportWidget::newComic);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::xmlScanStarted, importWidget, &ImportWidget::setXMLScanLook);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::xmlScanStarted, this, &LibraryWindow::showImportingWidget);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::xmlScanFinished, this, &LibraryWindow::showRootWidget);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::xmlScanFinished, this, &LibraryWindow::reloadCurrentFolderComicsContent);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::xmlComicScanned, importWidget, &ImportWidget::newComic);
    connect(libraryManagementCoordinator, &LibraryManagementCoordinator::packageFailed, this, [this](const QString &error) {
        QMessageBox::critical(this, tr("Package operation failed"), error.isEmpty() ? tr("The covers package operation could not be completed.") : error);
    });

    auto canStartUpdateProvider = [this]() {
        return comicVineDialog->isVisible() == false &&
                propertiesDialog->isVisible() == false;
    };
    librariesUpdateCoordinator = new LibrariesUpdateCoordinator(settings, libraries, canStartUpdateProvider, this);

    // Allow HTTP requests (e.g. the WebUI "Update now" button) to trigger updates.
    Static::librariesUpdateCoordinator = librariesUpdateCoordinator;

    connect(librariesUpdateCoordinator, &LibrariesUpdateCoordinator::updateStarted, sideBar->librariesTitle, &YACReaderTitledToolBar::showBusyIndicator);
    connect(librariesUpdateCoordinator, &LibrariesUpdateCoordinator::updateEnded, sideBar->librariesTitle, &YACReaderTitledToolBar::hideBusyIndicator);

    connect(librariesUpdateCoordinator, &LibrariesUpdateCoordinator::updateStarted, this, [=, this]() {
        actions.disableAllActions();
    });
    connect(librariesUpdateCoordinator, &LibrariesUpdateCoordinator::updateEnded, this, &LibraryWindow::reloadCurrentLibrary);

    librariesUpdateCoordinator->init();

    connect(sideBar->librariesTitle, &YACReaderTitledToolBar::cancelOperationRequested, librariesUpdateCoordinator, &LibrariesUpdateCoordinator::cancel);
}

bool LibraryWindow::hasLoadedLibraryModels() const
{
    return foldersView->model() == foldersModelProxy &&
            listsView->model() == listsModelProxy &&
            foldersModelProxy->sourceModel() == foldersModel &&
            listsModelProxy->sourceModel() == listsModel;
}

namespace {

QToolButton *createMenuToolButton(const QList<QAction *> &entries, const QString &toolTip)
{
    Q_ASSERT(!entries.isEmpty());

    auto button = new QToolButton();
    for (auto *entry : entries)
        button->addAction(entry);

    button->setPopupMode(QToolButton::InstantPopup);
    button->setToolTip(toolTip);

    auto *first = entries.first();
    const auto followFirstEntry = [button, first] { button->setEnabled(first->isEnabled()); };
    QObject::connect(first, &QAction::changed, button, followFirstEntry);
    followFirstEntry();

    return button;
}

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

    auto *searchMenu = createSearchMenu();
#ifdef Y_MAC_UI
    libraryToolBar->setSearchMenu(searchMenu);
#else
    searchEdit->setSearchMenu(searchMenu);
#endif

    editInfoToolBar->setIconSize(QSize(18, 18));
    editInfoToolBar->addAction(actions.openComicAction);
    editInfoToolBar->addSeparator();
    editInfoToolBar->addAction(actions.editSelectedComicsAction);
    if (YACReader::FeatureFlags::organizeFiles) {
        organizeToolButton = createMenuToolButton({ actions.renameComicsFilesAction, actions.organizeComicsFilesAction },
                                                  tr("Rename or organize files"));
        editInfoToolBar->addWidget(organizeToolButton);
    }
    editInfoToolBar->addAction(actions.getInfoAction);
    editInfoToolBar->addAction(actions.asignOrderAction);

    editInfoToolBar->addSeparator();

    editInfoToolBar->addAction(actions.selectAllComicsAction);

    editInfoToolBar->addSeparator();

    editInfoToolBar->addAction(actions.setAsReadAction);
    editInfoToolBar->addAction(actions.setAsNonReadAction);

    editInfoToolBar->addAction(actions.showHideMarksAction);

    editInfoToolBar->addSeparator();

    setTypeToolButton = createMenuToolButton({ actions.setNormalAction, actions.setMangaAction,
                                               actions.setWesternMangaAction, actions.setWebComicAction,
                                               actions.setYonkomaAction },
                                             tr("Set the type of the selected comics"));
    editInfoToolBar->addWidget(setTypeToolButton);

    editInfoToolBar->addSeparator();

    editInfoToolBar->addAction(actions.deleteComicsAction);

    comicToolbarEntries = editInfoToolBar->actions();

    auto toolBarStretch = new YACReaderToolBarStretch(this);
    comicToolbarEndAnchor = editInfoToolBar->addWidget(toolBarStretch);

    editInfoToolBar->addAction(actions.toogleShowRecentIndicatorAction);

    contentViewsManager->comicsView->setToolBar(editInfoToolBar);
}

QMenu *LibraryWindow::createSearchMenu()
{
    auto *menu = new QMenu(tr("Search filters"), this);
    menu->setMinimumWidth(190);

    auto addFilter = [this, menu](const QString &label, const QString &query) {
        auto *action = menu->addAction(label);
        connect(action, &QAction::triggered, this, [this, query] {
            applySearchQuery(query);
        });
    };

    addFilter(tr("Unread"), QStringLiteral("read:false"));
    addFilter(
            tr("In progress"),
            QStringLiteral("hasBeenOpened:true AND read:false"));
    addFilter(tr("Highly rated"), QStringLiteral("rating>=4"));

    auto *recentlyAdded = menu->addAction(tr("Recently added"));
    connect(recentlyAdded, &QAction::triggered, this, [this] {
        const int days = settings->value(NUM_DAYS_TO_CONSIDER_RECENT, 1).toInt();
        applySearchQuery(QStringLiteral("added>%1").arg(days));
    });

    menu->addSeparator();
    auto *syntaxAction = menu->addAction(tr("Search syntax…"));
    connect(syntaxAction, &QAction::triggered, this, &LibraryWindow::showSearchSyntax);

    return menu;
}

void LibraryWindow::applySearchQuery(const QString &query)
{
#ifdef Y_MAC_UI
    libraryToolBar->setSearchText(query);
    libraryToolBar->focusSearch();
#else
    searchEdit->setText(query);
    searchEdit->setFocus(Qt::ShortcutFocusReason);
#endif
}

void LibraryWindow::setSearchInputEnabled(bool enabled)
{
#ifdef Y_MAC_UI
    libraryToolBar->setSearchEnabled(enabled);
#else
    searchEdit->setEnabled(enabled);
#endif
}

void LibraryWindow::clearSearchInput(bool notify)
{
#ifdef Y_MAC_UI
    libraryToolBar->clearSearchText(notify);
#else
    if (notify)
        searchEdit->clear();
    else
        searchEdit->clearText();
#endif
}

void LibraryWindow::focusSearchInput()
{
#ifdef Y_MAC_UI
    libraryToolBar->focusSearch();
#else
    searchEdit->setFocus(Qt::ShortcutFocusReason);
#endif
}

QString LibraryWindow::searchText() const
{
#ifdef Y_MAC_UI
    return libraryToolBar->searchText();
#else
    return searchEdit->text();
#endif
}

void LibraryWindow::showSearchSyntax()
{
    auto *dialog = new SearchSyntaxDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->open();
}
void LibraryWindow::createConnections()
{
    actions.createConnections(
            historyController,
            navigationController,
            this,
            had,
            contentViewsManager,
            editShortcutsDialog,
            foldersView,
            optionsDialog,
            serverConfigDialog,
            recentVisibilityCoordinator,
            comicManagementCoordinator,
            readingListManagementCoordinator,
            folderManagementCoordinator,
            organizeFilesCoordinator,
            libraryManagementCoordinator,
            libraryDatabaseMaintenanceCoordinator,
            libraryRepairCoordinator,
            renameLibraryDialog);
    connect(actions.focusSearchLineAction, &QAction::triggered, this, &LibraryWindow::focusSearchInput);

    connect(importComicsInfoDialog, &QDialog::finished, this, &LibraryWindow::reloadCurrentLibrary);

    // new import widget
    connect(importWidget, &ImportWidget::stop, libraryManagementCoordinator, &LibraryManagementCoordinator::stop);
    connect(importWidget, &ImportWidget::stop, libraryRepairCoordinator, &LibraryRepairCoordinator::stop);

    // load library when selected library changes
    connect(selectedLibrary, &YACReaderLibraryListWidget::currentIndexChanged, libraryManagementCoordinator, &LibraryManagementCoordinator::loadLibrary);

    // navigations between view modes (tree,list and flow)
    // TODO connect(foldersView, SIGNAL(pressed(QModelIndex)), this, SLOT(updateFoldersViewConextMenu(QModelIndex)));
    // connect(foldersView, SIGNAL(clicked(QModelIndex)), this, SLOT(loadCovers(QModelIndex)));

    // drops in folders view
    connect(foldersView, QOverload<QList<QPair<QString, QString>>, QModelIndex>::of(&YACReaderFoldersView::copyComicsToFolder),
            comicManagementCoordinator, &ComicManagementCoordinator::copyAndImportComicsToFolder);
    connect(foldersView, QOverload<QList<QPair<QString, QString>>, QModelIndex>::of(&YACReaderFoldersView::moveComicsToFolder),
            comicManagementCoordinator, &ComicManagementCoordinator::moveAndImportComicsToFolder);

    connect(optionsDialog, &YACReaderOptionsDialog::optionsChanged, this, &LibraryWindow::reloadOptions);
    connect(optionsDialog, &YACReaderOptionsDialog::editShortcuts, editShortcutsDialog, &QWidget::show);

    auto searchDebouncer = new KDToolBox::KDStringSignalDebouncer(this);
    searchDebouncer->setTimeout(400);

// Search filter
#ifdef Y_MAC_UI
    connect(libraryToolBar, &YACReaderMacOSXToolbar::filterChanged, searchDebouncer, &KDToolBox::KDStringSignalDebouncer::throttle);
#else
    connect(searchEdit, &YACReaderSearchLineEdit::filterChanged, searchDebouncer, &KDToolBox::KDStringSignalDebouncer::throttle);
#endif
    connect(searchDebouncer, &KDToolBox::KDStringSignalDebouncer::triggered, librarySearchCoordinator, &LibrarySearchCoordinator::search);
}

void LibraryWindow::setCurrentLibraryAs(FileType fileType)
{
    foldersModel->updateTreeType(fileType);
}

void LibraryWindow::applyLoadedLibrary(const QString &libraryDataPath, bool readOnly)
{
    foldersModel->setupModelData(libraryDataPath);
    foldersModelProxy->setSourceModel(foldersModel);
    foldersView->setModel(foldersModelProxy);
    foldersView->setCurrentIndex(QModelIndex()); // By default this can return an arbitrary index.

    listsModel->setupReadingListsData(libraryDataPath);
    listsModelProxy->setSourceModel(listsModel);
    listsView->setModel(listsModelProxy);

    actions.disableFoldersActions(foldersModel->rowCount(QModelIndex()) == 0);
    actions.disableLibrariesActions(false);

    if (readOnly) {
        actions.updateLibraryAction->setDisabled(true);
        actions.repairLibraryAction->setDisabled(true);
        actions.openContainingFolderAction->setDisabled(true);
        actions.rescanLibraryForXMLInfoAction->setDisabled(true);

        setComicActionsDisabled(true);
#ifndef Q_OS_MACOS
        actions.toggleFullScreenAction->setEnabled(true);
#endif
    }
    importedCovers = readOnly;

    setRootIndex();
    clearSearchInput(true);
}

void LibraryWindow::showLibraryManagementOnly()
{
    contentViewsManager->comicsView->setModel(nullptr);
    foldersView->setModel(nullptr);
    listsView->setModel(nullptr);
    actions.disableAllActions();
    actions.renameLibraryAction->setEnabled(true);
    actions.removeLibraryAction->setEnabled(true);
    actions.restoreLibraryAction->setEnabled(true);
}

void LibraryWindow::loadCoversFromCurrentModel()
{
    contentViewsManager->comicsView->setModel(comicsModel);
}

void LibraryWindow::reloadCurrentFolderComicsContent()
{
    navigationController->loadFolderContent(getCurrentFolderIndex());

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

        navigationController->refreshCurrentSource();
    }

    enableNeededActions();
}

QModelIndex LibraryWindow::getCurrentFolderIndex()
{
    if (!hasLoadedLibraryModels())
        return QModelIndex();

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
        setComicActionsDisabled(false);

    actions.disableLibrariesActions(false);
}

void LibraryWindow::setComicActionsDisabled(bool disabled)
{
    if (!disabled && librariesUpdateCoordinator->isRunning()) {
        setComicActionsDisabled(true);
        return;
    }

    actions.setComicActionsDisabled(disabled);
    setComicToolbarEntriesVisible(comicsModel != nullptr && comicsModel->rowCount() > 0);
}

void LibraryWindow::setComicToolbarEntriesVisible(bool visible)
{
    if (editInfoToolBar == nullptr || comicToolbarEndAnchor == nullptr)
        return;

    const auto currentActions = editInfoToolBar->actions();
    for (auto *action : std::as_const(comicToolbarEntries)) {
        if (visible && !currentActions.contains(action))
            editInfoToolBar->insertAction(comicToolbarEndAnchor, action);
        else if (!visible && currentActions.contains(action))
            editInfoToolBar->removeAction(action);
    }
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

// this methods is only using after deleting comics
// TODO broken window :)
void LibraryWindow::checkEmptyFolder()
{
    if (comicsModel->rowCount() > 0 && !importedCovers) {
        setComicActionsDisabled(false);
    } else {
        setComicActionsDisabled(true);
#ifndef Q_OS_MACOS
        if (comicsModel->rowCount() > 0)
            actions.toggleFullScreenAction->setEnabled(true);
#endif
        if (comicsModel->rowCount() == 0)
            navigationController->reselectCurrentFolder();
    }
}

void LibraryWindow::reloadCurrentLibrary()
{
    if (!hasLoadedLibraryModels())
        return;

    foldersModel->reload();
    navigationController->refreshCurrentSource();

    enableNeededActions();
}

void LibraryWindow::loadLibraries()
{
    const auto storedLibraries = libraryManagementCoordinator->loadLibraries();
    for (const auto &[name, path] : storedLibraries)
        selectedLibrary->addItem(name, path);
}

void LibraryWindow::addLibraryToSelector(const QString &libraryName, const QString &libraryPath)
{
    const QSignalBlocker blocker(selectedLibrary);
    selectedLibrary->addItem(libraryName, libraryPath);
    selectedLibrary->setCurrentIndex(selectedLibrary->findText(libraryName));
    addLibraryDialog->close();
    libraryManagementCoordinator->loadLibrary(libraryName);
}

void LibraryWindow::handleLibraryRemoved(const QString &libraryName, bool librariesEmpty)
{
    const auto index = selectedLibrary->findText(libraryName);
    if (index >= 0)
        selectedLibrary->removeItem(index);

    if (!librariesEmpty)
        return;

    contentViewsManager->comicsView->setModel(nullptr);
    foldersView->setModel(nullptr);
    listsView->setModel(nullptr);
    actions.disableAllActions();
    showNoLibrariesWidget();
}

void LibraryWindow::setRootIndex()
{
    if (!libraries.isEmpty()) {
        QString path = LibraryPaths::libraryDataPath(libraries.getPath(selectedLibrary->currentText()));
        QDir d; // TODO change this by static methods (utils class?? with delTree for example)
        if (d.exists(path)) {
            navigationController->selectedFolder(QModelIndex());
        } else {
            contentViewsManager->comicsView->setModel(NULL);
        }

        auto selectionModel = foldersView->selectionModel();
        if (selectionModel != nullptr)
            selectionModel->clear();
    }
}

void LibraryWindow::toggleFullScreen()
{
    fullscreen ? toNormal() : toFullScreen();
    fullscreen = !fullscreen;
}

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

void LibraryWindow::showExportComicsInfo()
{
    exportComicsInfoDialog->source = LibraryPaths::libraryDatabasePath(currentPath());
    exportComicsInfoDialog->open();
}

void LibraryWindow::showImportComicsInfo()
{
    importComicsInfoDialog->dest = LibraryPaths::libraryDatabasePath(currentPath());
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

    libraryManagementCoordinator->stop();
    librariesUpdateCoordinator->stop();
    libraryRepairCoordinator->stop();

    settings->setValue(MAIN_WINDOW_GEOMETRY, saveGeometry());
    settings->setValue(MAIN_WINDOW_STATE, saveState());

    contentViewsManager->prepareToClose();
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
    setSearchInputEnabled(false);
    mainWidget->setCurrentIndex(1);
}

void LibraryWindow::showRootWidget()
{
#ifndef Y_MAC_UI
    libraryToolBar->setDisabled(false);
#endif
    setSearchInputEnabled(true);
    mainWidget->setCurrentIndex(0);
}

void LibraryWindow::showImportingWidget()
{
    actions.disableAllActions();
    importWidget->clear();
#ifndef Y_MAC_UI
    libraryToolBar->setDisabled(true);
#endif
    setSearchInputEnabled(false);
    mainWidget->setCurrentIndex(2);
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

void LibraryWindow::updateViewsOnClientSync()
{
    comicsModel->reload();
    contentViewsManager->updateCurrentComicView();
    navigationController->reloadRootContinueReading();
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
            QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(path));
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
        navigationController->reloadRootContinueReading();
    }
}
