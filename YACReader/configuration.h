#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H
#include <QString>
#include <QSize>
#include <QObject>
#include <QPoint>
#include <QColor>
#include <QSettings>
#include <QDate>

#include "yacreader_global_gui.h"

#define CONF_FILE_PATH "."
#define SLIDE_ASPECT_RATIO 1.585

using namespace YACReader;

	class Configuration : public QObject
	{
		Q_OBJECT

	private:
		QSettings * settings;

		QString defaultPath;
		//configuration properties
		QSize magnifyingGlassSize;
		QSize gotoSlideSize;
		float zoomLevel;
		bool adjustToWidth;
		bool fullScreen;
		FlowType flowType;
		float fitToWidthRatio;
		QPoint windowPos;
		QSize windowSize;
		bool maximized;
		bool doublePage;
		bool doubleMangaPage;
		bool alwaysOnTop;
		bool adjustToFullSize;
		QColor backgroundColor;

		Configuration();
		//Configuration(const Configuration & conf);
		void load(const QString & path = CONF_FILE_PATH);


	public:
		static Configuration & getConfiguration()
		{
			static Configuration configuration;
			return configuration;
		};
        QSettings *getSettings();
		void load(QSettings * settings);
		QString getDefaultPath() { return settings->value(PATH).toString(); }
		void setDefaultPath(QString defaultPath){settings->setValue(PATH,defaultPath);}
		QSize getMagnifyingGlassSize() { return settings->value(MAG_GLASS_SIZE).toSize();}
		void setMagnifyingGlassSize(const QSize & mgs) { settings->setValue(MAG_GLASS_SIZE,mgs);}
		QSize getGotoSlideSize() { return settings->value(GO_TO_FLOW_SIZE).toSize();}
		void setGotoSlideSize(const QSize & gss) { settings->setValue(GO_TO_FLOW_SIZE,gss);}
		float getZoomLevel() { return settings->value(ZOOM_LEVEL).toFloat();}
		void setZoomLevel(float zl) { settings->setValue(ZOOM_LEVEL,zl);}

		//Unified enum based fitmode
		YACReader::FitMode getFitMode() { return static_cast<YACReader::FitMode>(settings->value(FITMODE, YACReader::FitMode::FullPage).toInt()); }
		void setFitMode ( YACReader::FitMode fitMode ){ settings->setValue(FITMODE, static_cast<int>(fitMode)); }

		//openRecent
		int getOpenRecentSize() { return settings->value("recentSize", 25).toInt();}
		QStringList openRecentList() { return settings->value("recentFiles").toStringList(); }
		void updateOpenRecentList (QString path);
		void clearOpenRecentList() { settings->remove("recentFiles"); }

		//Old fitmodes
		/*
		bool getAdjustToWidth() {return settings->value(FIT).toBool();}
		void setAdjustToWidth(bool atw=true) {settings->setValue(FIT,atw);}
		float getFitToWidthRatio(){return settings->value(FIT_TO_WIDTH_RATIO).toFloat();}
		void setFitToWidthRatio(float r){settings->setValue(FIT_TO_WIDTH_RATIO,r);}
		bool getAdjustToFullSize(){return settings->value(ADJUST_TO_FULL_SIZE).toBool();}
		void setAdjustToFullSize(bool b){settings->setValue(ADJUST_TO_FULL_SIZE,b);}
		*/

		FlowType getFlowType(){return (FlowType)settings->value(FLOW_TYPE_SW).toInt();}
		void setFlowType(FlowType type){settings->setValue(FLOW_TYPE_SW,type);}
		bool getFullScreen(){return settings->value(FULLSCREEN).toBool();}
		void setFullScreen(bool f){settings->setValue(FULLSCREEN,f);}

		QPoint getPos(){return settings->value(Y_WINDOW_POS).toPoint();}
		void setPos(QPoint p){settings->setValue(Y_WINDOW_POS,p);}
		QSize getSize(){return settings->value(Y_WINDOW_SIZE).toSize();}
		void setSize(QSize s){settings->setValue(Y_WINDOW_SIZE,s);}
		bool getMaximized(){return settings->value(MAXIMIZED).toBool();}
		void setMaximized(bool b){settings->setValue(MAXIMIZED,b);}
		bool getDoublePage(){return settings->value(DOUBLE_PAGE).toBool();}
		void setDoublePage(bool b){settings->setValue(DOUBLE_PAGE,b);}
		bool getDoubleMangaPage(){return settings->value(DOUBLE_MANGA_PAGE).toBool();}
		void setDoubleMangaPage(bool b){settings->setValue(DOUBLE_MANGA_PAGE,b);}

		QColor getBackgroundColor(){return settings->value(BACKGROUND_COLOR).value<QColor>();}
		void setBackgroundColor(const QColor& color){settings->value(BACKGROUND_COLOR,color);}
		bool getAlwaysOnTop(){return settings->value(ALWAYS_ON_TOP).toBool();}
		void setAlwaysOnTop(bool b){ settings->setValue(ALWAYS_ON_TOP,b);}
		bool getShowToolbars(){return settings->value(SHOW_TOOLBARS).toBool();}
		void setShowToolbars(bool b){settings->setValue(SHOW_TOOLBARS,b);}
		bool getShowInformation(){return settings->value(SHOW_INFO,false).toBool();}
		void setShowInformation(bool b){settings->setValue(SHOW_INFO,b);}
		QDate getLastVersionCheck(){return settings->value(LAST_VERSION_CHECK).toDate();}
		void setLastVersionCheck(const QDate & date){ settings->setValue(LAST_VERSION_CHECK,date);}
		int getNumDaysBetweenVersionChecks() {return settings->value(NUM_DAYS_BETWEEN_VERSION_CHECKS,1).toInt();}
		void setNumDaysBetweenVersionChecks(int days) {return settings->setValue(NUM_DAYS_BETWEEN_VERSION_CHECKS,days);}
		bool getQuickNaviMode(){return settings->value(QUICK_NAVI_MODE).toBool();}
        bool getDisableShowOnMouseOver(){return settings->value(DISABLE_MOUSE_OVER_GOTO_FLOW).toBool();}
	};

#endif
