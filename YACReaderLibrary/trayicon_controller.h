#ifndef TRAYICON_CONTROLLER_H
#define TRAYICON_CONTROLLER_H

#include <QtWidgets>
#include <QSystemTrayIcon>

class LibraryWindow;

namespace YACReader {

class TrayIconController : public QObject
{
    Q_OBJECT
public:
    TrayIconController(QSettings *settings,
                       LibraryWindow *window);

    void updateIconVisibility();

    bool handleCloseToTrayIcon(QCloseEvent *event);

    QSystemTrayIcon trayIcon;

public slots:
    void showWindow();

private:
    QSettings *settings;
    LibraryWindow *window;

    QMenu *trayIconMenu;
};

}

#endif // TRAYICON_CONTROLLER_H
