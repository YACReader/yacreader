#ifndef __MAIN_WINDOW_VIEWER_H
#define __MAIN_WINDOW_VIEWER_H
#include <QMainWindow>
#include <QScrollArea>
#include <QToolBar>
#include <QAction>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QSettings>

#ifdef Q_OS_MAC
    #include "yacreader_macosx_toolbar.h"
#endif

#include "comic_db.h"

class Comic;
class Viewer;
class OptionsDialog;
class HelpAboutDialog;
class HttpVersionChecker;
class ShortcutsDialog;
class YACReaderSliderAction;
class EditShortcutsDialog;

	class MainWindowViewer : public QMainWindow
	{
	Q_OBJECT

	public slots:
		void open();
		void open(QString path, ComicDB & comic, QList<ComicDB> & siblings);
		void open(QString path, qint64 comicId, qint64 libraryId);
		void openFolder();
		void saveImage();
		void toggleToolBars();
		void hideToolBars();
		void showToolBars();
		void changeFit();
		void enableActions();
		void disableActions();
		void toggleFullScreen();
		void toFullScreen();
		void toNormal();
		void loadConfiguration();
		void newVersion();
		void openPreviousComic();
		void openNextComic();
		void openComicFromPath(QString pathFile);
		void openFolderFromPath(QString pathDir);
		void openFolderFromPath(QString pathFile, QString atFileName);
		void alwaysOnTopSwitch();
		void adjustToFullSizeSwitch();
		void reloadOptions();
		void fitToWidth();
		void fitToHeight();
		void checkNewVersion();
        void processReset();
        void setUpShortcutsManagement();
		/*void viewComic();
		void prev();
		void next();
		void updatePage();*/

	private:
		//!State
		bool fullscreen;
		bool toolbars;
		bool alwaysOnTop;
		bool fromMaximized;
		
		QString currentDirectory;
		QString currentDirectoryImgDest;
		//!Widgets
		Viewer * viewer;
		//GoToDialog * goToDialog;
		OptionsDialog * optionsDialog;
		HelpAboutDialog * had;
		ShortcutsDialog * shortcutsDialog;
        EditShortcutsDialog * editShortcutsDialog;

		//! ToolBars
        #ifdef Q_OS_MAC
        YACReaderMacOSXToolbar * comicToolBar;
#else
        QToolBar * comicToolBar;
#endif

		//! Actions
		QAction *openAction;
		QAction *openFolderAction;
		QAction *saveImageAction;
		QAction *openPreviousComicAction;
		QAction *openNextComicAction;
		QAction *nextAction;
		QAction *prevAction;
        QAction *adjustWidthAction;
        QAction *adjustHeightAction;
        QAction *goToPageAction;
		QAction *optionsAction;
		QAction *helpAboutAction;
        QAction *showMagnifyingGlassAction;
        QAction *setBookmarkAction;
        QAction *showBookmarksAction;
		QAction *leftRotationAction;
		QAction *rightRotationAction;
        QAction *showInfoAction;
		QAction *closeAction;
		QAction *doublePageAction;
		QAction *doubleMangaPageAction;
		QAction *showShorcutsAction;
		QAction *showDictionaryAction;
		QAction *alwaysOnTopAction;
		QAction *adjustToFullSizeAction;
		QAction *showFlowAction;

        QAction *showEditShortcutsAction;

		YACReaderSliderAction * sliderAction;

		HttpVersionChecker * versionChecker;
		QString previousComicPath;
		QString nextComicPath;
		//! Método que inicializa el interfaz.
		void setupUI();
		void createActions();
		void createToolBars();
		void getSiblingComics(QString path,QString currentComic);

		//! Manejadores de evento:
        void keyPressEvent(QKeyEvent *event);
        //void resizeEvent(QResizeEvent * event);
		void mouseDoubleClickEvent ( QMouseEvent * event );
		void dropEvent(QDropEvent *event);
		void dragEnterEvent(QDragEnterEvent *event);

		QSettings * settings;

		ComicDB currentComicDB;
		QList<ComicDB> siblingComics;
		bool isClient;
		QString startComicPath;
		quint64 libraryId;
signals:
		void closed();
	protected:
		virtual void closeEvent ( QCloseEvent * event );
		void sendComic();
	public:
		MainWindowViewer();
		~MainWindowViewer();

};
#endif
