#ifndef __LIBRARYWINDOW_H
#define __LIBRARYWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QModelIndex>
#include <QFileInfo>
#include "yacreader_global.h"

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
class ComicFlowWidget;
class QCheckBox;
class QPushButton;
class TableModel;
class QSplitter;
class TreeItem;
class TreeModel;
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
class YACReaderTreeView;
class YACReaderMainToolBar;
#include "comic_db.h"

using namespace YACReader;

class LibraryWindow : public QMainWindow
{
	Q_OBJECT
private:
	YACReaderSideBar * sideBar;
	QSplitter * sVertical;
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
	//YACReaderSocialDialog * socialDialog;
	bool fullscreen;
	bool importedCovers; //if true, the library is read only (not updates,open comic or properties)
	bool fromMaximized;
	//Ya no se usan proxies, el rendimiento de la BD es suficiente
	//YACReaderTreeSearch * proxyFilter;
	//YACReaderSortComics * proxySort;
	PackageManager * packageManager;

	ComicFlowWidget * comicFlow;
	QSize slideSizeW;
	QSize slideSizeF;
	//search filter
	YACReaderSearchLineEdit * foldersFilter;
	TreeItem * index; //index al que hay que hacer scroll después de pulsar sobre un folder filtrado
	int column;
	QString previousFilter;
	QPushButton * clearFoldersFilter;
	QCheckBox * includeComicsCheckBox;
	//-------------
	QWidget *comics;
	YACReaderTableView * comicView;
	YACReaderTreeView * foldersView;
	YACReaderLibraryListWidget * selectedLibrary;
	TreeModel * dm;
	QItemSelectionModel * sm;
	TableModel * dmCV;
	//QStringList paths;
	QMap<QString,QString> libraries;
	QLabel * fullScreenToolTip;

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
	
	QAction * exportComicsInfo;
	QAction * importComicsInfo;

	QAction * exportLibraryAction;
	QAction * importLibraryAction;

	QAction * updateLibraryAction;
	QAction * removeLibraryAction;
	QAction * helpAboutAction;
	QAction * renameLibraryAction;
	QAction * toggleFullScreenAction;
	QAction * optionsAction;
	QAction * serverConfigAction;
	//QAction * socialAction;

	//tree actions
	QAction * setRootIndexAction;
	QAction * expandAllNodesAction;
	QAction * colapseAllNodesAction;

	QAction * openContainingFolderAction;
	QAction * openContainingFolderComicAction;
	QAction * setAsReadAction;
	QAction * setAsNonReadAction;
	//QAction * setAllAsReadAction;
	//QAction * setAllAsNonReadAction;
	QAction * showHideMarksAction;
	QAction * getInfoAction; //comic vine

	//edit info actions
	QAction * selectAllComicsAction;
	QAction * editSelectedComicsAction;
	QAction * asignOrderActions;
	QAction * forceConverExtractedAction;
	QAction * deleteComicsAction;
	QAction * hideComicViewAction;

#ifdef Q_OS_MAC
	QToolBar * libraryToolBar;
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

	QModelIndex _rootIndex;
	QModelIndex _rootIndexCV;

	quint64 _comicIdEdited;

	void setupUI();
	void createActions();
	void createToolBars();
	void createMenus();
	void createConnections();
	void doLayout();
	void doDialogs();
	void doModels();

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

	//settings
	QSettings * settings;

	//navigation backward and forward
	int currentFolderNavigation;
	QList<QModelIndex> history;

protected:
		virtual void closeEvent ( QCloseEvent * event );
public:
	LibraryWindow();
	public slots:
		void loadLibrary(const QString & path);
		void loadCovers(const QModelIndex & mi);
		void checkEmptyFolder(QStringList * paths = 0);
		void reloadCovers();
		void centerComicFlow(const QModelIndex & mi);
		void updateComicView(int i);
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
		void setFoldersFilter(QString filter);
		void showProperties();
		void exportLibrary(QString destPath);
		void importLibrary(QString clc,QString destPath,QString name);
		void reloadOptions();
		void setCurrentComicsStatusReaded(YACReaderComicReadStatus readStatus);
		void setCurrentComicReaded();
		void setCurrentComicUnreaded();
		void setComicsReaded();
		void setComicsUnreaded();
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
		void backward();
		void forward();
		void updateHistory(const QModelIndex & mi);
		void libraryAlreadyExists(const QString & name);
		void importLibraryPackage();
		void updateComicsView(quint64 libraryId, const ComicDB & comic);
		void setCurrentComicOpened();
		void showComicVineScraper();
};

#endif



