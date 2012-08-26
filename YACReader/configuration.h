#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H
#include <QString>
#include <QSize>
#include <QObject>
#include "pictureflow.h"

#define CONF_FILE_PATH "."
#define SLIDE_ASPECT_RATIO 1.585

	class Configuration : public QObject
	{
		Q_OBJECT
		
	private:
		QString defaultPath;
		//configuration properties
		QSize magnifyingGlassSize;
		QSize gotoSlideSize;
		float zoomLevel;
		bool adjustToWidth;
		bool fullScreen;
		PictureFlow::FlowType flowType;
		float fitToWidthRatio;
		QPoint windowPos;
		QSize windowSize;
		bool maximized;
		bool doublePage;
		bool alwaysOnTop;
		bool adjustToFullSize;
		QColor backgroundColor;

		Configuration();
		Configuration(const Configuration & conf);
		void load(const QString & path = CONF_FILE_PATH);
		
	public:
		static Configuration & getConfiguration()
		{
			static Configuration configuration;
			return configuration;
		};
		QString getDefaultPath() { return defaultPath; };
		void setDefaultPath(QString defaultPath){this->defaultPath = defaultPath;};
		QSize getMagnifyingGlassSize() { return magnifyingGlassSize;};
		void setMagnifyingGlassSize(const QSize & mgs) { magnifyingGlassSize = mgs;};
		QSize getGotoSlideSize() { return gotoSlideSize;};
		void setGotoSlideSize(const QSize & gss) { gotoSlideSize = gss;};
		float getZoomLevel() { return zoomLevel;};
		void setZoomLevel(float zl) { zoomLevel = zl;};
		bool getAdjustToWidth() {return adjustToWidth;};
		void setAdjustToWidth(bool atw=true) {adjustToWidth = atw;};
		PictureFlow::FlowType getFlowType(){return flowType;};
		void setFlowType(PictureFlow::FlowType type){flowType = type;};
		bool getFullScreen(){return fullScreen;};
		void setFullScreen(bool f){fullScreen = f;};
		float getFitToWidthRatio(){return fitToWidthRatio;};
		void setFitToWidthRatio(float r){fitToWidthRatio = r;};
		QPoint getPos(){return windowPos;};
		void setPos(QPoint p){windowPos = p;};
		QSize getSize(){return windowSize;};
		void setSize(QSize s){windowSize = s;};
		bool getMaximized(){return maximized;};
		void setMaximized(bool b){maximized = b;};
		bool getDoublePage(){return doublePage;};
		void setDoublePage(bool b){doublePage = b;};
		void setAdjustToFullSize(bool b){adjustToFullSize = b;};
		bool getAdjustToFullSize(){return adjustToFullSize;};
		void setBackgroundColor(const QColor& color){backgroundColor = color;};
		QColor getBackgroundColor(){return backgroundColor;};
		void setAlwaysOnTop(bool b){alwaysOnTop = b;};
		bool getAlwaysOnTop(){return alwaysOnTop;};
		void save();

	};

#endif
