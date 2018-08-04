#ifndef __YACREADER_GLOBAL_H
#define __YACREADER_GLOBAL_H

#include <QStandardPaths>

#define VERSION "9.5.0"

#define USE_BACKGROUND_IMAGE_IN_GRID_VIEW "USE_BACKGROUND_IMAGE_IN_GRID_VIEW"
#define OPACITY_BACKGROUND_IMAGE_IN_GRID_VIEW "OPACITY_BACKGROUND_IMAGE_IN_GRID_VIEW"
#define BLUR_RADIUS_BACKGROUND_IMAGE_IN_GRID_VIEW "BLUR_RADIUS_BACKGROUND_IMAGE_IN_GRID_VIEW"
#define USE_SELECTED_COMIC_COVER_AS_BACKGROUND_IMAGE_IN_GRID_VIEW "USE_SELECTED_COMIC_COVER_AS_BACKGROUND_IMAGE_IN_GRID_VIEW"

#define REMOTE_BROWSE_PERFORMANCE_WORKAROUND "REMOTE_BROWSE_PERFORMANCE_WORKAROUND"

#define NUM_DAYS_BETWEEN_VERSION_CHECKS "NUM_DAYS_BETWEEN_VERSION_CHECKS"
#define LAST_VERSION_CHECK "LAST_VERSION_CHECK"

#define YACREADERLIBRARY_GUID "ea343ff3-2005-4865-b212-7fa7c43999b8"

#define LIBRARIES "LIBRARIES"

#define MAX_LIBRARIES_WARNING_NUM 10

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
