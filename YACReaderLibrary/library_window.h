#ifndef __LIBRARYWINDOW_H
#define __LIBRARYWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QTreeView>
#include <QModelIndex>
#include <QDirModel>
#include <QAction>
#include <QToolBar>
#include <QComboBox>
#include <QThread>
#include <QFileInfoList>

#include "create_library_dialog.h"
#include "add_library_dialog.h"
#include "library_creator.h"
#include "comic_flow.h"
#include "custom_widgets.h"
#include "rename_library_dialog.h"
#include "properties_dialog.h"
#include "options_dialog.h"
#include "export_library_dialog.h"
#include "import_library_dialog.h"
#include "package_manager.h"
#include "treemodel.h"
#include "tablemodel.h"

class LibraryWindow : public QMainWindow
{
	Q_OBJECT
private:
	QWidget * left;
	CreateLibraryDialog * createLibraryDialog;
	UpdateLibraryDialog * updateLibraryDialog;
	ExportLibraryDialog * exportLibraryDialog;
	ImportLibraryDialog * importLibraryDialog;
	AddLibraryDialog * addLibraryDialog;
	LibraryCreator * libraryCreator;
	HelpAboutDialog * had;
	RenameLibraryDialog * renameLibraryDialog;
	PropertiesDialog * propertiesDialog;
	bool fullscreen;
	bool importedCovers; //if true, the library is read only (not updates,open comic or properties)
	YACReaderTreeSearch * proxyFilter;
	YACReaderSortComics * proxySort;
	PackageManager * packageManager;

	ComicFlow * comicFlow;
	QSize slideSizeW;
	QSize slideSizeF;
	//search filter
	QLineEdit * foldersFilter;
	QString previousFilter;
	QPushButton * clearFoldersFilter;
	QCheckBox * includeComicsCheckBox;
	//-------------
	QTableView * comicView;
	QTreeView * foldersView;
	QComboBox * selectedLibrary;
	TreeModel * dm;
	TableModel * dmCV;
	//QStringList paths;
	QMap<QString,QString> libraries;
	QLabel * fullScreenToolTip;
	YACReaderIconProvider fip;

	bool fetching;

	int i;
	unsigned int skip;

	QAction * openComicAction;
	QAction * showPropertiesAction;
	QAction * createLibraryAction;
	QAction * openLibraryAction;
	QAction * exportLibraryAction;
	QAction * importLibraryAction;
	QAction * updateLibraryAction;
	QAction * deleteLibraryAction;
	QAction * removeLibraryAction;
	QAction * helpAboutAction;
	QAction * renameLibraryAction;
	QAction * toggleFullScreenAction;
	QAction * optionsAction;

	QAction * setRootIndexAction;
	QAction * expandAllNodesAction;
	QAction * colapseAllNodesAction;

	QAction * openContainingFolderAction;
	QAction * openContainingFolderComicAction;
	QAction * setAsReadAction;
	QAction * setAsNonReadAction;
	QAction * setAllAsReadAction;
	QAction * setAllAsNonReadAction;
	QAction * showHideMarksAction;

	QAction * selectAllComicsAction;
	QAction * editSelectedComicsAction;
	QAction * asignOrderActions;
	QAction * forceConverExtractedAction;


	QToolBar * libraryToolBar;
	QToolBar * treeActions;
	QToolBar * comicsToolBar;
	QToolBar * editInfoToolBar;

	OptionsDialog * optionsDialog;

	QString libraryPath;
	QString comicsPath;

	QString _lastAdded;

	QModelIndex _rootIndex;
	QModelIndex _rootIndexCV;

	void setupUI();
	void createActions();
	void createToolBars();
	void createMenus();
	void createConnections();
	void doLayout();
	void doDialogs();
	void doModels();

	void disableAllActions();
	void disableActions();
	void enableActions();
	void enableLibraryActions();

	QString currentPath();
public:
	LibraryWindow();
	public slots:
		void loadLibrary(const QString & path);
		void loadCovers(const QModelIndex & mi);
		void centerComicFlow(const QModelIndex & mi);
		void updateComicView(int i);
		void openComic();
		void createLibrary();
		void create(QString source,QString dest, QString name);
		void showAddLibrary();
		void openLibrary(QString path, QString name);
		void loadLibraries();
		void saveLibraries();
		void openLastCreated();
		void updateLibrary();
		void deleteLibrary();
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
		void updateFoldersView(QString);
		void setCurrentComicReaded();
		void setCurrentComicUnreaded();
		void setComicsReaded();
		void setComicsUnreaded();
		void searchInFiles(int);
};

#endif



