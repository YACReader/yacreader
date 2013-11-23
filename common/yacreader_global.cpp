#include "yacreader_global.h"

using namespace YACReader;

QString YACReader::getSettingsPath()
{
#if QT_VERSION >= 0x050000
	return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
#else
	return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif

}
