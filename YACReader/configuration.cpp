#include "configuration.h"

#include <QFile>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QStringList>
#include <QMessageBox>

#include "yacreader_global.h"

Configuration::Configuration()
{
}

QSettings *Configuration::getSettings()
{
    return settings;
}

void Configuration::load(QSettings *settings)
{
    this->settings = settings;

    //TODO set defaults
    if (!settings->contains(PATH))
        settings->setValue(PATH, ".");
    if (!settings->contains(GO_TO_FLOW_SIZE))
        settings->setValue(GO_TO_FLOW_SIZE, QSize(126, 200));
    if (!settings->contains(MAG_GLASS_SIZE))
        settings->setValue(MAG_GLASS_SIZE, QSize(350, 175));
    if (!settings->contains(ZOOM_LEVEL))
        settings->setValue(MAG_GLASS_SIZE, QSize(350, 175));
    if (!settings->contains(FLOW_TYPE))
        settings->setValue(FLOW_TYPE, 0);
    if (!settings->contains(FULLSCREEN))
        settings->setValue(FULLSCREEN, false);
    if (!settings->contains(MAXIMIZED))
        settings->setValue(MAXIMIZED, false);
    if (!settings->contains(DOUBLE_PAGE))
        settings->setValue(DOUBLE_PAGE, false);
    if (!settings->contains(BACKGROUND_COLOR))
        settings->setValue(BACKGROUND_COLOR, QColor(40, 40, 40));
    if (!settings->contains(ALWAYS_ON_TOP))
        settings->setValue(ALWAYS_ON_TOP, false);
    if (!settings->contains(SHOW_TOOLBARS))
        settings->setValue(SHOW_TOOLBARS, true);
    if (!settings->contains(QUICK_NAVI_MODE))
        settings->setValue(QUICK_NAVI_MODE, false);
}
void Configuration::updateOpenRecentList(QString path)
{
    QStringList list = openRecentList();
    list.removeAll(path);
    list.prepend(path);
    //TODO: Make list lenght configurable
    while (list.length() > getOpenRecentSize()) {
        list.removeLast();
    }
    settings->setValue("recentFiles", list);
}
