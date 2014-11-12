#include "yacreader_global.h"
#include <QAction>

using namespace YACReader;

QString YACReader::getSettingsPath()
{
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
	return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif

}

void YACReader::addSperator(QWidget *w)
{
    QAction * separator = new QAction(w);
    separator->setSeparator(true);
    w->addAction(separator);
}


QAction * YACReader::createSeparator()
{
    QAction * a = new QAction(0);
    a->setSeparator(true);
    return a;
}


QString YACReader::colorToName(LabelColors colors)
{
    switch(colors){
    case 0:
        return "red";
    case 1:
        return "orange";
    case 2:
        return "yellow";
    case 3:
        return "green";
    case 4:
        return "cyan";
    case 5:
        return "blue";
    case 6:
        return "violet";
    case 7:
        return "purple";
    case 8:
        return "pink";
    case 9:
        return "white";
    case 10:
        return "light";
    case 11:
        return "dark";
    }
}


QIcon YACReader::noHighlightedIcon(const QString &path)
{
    QPixmap p(path);
    QIcon icon;
    icon.addPixmap(p,QIcon::Normal);
    icon.addPixmap(p,QIcon::Selected);
    return icon;
}
