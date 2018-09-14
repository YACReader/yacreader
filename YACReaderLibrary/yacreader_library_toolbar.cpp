#include "yacreader_library_toolbar.h"

#include "shortcuts_manager.h"
#include "yacreader_library_search_edit.h"
#include "yacreader_global_gui.h"

using namespace YACReader;

YACReaderLibraryToolbar::YACReaderLibraryToolbar(QMainWindow *mainWindow, bool isMacosNative, bool useNativeFullScreen, QSettings *settings)
    :QObject(mainWindow), mainWindow(mainWindow), isMacosNative(isMacosNative), useNativeFullScreen(useNativeFullScreen), settings(settings)
{
    backAction = new QAction(mainWindow);
    QIcon icoBackButton;
    icoBackButton.addFile(":/images/main_toolbar/back.png",QSize(), QIcon::Normal);
    //icoBackButton.addPixmap(QPixmap(":/images/main_toolbar/back_disabled.png"), QIcon::Disabled);
    backAction->setData(BACK_ACTION_YL);
    backAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(BACK_ACTION_YL));
    backAction->setIcon(icoBackButton);
    backAction->setDisabled(true);

    forwardAction = new QAction(mainWindow);
    QIcon icoFordwardButton;
    icoFordwardButton.addFile(":/images/main_toolbar/forward.png", QSize(), QIcon::Normal);
    //icoFordwardButton.addPixmap(QPixmap(":/images/main_toolbar/forward_disabled.png"), QIcon::Disabled);
    forwardAction->setData(FORWARD_ACTION_YL);
    forwardAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(FORWARD_ACTION_YL));
    forwardAction->setIcon(icoFordwardButton);
    forwardAction->setDisabled(true);

    optionsAction = new QAction(mainWindow);
    optionsAction->setToolTip(tr("Show options dialog"));
    optionsAction->setData(OPTIONS_ACTION_YL);
    optionsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPTIONS_ACTION_YL));
    QIcon icoSettingsButton;
    icoSettingsButton.addFile(":/images/main_toolbar/settings.png", QSize(), QIcon::Normal);
    optionsAction->setIcon(icoSettingsButton);

    serverConfigAction = new QAction(mainWindow);
    serverConfigAction->setToolTip(tr("Show comics server options dialog"));
    serverConfigAction->setData(SERVER_CONFIG_ACTION_YL);
    serverConfigAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SERVER_CONFIG_ACTION_YL));
    QIcon icoServerButton;
    icoServerButton.addFile(":/images/main_toolbar/server.png", QSize(), QIcon::Normal);
    serverConfigAction->setIcon(icoServerButton);

    toggleComicsViewAction = new QAction(tr("Change between comics views"),mainWindow);
    toggleComicsViewAction->setToolTip(tr("Change between comics views"));
    QIcon icoViewsButton;

    if(!settings->contains(COMICS_VIEW_STATUS) || settings->value(COMICS_VIEW_STATUS) == Flow)
        icoViewsButton.addFile(":/images/main_toolbar/grid.png", QSize(), QIcon::Normal);
    else if(settings->value(COMICS_VIEW_STATUS) == Grid)
        icoViewsButton.addFile(":/images/main_toolbar/info.png", QSize(), QIcon::Normal);
    else
        icoViewsButton.addFile(":/images/main_toolbar/flow.png", QSize(), QIcon::Normal);

    toggleComicsViewAction->setData(TOGGLE_COMICS_VIEW_ACTION_YL);
    toggleComicsViewAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_COMICS_VIEW_ACTION_YL));
    toggleComicsViewAction->setIcon(icoViewsButton);

    helpAboutAction = new QAction(mainWindow);
    helpAboutAction->setToolTip(tr("Help, About YACReader"));
    helpAboutAction->setData(HELP_ABOUT_ACTION_YL);
    helpAboutAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(HELP_ABOUT_ACTION_YL));
    QIcon icoHelpButton;
    icoHelpButton.addFile(":/images/main_toolbar/help.png",QSize(), QIcon::Normal);
    helpAboutAction->setIcon(icoHelpButton);

    if (!useNativeFullScreen) {
        toggleFullScreenAction = new QAction(tr("Fullscreen mode on/off"),mainWindow);
        toggleFullScreenAction->setToolTip(tr("Fullscreen mode on/off"));
        toggleFullScreenAction->setData(TOGGLE_FULL_SCREEN_ACTION_YL);
        toggleFullScreenAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_FULL_SCREEN_ACTION_YL));
        QIcon icoFullscreenButton;
        icoFullscreenButton.addPixmap(QPixmap(":/images/main_toolbar/fullscreen.png"), QIcon::Normal);
        toggleFullScreenAction->setIcon(icoFullscreenButton);
    }

    if (isMacosNative) {
#ifdef Q_OS_MAC
        macosxToolbar = new YACReaderMacOSXToolbar(mainWindow);

        macosxToolbar->addAction(backAction);
        macosxToolbar->addAction(forwardAction);

        macosxToolbar->addSpace(10);

        macosxToolbar->addAction(serverConfigAction);

        macosxToolbar->addAction(optionsAction);
        macosxToolbar->addAction(helpAboutAction);

        macosxToolbar->addSpace(10);

        macosxToolbar->addAction(toggleComicsViewAction);

        macosxToolbar->addStretch();

        auto actualSearchEdit = macosxToolbar->addSearchEdit();
        searchEdit = new YACReaderLibrarySearchEdit(actualSearchEdit);

        macosxToolbar->attachToWindow(mainWindow->windowHandle());
#endif
    } else {
        defaultToolbar = new YACReaderMainToolBar(!useNativeFullScreen, mainWindow);
        auto actualSearchEdit = new YACReaderSearchLineEdit();
        searchEdit = new YACReaderLibrarySearchEdit(actualSearchEdit);

        defaultToolbar->backButton->setDefaultAction(backAction);
        defaultToolbar->forwardButton->setDefaultAction(forwardAction);
        defaultToolbar->settingsButton->setDefaultAction(optionsAction);
        defaultToolbar->serverButton->setDefaultAction(serverConfigAction);
        defaultToolbar->helpButton->setDefaultAction(helpAboutAction);
        defaultToolbar->toggleComicsViewButton->setDefaultAction(toggleComicsViewAction);
        defaultToolbar->fullscreenButton->setDefaultAction(toggleFullScreenAction);

        defaultToolbar->setSearchWidget(actualSearchEdit);
    }
}

void YACReaderLibraryToolbar::show()
{
    if (isMacosNative) {
#ifdef Q_OS_MAC
        macosxToolbar->show();
#endif
    } else {
        defaultToolbar->show();
    }
}

void YACReaderLibraryToolbar::hide()
{
    if (isMacosNative) {
#ifdef Q_OS_MAC
        macosxToolbar->hide();
#endif
    } else {
        defaultToolbar->hide();
    }
}

void YACReaderLibraryToolbar::updateViewSelectorIcon(const QIcon &icon)
{
#ifdef Q_OS_MAC
    if (macosxToolbar != nullptr) {
        macosxToolbar->updateViewSelectorIcon(icon);
    }
#endif
}

void YACReaderLibraryToolbar::setDisabled(bool disabled)
{
    if (defaultToolbar != nullptr) {
        defaultToolbar->setDisabled(disabled);
    }
}

void YACReaderLibraryToolbar::setTitle(const QString &title)
{
    if (defaultToolbar != nullptr) {
        defaultToolbar->setTitle(title);
    }
}
