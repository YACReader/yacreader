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
#include "yacreader_global.h"

class Comic;
class Viewer;
class OptionsDialog;
class HelpAboutDialog;
class HttpVersionChecker;
class ShortcutsDialog;
class YACReaderSliderAction;
class YACReaderSlider;
class EditShortcutsDialog;

namespace YACReader {

class MainWindowViewer : public QMainWindow
{
    Q_OBJECT

public slots:
    void open();
    void open(QString path, ComicDB &comic, QList<ComicDB> &siblings);
    void open(QString path, qint64 comicId, qint64 libraryId, OpenComicSource source);
    void openFolder();
    void openRecent();
    void openLatestComic();
    void openComicFromRecentAction(QAction *action);
    void saveImage();
    void toggleToolBars();
    void hideToolBars();
    void showToolBars();
    void enableActions();
    void disableActions();
    void toggleFullScreen();
    void toFullScreen();
    void toNormal();
    void loadConfiguration();
    void newVersion();
    void openPreviousComic();
    void openNextComic();
    void openLeftComic();
    void openRightComic();
    void openComicFromPath(QString pathFile);
    void openSiblingComic(QString pathFile);
    void openComic(QString pathFile);
    void openFolderFromPath(QString pathDir);
    void openFolderFromPath(QString pathFile, QString atFileName);
    void alwaysOnTopSwitch();
    void adjustToFullSizeSwitch();
    void fitToPageSwitch();
    void resetZoomLevel();
    void increasePageZoomLevel();
    void decreasePageZoomLevel();
    void reloadOptions();
    void fitToWidth();
    void fitToHeight();
    void toggleWidthHeight();
    void checkNewVersion();
    void processReset();
    void setUpShortcutsManagement();
    void doubleMangaPageSwitch();

    void toggleFitToWidthSlider();

    /*void viewComic();
                void prev();
                void next();
                void updatePage();*/

private:
    //! State
    bool fullscreen;
    bool toolbars;
    bool alwaysOnTop;
    bool fromMaximized;

    // QTBUG-41883
    QSize _size;
    QPoint _pos;

    QString currentDirectory;
    QString currentDirectoryImgDest;
    //! Widgets
    Viewer *viewer;
    // GoToDialog * goToDialog;
    OptionsDialog *optionsDialog;
    HelpAboutDialog *had;
    // ShortcutsDialog * shortcutsDialog;
    EditShortcutsDialog *editShortcutsDialog;

    //! ToolBars
#ifdef Q_OS_MAC
    YACReaderMacOSXToolbar *comicToolBar;
#else
    QToolBar *comicToolBar;
#endif

    //! Actions
    QAction *openAction;
#ifdef Q_OS_MAC
    QAction *newInstanceAction; // needed in macos
#endif
    QAction *openFolderAction;
    QAction *openLatestComicAction;
    QList<QAction *> recentFilesActionList;
    QAction *clearRecentFilesAction;
    QAction *saveImageAction;
    QAction *openComicOnTheLeftAction;
    QAction *openComicOnTheRightAction;
    QAction *goToPageOnTheRightAction;
    QAction *goToPageOnTheLeftAction;
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
    QAction *fitToPageAction;
    QAction *resetZoomAction;
    QAction *showZoomSliderlAction;
    QAction *increasePageZoomAction;
    QAction *decreasePageZoomAction;
    QAction *showFlowAction;

    QAction *showEditShortcutsAction;

    YACReaderSlider *zoomSliderAction;

    HttpVersionChecker *versionChecker;
    QString previousComicPath;
    QString nextComicPath;
    //! Método que inicializa el interfaz.
    void setupUI();
    void createActions();
    void createToolBars();
    void refreshRecentFilesActionList();
    void clearRecentFiles();
    void getSiblingComics(QString path, QString currentComic);

    //! Manejadores de evento:
    void keyPressEvent(QKeyEvent *event) override;
    // void resizeEvent(QResizeEvent * event);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;

    QSettings *settings;

    ComicDB currentComicDB;
    QList<ComicDB> siblingComics;
    bool isClient;
    QString startComicPath;
    quint64 libraryId;
    OpenComicSource source;

    // fullscreen mode in Windows for preventing this bug: QTBUG-41309 https://bugreports.qt.io/browse/QTBUG-41309
    Qt::WindowFlags previousWindowFlags;
    QPoint previousPos;
    QSize previousSize;

protected:
    void closeEvent(QCloseEvent *event) override;
    void sendComic();
    void updatePrevNextActions(bool thereIsPrevious, bool thereIsNext);
    void afterLaunchTasks();

public:
    MainWindowViewer();
    ~MainWindowViewer() override;
};

}

#endif
