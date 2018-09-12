#ifndef YACREADER_LIBRARY_TOOLBAR_H
#define YACREADER_LIBRARY_TOOLBAR_H

#include <QtWidgets>

#include "yacreader_main_toolbar.h"
#ifdef Q_OS_MAC
#include "yacreader_macosx_toolbar.h"
#endif

#include "yacreader_library_search_edit.h"

class YACReaderLibraryToolbar: public QObject
{
public:
    YACReaderLibraryToolbar(QMainWindow *mainWindow, bool isMacosNative, bool useNativeFullScreen, QSettings *settings);

    QWidget * widget() { return defaultToolbar; }

    QAction  * backAction;
    QAction  * forwardAction;

    QAction * optionsAction;
    QAction * serverConfigAction;
    QAction * toggleComicsViewAction;

    QAction * helpAboutAction;

    QAction * toggleFullScreenAction = nullptr;

    YACReaderLibrarySearchEdit *searchEdit;
public slots:
    void show();
    void hide();

    void updateViewSelectorIcon(const QIcon &icon);

    void setDisabled(bool disabled);

    void setTitle(const QString &title);

private:
    QMainWindow *mainWindow;
    bool isMacosNative;
    bool useNativeFullScreen;
    QSettings * settings;

#ifdef Q_OS_MAC
    YACReaderMacOSXToolbar *macosxToolbar;
#endif

    YACReaderMainToolBar *defaultToolbar = nullptr;


};

#endif // YACREADER_LIBRARY_TOOLBAR_H
