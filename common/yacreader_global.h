#ifndef __YACREADER_GLOBAL_H
#define __YACREADER_GLOBAL_H

#if QT_VERSION >= 0x050000
	#include <QStandardPaths>
#else
	#include <QDesktopServices>
#endif

#include <QWidget>
#include <QMimeData>

#define VERSION "8.0.0"

#define PATH "PATH"
#define MAG_GLASS_SIZE "MAG_GLASS_SIZE"
#define ZOOM_LEVEL "ZOOM_LEVEL"
#define SLIDE_SIZE "SLIDE_SIZE"
#define GO_TO_FLOW_SIZE "GO_TO_FLOW_SIZE"
#define FLOW_TYPE_SW "FLOW_TYPE_SW"
#define FIT "FIT"
#define FITMODE "FITMODE"
#define FLOW_TYPE "FLOW_TYPE"
#define FULLSCREEN "FULLSCREEN"
#define FIT_TO_WIDTH_RATIO "FIT_TO_WIDTH_RATIO"
#define Y_WINDOW_POS "POS"
#define Y_WINDOW_SIZE "SIZE"
#define MAXIMIZED "MAXIMIZED"
#define DOUBLE_PAGE "DOUBLE_PAGE"
#define DOUBLE_MANGA_PAGE "DOUBLE_MANGA_PAGE"
#define ADJUST_TO_FULL_SIZE "ADJUST_TO_FULL_SIZE"
#define BACKGROUND_COLOR "BACKGROUND_COLOR"
#define ALWAYS_ON_TOP "ALWAYS_ON_TOP"
#define SHOW_TOOLBARS "SHOW_TOOLBARS"
#define BRIGHTNESS "BRIGHTNESS"
#define CONTRAST "CONTRAST"
#define GAMMA "GAMMA"
#define SHOW_INFO "SHOW_INFO"

#define FLOW_TYPE_GL "FLOW_TYPE_GL"
#define Y_POSITION "Y_POSITION"
#define COVER_DISTANCE "COVER_DISTANCE"
#define CENTRAL_DISTANCE "CENTRAL_DISTANCE"
#define ZOOM_LEVEL "ZOOM_LEVEL"
#define Z_COVER_OFFSET "Z_COVER_OFFSET"
#define COVER_ROTATION "COVER_ROTATION"
#define FADE_OUT_DIST "FADE_OUT_DIST"
#define LIGHT_STRENGTH "LIGHT_STRENGTH"
#define MAX_ANGLE "MAX_ANGLE"
#define PERFORMANCE "PERFORMANCE"
#define USE_OPEN_GL "USE_OPEN_GL"
#define X_ROTATION "X_ROTATION"
#define Y_COVER_OFFSET "Y_COVER_OFFSET"
#define V_SYNC "V_SYNC"
#define SERVER_ON "SERVER_ON"

#define MAIN_WINDOW_GEOMETRY "MAIN_WINDOW_GEOMETRY"
#define MAIN_WINDOW_STATE "MAIN_WINDOW_STATE"
#define COMICS_VIEW_HEADERS "COMICS_VIEW_HEADERS"
#define COMICS_VIEW_HEADERS_GEOMETRY "COMICS_VIEW_HEADERS_GEOMETRY"
#define COMICS_VIEW_STATUS "COMICS_VIEW_STATUS"
#define COMICS_VIEW_FLOW_SPLITTER_STATUS "COMICS_VIEW_FLOW_SPLITTER_STATUS"
#define SIDEBAR_SPLITTER_STATUS "SIDEBAR_SPLITTER_STATUS"
#define COMICS_GRID_COVER_SIZES "COMICS_GRID_COVER_SIZES"

#define NUM_DAYS_BETWEEN_VERSION_CHECKS "NUM_DAYS_BETWEEN_VERSION_CHECKS"
#define LAST_VERSION_CHECK "LAST_VERSION_CHECK"

#define YACREADERLIBRARY_GUID "ea343ff3-2005-4865-b212-7fa7c43999b8"

#define LIBRARIES "LIBRARIES"

#define COMIC_VINE_API_KEY "COMIC_VINE_API_KEY"

namespace YACReader
{

static const QString YACReaderLibrarComiscSelectionMimeDataFormat = "application/yacreaderlibrary-comics-ids";
static const QString YACReaderLibrarSubReadingListMimeDataFormat = "application/yacreaderlibrary-sublist-rows";

 enum FlowType
  {
	CoverFlowLike=0,
	Strip,
	StripOverlapped,
	Modern,
	Roulette,
	Custom
  };

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

 enum ComicsViewStatus
 {
     Flow,
     Grid
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
 
 enum FitMode{
	ToWidth=0x01,
	ToHeight=0x02,
	FullRes=0x03,
	FullPage=0x04//,
	//Text=0x05
 };

QString getSettingsPath();
void addSperator(QWidget * w);
QAction * createSeparator();
QString colorToName(LabelColors colors);
QIcon noHighlightedIcon(const QString & path);
void colorize(QImage &img, QColor &col);
QString labelColorToRGBString(LabelColors color);
QList<qulonglong> mimeDataToComicsIds(const QMimeData * data);
}
#endif

