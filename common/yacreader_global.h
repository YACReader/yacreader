#ifndef __YACREADER_GLOBAL_H
#define __YACREADER_GLOBAL_H

#if QT_VERSION >= 0x050000
	#include <QStandardPaths>
#else
	#include <QDesktopServices>
#endif

#define VERSION "8.0.0"

#define NUM_DAYS_BETWEEN_VERSION_CHECKS "NUM_DAYS_BETWEEN_VERSION_CHECKS"
#define LAST_VERSION_CHECK "LAST_VERSION_CHECK"

#define YACREADERLIBRARY_GUID "ea343ff3-2005-4865-b212-7fa7c43999b8"

#define LIBRARIES "LIBRARIES"

namespace YACReader
{

 enum YACReaderIPCMessages
 {
	 RequestComicInfo = 0,
	 SendComicInfo,
 };

 enum YACReaderComicReadStatus
 {
	 Unread = 0,
	 Read = 1,
	 Opened = 2
 };

 enum YACReaderErrors
 {
	 SevenZNotFound = 700
 };

 enum SearchModifiers{
     NoModifiers = 0,
     OnlyRead,
     OnlyUnread,
     ByAuthor
 };

 enum LabelColors{
     YRed = 1,
     YOrange,
     YYellow,
     YGreen,
     YCyan,
     YBlue,
     YViolet,
     YPurple,
     YPink,
     YWhite,
     YLight,
     YDark
 };
 
QString getSettingsPath();
QString colorToName(LabelColors colors);
QString labelColorToRGBString(LabelColors color);

}
#endif

