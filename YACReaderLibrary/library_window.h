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
//#include "comic_flow.h"
#include "comic_flow_widget.h"
#include "custom_widgets.h"
#include "rename_library_dialog.h"
#include "properties_dialog.h"
#include "options_dialog.h"
#include "export_comics_info_dialog.h"
#include "import_comics_info_dialog.h"
#include "export_library_dialog.h"
#include "import_library_dialog.h"
#include "package_manager.h"
#include "treemodel.h"
#include "tablemodel.h"
#include "treeitem.h"
#include "server_config_dialog.h"

class LibraryWindow : public QMainWindow
{
	Q_OBJECT
private:
	QWidget * left;
	QSplitter * sVertical;
	CreateLibraryDialog * createLibraryDialog;
	UpdateLibraryDialog * updateLibraryDialog;
	ExportLibraryDialog * exportLibraryDialog;
	ImportLibraryDialog * importLibraryDialog;
	ExportComicsInfoDialog * exportComicsInfoDialog;
	ImportComicsInfoDialog * importComicsInfoDialog;
	AddLibraryDialog * addLibraryDialog;
	LibraryCreator * libraryCreator;
	HelpAboutDialog * had;
	RenameLibraryDialog * renameLibraryDialog;
	PropertiesDialog * propertiesDialog;
	bool fullscreen;
	bool importedCovers; //if true, the library is read only (not updates,open comic or properties)
	//Ya no se usan proxies, el rendimiento de la BD es suficiente
	//YACReaderTreeSearch * proxyFilter;
	//YACReaderSortComics * proxySort;
	PackageManager * packageManager;

	ComicFlowWidget * comicFlow;
	QSize slideSizeW;
	QSize slideSizeF;
	//search filter
	QLineEdit * foldersFilter;
	TreeItem * index; //index al que hay que hacer scroll después de pulsar sobre un folder filtrado
	int column;
	QString previousFilter;
	QPushButton * clearFoldersFilter;
	QCheckBox * includeComicsCheckBox;
	//-------------
	QWidget *comics;
	QTableView * comicView;
	QTreeView * foldersView;
	QComboBox * selectedLibrary;
	TreeModel * dm;
	QItemSelectionModel * sm;
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
	
	QAction * exportComicsInfo;
	QAction * importComicsInfo;

	QAction * exportLibraryAction;
	QAction * importLibraryAction;

	QAction * updateLibraryAction;
	QAction * deleteLibraryAction;
	QAction * removeLibraryAction;
	QAction * helpAboutAction;
	QAction * renameLibraryAction;
	QAction * toggleFullScreenAction;
	QAction * optionsAction;
	QAction * serverConfigAction;

	//tree actions
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

	//edit info actions
	QAction * selectAllComicsAction;
	QAction * editSelectedComicsAction;
	QAction * asignOrderActions;
	QAction * forceConverExtractedAction;
	QAction * hideComicViewAction;


	QToolBar * libraryToolBar;
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

	void disableAllActions();
	void disableActions();
	void enableActions();
	void enableLibraryActions();

	QString currentPath();

	//settings
	QSettings * settings;
protected:
		virtual void closeEvent ( QCloseEvent * event );
public:
	LibraryWindow();
	public slots:
		void loadLibrary(const QString & path);
		void loadCovers(const QModelIndex & mi);
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
		void hideComicFlow(bool hide);
		void showExportComicsInfo();
		void showImportComicsInfo();
		void asignNumbers();

		//server interface
		QMap<QString,QString> getLibraries(){return libraries;};
		QList<LibraryItem *> getFolderContentFromLibrary(const QString & libraryName, qulonglong folderId);
		QList<LibraryItem *> getFolderComicsFromLibrary(const QString & libraryName, qulonglong folderId);
		qulonglong getParentFromComicFolderId(const QString & libraryName, qulonglong id);
		ComicDB getComicInfo(const QString & libraryName, qulonglong id);
		QString getFolderName(const QString & libraryName, qulonglong id);
};

#endif



