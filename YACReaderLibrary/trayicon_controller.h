#ifndef TRAYICON_CONTROLLER_H
#define TRAYICON_CONTROLLER_H

#include <QtWidgets>
#include <QSystemTrayIcon>

namespace YACReader {

class TrayIconController : public QObject
{
    Q_OBJECT
public:
    TrayIconController(QSettings *settings,
                       QMainWindow *window);

    void updateIconVisibility();

    bool handleCloseToTrayIcon(QCloseEvent *event);

    QSystemTrayIcon trayIcon;

public slots:
    void showWindow();

private:
    QSettings *settings;
    QMainWindow *window;

    QMenu *trayIconMenu;
};

}

#endif // TRAYICON_CONTROLLER_H
