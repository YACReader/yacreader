#ifndef __VIEWER_H
#define __VIEWER_H

#include <QMainWindow>

#include <QScrollArea>
#include <QAction>
#include <QTimer>
#include <QLabel>
#include <QPixmap>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QPropertyAnimation>
#include <QSettings>


class Comic;
class MagnifyingGlass;
class GoToFlow;
class BookmarksDialog;
class Render;
class GoToDialog;
class YACReaderTranslator;
class GoToFlowWidget;
class Bookmarks;
class PageLabelWidget; 

	class Viewer : public QScrollArea
	{
		Q_OBJECT
	public:
		bool fullscreen; //TODO, change by the right use of windowState();
	public slots:
		void open(QString pathFile);
		void prev();
		void next();
		void showGoToDialog();
		void goTo(unsigned int page);
		void updatePage();
		void updateContentSize();
		void updateVerticalScrollBar();
		void updateOptions();
		void scrollDown();
		void scrollUp();
		void magnifyingGlassSwitch();
		void showMagnifyingGlass();
		void hideMagnifyingGlass();
		void informationSwitch();
		void updateInformation();
		void goToFlowSwitch();
		void showGoToFlow();
		void moveCursoToGoToFlow();
		void animateShowGoToFlow();
		void animateHideGoToFlow();
		void rotateLeft();
		void rotateRight();
		bool magnifyingGlassIsVisible() {return magnifyingGlassShowed;}
		void setBookmark(bool);
		void save();
        void doublePageSwitch();
		void resetContent();
		void setLoadingMessage();
		void configureContent(QString msg);
		void hideCursor();
		void showCursor();
		void createConnections();
		void translatorSwitch();
		void animateShowTranslator();
		void animateHideTranslator();
virtual void mousePressEvent ( QMouseEvent * event );
virtual void mouseReleaseEvent ( QMouseEvent * event );
		void updateBackgroundColor(const QColor & color);
		void updateFitToWidthRatio(float ratio);
		void updateConfig(QSettings * settings);
		void showMessageErrorOpening();
		void setBookmarks();

	private:
		bool information;
		bool doublePage;
		PageLabelWidget * informationLabel;
		//QTimer * scroller;
		QPropertyAnimation * verticalScroller;
		int posByStep;
		int nextPos;
		GoToFlowWidget * goToFlow;
		QPropertyAnimation * showGoToFlowAnimation;
		GoToDialog * goToDialog;
		//!Image properties
		float adjustToWidthRatio;
		//! Comic
		//Comic * comic;
		int index;
		QPixmap *currentPage;
		BookmarksDialog * bd;
		bool wheelStop;
		Render * render;
		QTimer * hideCursorTimer;
		int direction;
		bool drag;

		//!Widgets
		QLabel *content;

		YACReaderTranslator * translator;
		int translatorXPos;
		QPropertyAnimation * translatorAnimation;

		int yDragOrigin;
		int xDragOrigin;
	private:
		//!Magnifying glass
		MagnifyingGlass *mglass;
		bool magnifyingGlassShowed;
		bool restoreMagnifyingGlass;

		//! Manejadores de evento:
		void keyPressEvent(QKeyEvent * event);
		void resizeEvent(QResizeEvent * event);
		void wheelEvent(QWheelEvent * event);
		void mouseMoveEvent(QMouseEvent * event);

	public:
		Viewer(QWidget * parent = 0);
		void toggleFullScreen();
		const QPixmap * pixmap();
		//Comic * getComic(){return comic;}
		const BookmarksDialog * getBookmarksDialog(){return bd;}
	signals:
		void backgroundChanges();
		void pageAvailable(bool);
		void pageIsBookmark(bool);
		void reset();
	};

#endif
