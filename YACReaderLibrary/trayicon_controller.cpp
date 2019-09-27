#include "trayicon_controller.h"

#include "yacreader_global_gui.h"

#include "library_window.h"

#include <QtWidgets>
#include <QMessageBox>

#ifdef Q_OS_MACOS
#include "trayhandler.h"
#endif

using namespace YACReader;

TrayIconController::TrayIconController(QSettings *settings, LibraryWindow *window)
    : QObject(nullptr), settings(settings), window(window)
{

    // If a window icon was set in main() we reuse it for the tray too.
    // This allows support for third party icon themes on Freedesktop(Linux/Unix)
    // systems.
    if (!QApplication::windowIcon().isNull()) {
        trayIcon.setIcon(QApplication::windowIcon());
    } else {
#ifdef Q_OS_WIN
        trayIcon.setIcon(QIcon(":/icon.ico"));
#else
#ifdef Q_OS_MACOS
        auto icon = QIcon(":/macostrayicon.svg");
        icon.setIsMask(true);
        trayIcon.setIcon(icon);
#else
        trayIcon.setIcon(QIcon(":/images/iconLibrary.png"));
#endif
#endif
    }

    connect(&trayIcon, &QSystemTrayIcon::activated,
            [=](QSystemTrayIcon::ActivationReason reason) {
#ifdef Q_OS_LINUX
                auto expectedReason = QSystemTrayIcon::Trigger;
#else
                auto expectedReason = QSystemTrayIcon::DoubleClick;
#endif

                if (reason == expectedReason) {
                    showWindow();
                }
            });

    auto restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, &QAction::triggered, this, &TrayIconController::showWindow);

    auto quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, window, &LibraryWindow::closeApp);

    trayIconMenu = new QMenu(this->window);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon.setContextMenu(trayIconMenu);

    updateIconVisibility();
}

void TrayIconController::updateIconVisibility()
{
    trayIcon.setVisible(settings->value(CLOSE_TO_TRAY, false).toBool());
}

bool TrayIconController::handleCloseToTrayIcon(QCloseEvent *event)
{
#ifdef Q_OS_OSX
    if (!event->spontaneous() || !window->isVisible()) {
        return false;
    }
#endif
    if (trayIcon.isVisible()) {
        if (!settings->value(CLOSE_TO_TRAY_NOTIFIED, false).toBool()) {
            QMessageBox::information(window, tr("Systray"),
                                     tr("YACReaderLibrary will keep running in the "
                                        "system tray. To terminate the program, "
                                        "choose <b>Quit</b> in the context menu "
                                        "of the system tray icon."));
            settings->setValue(CLOSE_TO_TRAY_NOTIFIED, true);
        }
#ifdef Q_OS_OSX
        OSXHideDockIcon();
#endif
        window->hide();
        event->ignore();
        return true;
    } else {
        return false;
    }
}

void TrayIconController::showWindow()
{
#ifdef Q_OS_MACOS
    OSXShowDockIcon();
    window->showNormal();
#else
    if (window->isMinimized()) {
        window->setWindowState((window->windowState() & ~Qt::WindowMinimized));
    } else {
        window->show();
    }
#endif
    window->raise(); // for MacOS
    window->activateWindow(); // for Windows
}
