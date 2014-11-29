#ifndef __LIBRARYWINDOW_H
#define __LIBRARYWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QModelIndex>
#include <QFileInfo>
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include "yacreader_navigation_controller.h"

#ifdef Q_OS_MAC
    #include "yacreader_macosx_toolbar.h"
#endif

class QTreeView;
class QDirModel;
class QAction;
class QToolBar;
class QComboBox;
class QThread;
class QStackedWidget;
class YACReaderSearchLineEdit;
class CreateLibraryDialog;
class ExportLibraryDialog;
class ImportLibraryDialog;
class ExportComicsInfoDialog;
class ImportComicsInfoDialog;
class AddLibraryDialog;
class LibraryCreator;
class HelpAboutDialog;
class RenameLibraryDialog;
class PropertiesDialog;
class PackageManager;
class QCheckBox;
class QPushButton;
class ComicModel;
class QSplitter;
class FolderModel;
class FolderModelProxy;
class QItemSelectionModel;
class QString;
class QLabel;
class NoLibrariesWidget;
class OptionsDialog;
class ServerConfigDialog;
class QCloseEvent;
class ImportWidget;
class QSettings;
class LibraryItem;
class YACReaderTableView;
class YACReaderSideBar;
class YACReaderLibraryListWidget;
class YACReaderFoldersView;
class YACReaderMainToolBar;
class ComicVineDialog;
class ComicsView;
class ClassicComicsView;
class GridComicsView;
class ComicsViewTransition;
class EmptyFolderWidget;
class NoSearchResultsWidget;
class EditShortcutsDialog;
class ComicFilesManager;
class QProgressDialog;
class ReadingListModel;
class ReadingListModelProxy;
class YACReaderReadingListsView;
class YACReaderHistoryController;
class EmptyLabelWidget;
class EmptySpecialListWidget;
class EmptyReadingListWidget;

#include "comic_db.h"

using namespace YACReader;

class LibraryWindow : public QMainWindow
{
    friend class YACReaderNavigationController;

	Q_OBJECT
private:
	YACReaderSideBar * sideBar;

	CreateLibraryDialog * createLibraryDialog;
	ExportLibraryDialog * exportLibraryDialog;
	ImportLibraryDialog * importLibraryDialog;
	ExportComicsInfoDialog * exportComicsInfoDialog;
	ImportComicsInfoDialog * importComicsInfoDialog;
	AddLibraryDialog * addLibraryDialog;
	LibraryCreator * libraryCreator;
	HelpAboutDialog * had;
	RenameLibraryDialog * renameLibraryDialog;
	PropertiesDialog * propertiesDialog;
	ComicVineDialog * comicVineDialog;
    EditShortcutsDialog * editShortcutsDialog;
	//YACReaderSocialDialog * socialDialog;
	bool fullscreen;
	bool importedCovers; //if true, the library is read only (not updates,open comic or properties)
	bool fromMaximized;

	PackageManager * packageManager;

	QSize slideSizeW;
	QSize slideSizeF;
	//search filter
#ifdef Q_OS_MAC
    YACReaderMacOSXSearchLineEdit * searchEdit;
#else
    YACReaderSearchLineEdit * searchEdit;
#endif

	QString previousFilter;
	QCheckBox * includeComicsCheckBox;
	//-------------

    YACReaderNavigationController * navigationController;

    ComicsView * comicsView;
    ClassicComicsView * classicComicsView;
    GridComicsView * gridComicsView;
    QStackedWidget * comicsViewStack;
    ComicsViewTransition * comicsViewTransition;
    EmptyFolderWidget * emptyFolderWidget;
    EmptyLabelWidget * emptyLabelWidget;
    EmptySpecialListWidget * emptySpecialList;
    EmptyReadingListWidget * emptyReadingList;
    NoSearchResultsWidget * noSearchResultsWidget;

    YACReaderFoldersView * foldersView;
    YACReaderReadingListsView * listsView;
	YACReaderLibraryListWidget * selectedLibrary;
    FolderModel * foldersModel;
    FolderModelProxy * foldersModelProxy;
    ComicModel * comicsModel;
    ReadingListModel * listsModel;
    ReadingListModelProxy * listsModelProxy;
	//QStringList paths;
	YACReaderLibraries libraries;

	QStackedWidget * mainWidget;
	NoLibrariesWidget * noLibrariesWidget;
	ImportWidget * importWidget;

	bool fetching;

	int i;
	
	QAction  * backAction;
	QAction  * forwardAction;

	QAction * openComicAction;
	QAction * createLibraryAction;
	QAction * openLibraryAction;
	
    QAction * exportComicsInfoAction;
    QAction * importComicsInfoAction;

	QAction * exportLibraryAction;
	QAction * importLibraryAction;

	QAction * updateLibraryAction;
	QAction * removeLibraryAction;
	QAction * helpAboutAction;
	QAction * renameLibraryAction;
#ifndef Q_OS_MAC
	QAction * toggleFullScreenAction;
#endif
	QAction * optionsAction;
	QAction * serverConfigAction;
    QAction * toggleComicsViewAction;
	//QAction * socialAction;

	//tree actions
    QAction * addFolderAction;
    QAction * deleteFolderAction;
    //--
	QAction * setRootIndexAction;
	QAction * expandAllNodesAction;
	QAction * colapseAllNodesAction;

    QAction * openContainingFolderAction;
    //--
    QAction * setFolderAsNotCompletedAction;
    QAction * setFolderAsCompletedAction;
    //--
    QAction * setFolderAsReadAction;
    QAction * setFolderAsUnreadAction;

	QAction * openContainingFolderComicAction;
	QAction * setAsReadAction;
	QAction * setAsNonReadAction;
	//QAction * setAllAsReadAction;
	//QAction * setAllAsNonReadAction;
	QAction * showHideMarksAction;
	QAction * getInfoAction; //comic vine
    QAction * resetComicRatingAction;

	//edit info actions
	QAction * selectAllComicsAction;
	QAction * editSelectedComicsAction;
    QAction * asignOrderAction;
    QAction * forceCoverExtractedAction;
	QAction * deleteComicsAction;
	QAction * hideComicViewAction;

    QAction *showEditShortcutsAction;

    QAction * updateFolderAction;
    QAction * updateCurrentFolderAction;

    //reading lists actions
    QAction * addReadingListAction;
    QAction * deleteReadingListAction;
    QAction * addLabelAction;
    QAction * renameListAction;
    //--
    QAction * addToMenuAction;
    QAction * addToFavoritesAction;

#ifdef Q_OS_MAC
    YACReaderMacOSXToolbar * libraryToolBar;
#else
	YACReaderMainToolBar * libraryToolBar;
#endif
	QToolBar * treeActions;
	QToolBar * comicsToolBar;
	QToolBar * editInfoToolBar;

	OptionsDialog * optionsDialog;
	ServerConfigDialog * serverConfigDialog;

	QString libraryPath;
	QString comicsPath;

	QString _lastAdded;
	QString _sourceLastAdded;

	//QModelIndex _rootIndex;
	//QModelIndex _rootIndexCV;
    //QModelIndex updateDestination;

	quint64 _comicIdEdited;

    enum NavigationStatus
    {
        Normal, //
        Searching
    };

    NavigationStatus status;

	void setupUI();
	void createActions();
	void createToolBars();
	void createMenus();
	void createConnections();
	void doLayout();
	void doDialogs();
    void setUpShortcutsManagement();
	void doModels();
    void disconnectComicsViewConnections(ComicsView * widget);
    void doComicsViewConnections();


    //ACTIONS MANAGEMENT
    void disableComicsActions(bool disabled);
    void disableLibrariesActions(bool disabled);
    void disableNoUpdatedLibrariesActions(bool disabled);
    void disableFoldersActions(bool disabled);

    void disableAllActions();
    //void disableActions();
    //void enableActions();
    //void enableLibraryActions();

    QString currentPath();
    QString currentFolderPath();

    //settings
    QSettings * settings;

    //navigation backward and forward
    YACReaderHistoryController * historyController;

    bool removeError;

    ComicsViewStatus comicsViewStatus;

protected:
    virtual void closeEvent ( QCloseEvent * event );
public:
    LibraryWindow();

public slots:
    void loadLibrary(const QString & path);
    void selectSubfolder(const QModelIndex & mi, int child);
    void checkEmptyFolder();
    void openComic();
    void createLibrary();
    void create(QString source,QString dest, QString name);
    void showAddLibrary();
    void openLibrary(QString path, QString name);
    void loadLibraries();
    void saveLibraries();
    void reloadCurrentLibrary();
    void openLastCreated();
    void updateLibrary();
    //void deleteLibrary();
    void openContainingFolder();
    void setFolderAsNotCompleted();
    void setFolderAsCompleted();
    void setFolderAsRead();
    void setFolderAsUnread();
    void openContainingFolderComic();
    void deleteCurrentLibrary();
    void removeLibrary();
    void renameLibrary();
    void rename(QString newName);
    void cancelCreating();
    void stopLibraryCreator();
    void setRootIndex();
    void toggleFullScreen();
    void toNormal();
    void toFullScreen();
    void setSearchFilter(const YACReader::SearchModifiers modifier, QString filter);
    void clearSearchFilter();
    void showProperties();
    void exportLibrary(QString destPath);
    void importLibrary(QString clc,QString destPath,QString name);
    void reloadOptions();
    void setCurrentComicsStatusReaded(YACReaderComicReadStatus readStatus);
    void setCurrentComicReaded();
    void setCurrentComicUnreaded();
    void hideComicFlow(bool hide);
    void showExportComicsInfo();
    void showImportComicsInfo();
    void asignNumbers();
    void showNoLibrariesWidget();
    void showRootWidget();
    void showImportingWidget();
    void manageCreatingError(const QString & error);
    void manageUpdatingError(const QString & error);
    void manageOpeningLibraryError(const QString & error);
    QModelIndexList getSelectedComics();
    void deleteComics();
    //void showSocial();
    void showFoldersContextMenu(const QPoint & point);
    void libraryAlreadyExists(const QString & name);
    void importLibraryPackage();
    void updateComicsView(quint64 libraryId, const ComicDB & comic);
    void setCurrentComicOpened();
    void showComicVineScraper();
    void setRemoveError();
    void checkRemoveError();
    void resetComicRating();
    void switchToComicsView(ComicsView *from, ComicsView *to);
    void showComicsViewTransition();
    void toggleComicsView_delayed();//used in orther to avoid flickering;
    void showComicsView();
    void showEmptyFolderView();
    void showEmptyLabelView();
    void showEmptySpecialList();
    void showEmptyReadingListWidget();
    void showNoSearchResultsView();
    void toggleComicsView();
    void checkSearchNumResults(int numResults);
    void loadCoversFromCurrentModel();
    void copyAndImportComicsToCurrentFolder(const QList<QPair<QString,QString> > & comics);
    void moveAndImportComicsToCurrentFolder(const QList<QPair<QString, QString> > &comics);
    void copyAndImportComicsToFolder(const QList<QPair<QString,QString> > & comics, const QModelIndex & miFolder);
    void moveAndImportComicsToFolder(const QList<QPair<QString,QString> > & comics, const QModelIndex & miFolder);
    void processComicFiles(ComicFilesManager * comicFilesManager, QProgressDialog * progressDialog);
    void updateCopyMoveFolderDestination(const QModelIndex & mi); //imports new comics from the current folder
    void updateCurrentFolder();
    void updateFolder(const QModelIndex & miFolder);
    QProgressDialog * newProgressDialog(const QString & label, int maxValue);
    void reloadAfterCopyMove(const QModelIndex &mi);
    QModelIndex getCurrentFolderIndex();
    void enableNeededActions();
    void addFolderToCurrentIndex();
    void deleteSelectedFolder();
    void errorDeletingFolder();
    void addNewReadingList();
    void deleteSelectedReadingList();
    void showAddNewLabelDialog();
    void showRenameCurrentList();
    void addSelectedComicsToFavorites();
    void showComicsViewContextMenu(const QPoint & point);
    void showComicsItemContextMenu(const QPoint & point);
    void setupAddToSubmenu(QMenu & menu);

};

#endif



