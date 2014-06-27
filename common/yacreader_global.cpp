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
