#include "viewer.h"
#include "magnifying_glass.h"
#include "configuration.h"
#include "magnifying_glass.h"
#include "goto_flow.h"
#include "bookmarks_dialog.h"
#include "render.h"
#include "goto_dialog.h"
#include "translator.h"

#include <QWebView>
#include <QFile>
#define STEPS 22



Viewer::Viewer(QWidget * parent)
:QScrollArea(parent),
currentPage(0),
magnifyingGlassShowed(false),
fullscreen(false),
information(false),
adjustToWidthRatio(1),
doublePage(false),
wheelStop(false),
direction(1),
restoreMagnifyingGlass(false),
drag(false)

{
	translator = new YACReaderTranslator(this);
	translator->hide();
	translatorAnimation = new QPropertyAnimation(translator,"pos");
	translatorAnimation->setDuration(150);
	translatorXPos = -10000;
	translator->move(-translator->width(),10);
	//current comic page
	content = new QLabel(this);
	configureContent(tr("Press 'O' to open comic."));
	//scroll area configuration
	setBackgroundRole(QPalette::Dark);
	setWidget(content);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignCenter);

	QPalette palette;
	palette.setColor(backgroundRole(), Configuration::getConfiguration().getBackgroundColor());
	setPalette(palette);
	//---------------------------------------
	mglass = new MagnifyingGlass(Configuration::getConfiguration().getMagnifyingGlassSize(),this);
	mglass->hide();
	content->setMouseTracking(true);
	setMouseTracking(true);

	informationLabel = new QLabel(this);
	informationLabel->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
	informationLabel->setAutoFillBackground(true);
	informationLabel->setFont(QFont("courier new", 12));
	informationLabel->hide();
	informationLabel->resize(100,25);

	showCursor();

	goToDialog = new GoToDialog(this);

	goToFlow = new GoToFlow(this,Configuration::getConfiguration().getFlowType());
	goToFlow->hide();
	showGoToFlowAnimation = new QPropertyAnimation(goToFlow,"pos");
	showGoToFlowAnimation->setDuration(150);
	
	bd = new BookmarksDialog(this->parentWidget());
	
	render = new Render();

	scroller = new QTimer(this);

	hideCursorTimer = new QTimer();
	hideCursorTimer->setSingleShot(true); 

	if(Configuration::getConfiguration().getDoublePage())
		doublePageSwitch();

	createConnections();

	hideCursorTimer->start(2500);

	setMouseTracking(true);
}

void Viewer::createConnections()
{
	//magnifyingGlass (update mg after a background change
	connect(this,SIGNAL(backgroundChanges()),mglass,SLOT(updateImage()));

	//goToDialog
	connect(goToDialog,SIGNAL(goToPage(unsigned int)),this,SLOT(goTo(unsigned int)));

	//goToFlow goTo
	connect(goToFlow,SIGNAL(goToPage(unsigned int)),this,SLOT(goTo(unsigned int)));

	//current time
	QTimer * t = new QTimer();
	connect(t,SIGNAL(timeout()),this,SLOT(updateInformation()));
	t->start(1000);

	//hide cursor
	connect(hideCursorTimer,SIGNAL(timeout()),this,SLOT(hideCursor()));

	//bookmarks
	connect(bd,SIGNAL(goToPage(unsigned int)),this,SLOT(goTo(unsigned int)));

	//render
	connect(render,SIGNAL(errorOpening()),this,SLOT(resetContent()));
	connect(render,SIGNAL(numPages(unsigned int)),goToFlow,SLOT(setNumSlides(unsigned int)));
	connect(render,SIGNAL(numPages(unsigned int)),goToDialog,SLOT(setNumPages(unsigned int)));
	connect(render,SIGNAL(imageLoaded(int,QByteArray)),goToFlow,SLOT(setImageReady(int,QByteArray)));
	connect(render,SIGNAL(currentPageReady()),this,SLOT(updatePage()));
	connect(render,SIGNAL(processingPage()),this,SLOT(setLoadingMessage()));
	connect(render,SIGNAL(currentPageIsBookmark(bool)),this,SIGNAL(pageIsBookmark(bool)));
	connect(render,SIGNAL(bookmarksLoaded(const Bookmarks &)),bd,SLOT(setBookmarks(const Bookmarks &)));
}

void Viewer::open(QString pathFile)
{
	if(render->hasLoadedComic())
		save();
	//bd->setBookmarks(*bm);

	goToFlow->reset();
	render->load(pathFile);
	//render->update();

	verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum());
}

void Viewer::next()
{
	direction = 1;
	render->nextPage();
}

void Viewer::prev()
{
	direction = -1;
	render->previousPage();
}
void Viewer::showGoToDialog()
{
	goToDialog->show();
}
void Viewer::goTo(unsigned int page)
{
	direction = 1; //in "go to" direction is always fordward
	render->goTo(page-1);
}

void Viewer::updatePage()
{
	QPixmap * previousPage = currentPage;
	currentPage = render->getCurrentPage();
	content->setPixmap(*currentPage);
	updateContentSize();
	updateVerticalScrollBar();
	emit backgroundChanges();
	emit(pageAvailable(true));
	//TODO -> update bookmark action
	setFocus(Qt::ShortcutFocusReason);
	delete previousPage;
	if(restoreMagnifyingGlass)
	{
		restoreMagnifyingGlass = false;
		showMagnifyingGlass();
	}

}

void Viewer::updateContentSize()
{
	//there is an image to resize
	if(currentPage !=0 && !currentPage->isNull())
	{
		if(Configuration::getConfiguration().getAdjustToFullSize())
		{
			content->resize(currentPage->width(),currentPage->height());
		}
		else
		{
			float aspectRatio = (float)currentPage->width()/currentPage->height();
			//Fit to width
			if(Configuration::getConfiguration().getAdjustToWidth())
			{
				adjustToWidthRatio = Configuration::getConfiguration().getFitToWidthRatio();
				if(static_cast<int>(width()*adjustToWidthRatio/aspectRatio)<height())
					if(static_cast<int>(height()*aspectRatio)>width())
						content->resize(width(),static_cast<int>(width()/aspectRatio));
					else
						content->resize(static_cast<int>(height()*aspectRatio),height());
				else
					content->resize(width()*adjustToWidthRatio,static_cast<int>(width()*adjustToWidthRatio/aspectRatio));
			}
			//Fit to height or fullsize/custom size
			else
			{
				if(static_cast<int>(height()*aspectRatio)>width()) //page width exceeds window width
					content->resize(width(),static_cast<int>(width()/aspectRatio));
				else
					content->resize(static_cast<int>(height()*aspectRatio),height());
			}
		}
		emit backgroundChanges();
	}
	content->update(); //TODO, it shouldn't be neccesary
}

void Viewer::updateVerticalScrollBar()
{
	if(direction > 0)
		verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum());
	else
		verticalScrollBar()->setSliderPosition(verticalScrollBar()->maximum());
}

void Viewer::scrollDown()
{
	if(verticalScrollBar()->sliderPosition()==verticalScrollBar()->maximum())
	{
		next();
		scroller->stop();
	}
	else
	{
		int currentPos = verticalScrollBar()->sliderPosition();
		verticalScrollBar()->setSliderPosition(currentPos=currentPos+posByStep);
		if((verticalScrollBar()->sliderPosition()==verticalScrollBar()->maximum())
			||(verticalScrollBar()->sliderPosition()>=nextPos))
			scroller->stop();
		emit backgroundChanges();
	}
}

void Viewer::scrollUp()
{
	if(verticalScrollBar()->sliderPosition()==verticalScrollBar()->minimum())
	{
		prev();
		scroller->stop();
	}
	else
	{
		int currentPos = verticalScrollBar()->sliderPosition();
		verticalScrollBar()->setSliderPosition(currentPos=currentPos-posByStep);
		if((verticalScrollBar()->sliderPosition()==verticalScrollBar()->minimum())
			||(verticalScrollBar()->sliderPosition()<=nextPos))
			scroller->stop();
		emit backgroundChanges();
	}
}

void Viewer::keyPressEvent(QKeyEvent *event)
{
	if(render->hasLoadedComic())
	{
		if(goToFlow->isVisible() && event->key()!=Qt::Key_S)
			QCoreApplication::sendEvent(goToFlow,event);
		else
			switch (event->key())
		{
			case Qt::Key_Space:
				disconnect(scroller,SIGNAL(timeout()),this,0);
				connect(scroller,SIGNAL(timeout()),this,SLOT(scrollDown()));
				posByStep = height()/STEPS;
				nextPos=verticalScrollBar()->sliderPosition()+static_cast<int>((height()*0.80));
				scroller->start(20);
				break;
			case Qt::Key_B:
				disconnect(scroller,SIGNAL(timeout()),this,0);
				connect(scroller,SIGNAL(timeout()),this,SLOT(scrollUp()));
				posByStep = height()/STEPS;
				nextPos=verticalScrollBar()->sliderPosition()-static_cast<int>((height()*0.80));
				scroller->start(20);
				break;
			case Qt::Key_S:
				goToFlowSwitch();
				break;
			case Qt::Key_T:
				translatorSwitch();
				break;
			case Qt::Key_Down:
				/*if(verticalScrollBar()->sliderPosition()==verticalScrollBar()->maximum())
				next();
				else*/
				QAbstractScrollArea::keyPressEvent(event);
				emit backgroundChanges();
				break;
			case Qt::Key_Up:
				/*if(verticalScrollBar()->sliderPosition()==verticalScrollBar()->minimum())
				prev();
				else*/
				QAbstractScrollArea::keyPressEvent(event);
				emit backgroundChanges();
				break;
			case Qt::Key_Home:
				goTo(1);
				break;
			case Qt::Key_End:
				goTo(this->render->numPages());
				break;
			default:
				QAbstractScrollArea::keyPressEvent(event);
				break;
		}
		if(mglass->isVisible())
			switch(event->key())
		{
			case Qt::Key_Plus: case Qt::Key_Minus: case Qt::Key_Underscore: case Qt::Key_Asterisk:
				QCoreApplication::sendEvent(mglass,event);
		}
	}
}

void Viewer::wheelEvent(QWheelEvent * event)
{
    if(render->hasLoadedComic())
    {
	if((event->delta()<0)&&(verticalScrollBar()->sliderPosition()==verticalScrollBar()->maximum()))
	{
		if(wheelStop)
		{
			next();
			event->accept();
			wheelStop = false;
			return;
		}
		else
			wheelStop = true;
	}
	else
		if((event->delta()>0)&&(verticalScrollBar()->sliderPosition()==verticalScrollBar()->minimum()))
		{
		    if(wheelStop)
			{
				prev();
				event->accept();
				wheelStop = false;
				return;
			}
			else
				wheelStop = true;
		}

		QAbstractScrollArea::wheelEvent(event);
		emit backgroundChanges();
    }
}

void Viewer::resizeEvent(QResizeEvent * event)
{
	updateContentSize();
	goToFlow->move(QPoint((width()-goToFlow->width())/2,height()-goToFlow->height()));
	informationLabel->move(QPoint((width()-informationLabel->width())/2,0));
	QScrollArea::resizeEvent(event);
}

void Viewer::mouseMoveEvent(QMouseEvent * event)
{
	showCursor();
	hideCursorTimer->start(2500);

	if(magnifyingGlassShowed)
		mglass->move(static_cast<int>(event->x()-float(mglass->width())/2),static_cast<int>(event->y()-float(mglass->height())/2));

	if(render->hasLoadedComic())
	{
		if(showGoToFlowAnimation->state()!=QPropertyAnimation::Running)
		{
		if(goToFlow->isVisible())
		{
			animateHideGoToFlow();
			//goToFlow->hide();
		}
		else
		{
			int umbral = (width()-goToFlow->width())/2;
			if((event->y()>height()-15)&&(event->x()>umbral)&&(event->x()<width()-umbral))
			{

				animateShowGoToFlow();
				hideCursorTimer->stop();
			}
		}
		}

		if(drag)
		{
			int currentPosY = verticalScrollBar()->sliderPosition();
			int currentPosX = horizontalScrollBar()->sliderPosition();
			verticalScrollBar()->setSliderPosition(currentPosY=currentPosY+(yDragOrigin-event->y()));
			horizontalScrollBar()->setSliderPosition(currentPosX=currentPosX+(xDragOrigin-event->x()));
			yDragOrigin = event->y();
			xDragOrigin = event->x();
		}
	}


}

const QPixmap * Viewer::pixmap()
{
	return content->pixmap();
}

void Viewer::magnifyingGlassSwitch()
{
	magnifyingGlassShowed?hideMagnifyingGlass():showMagnifyingGlass();
}

void Viewer::showMagnifyingGlass()
{
	if(render->hasLoadedComic())
	{
		QPoint p = QPoint(cursor().pos().x(),cursor().pos().y());
		p = this->parentWidget()->mapFromGlobal(p);
		mglass->move(static_cast<int>(p.x()-float(mglass->width())/2)
			,static_cast<int>(p.y()-float(mglass->height())/2));
		mglass->show();
		mglass->updateImage(mglass->x()+mglass->width()/2,mglass->y()+mglass->height()/2);
		magnifyingGlassShowed = true;
	}
}

void Viewer::hideMagnifyingGlass()
{
	mglass->hide();
	magnifyingGlassShowed = false;
}

void Viewer::informationSwitch()
{
	information?informationLabel->hide():informationLabel->show();
	informationLabel->move(QPoint((width()-informationLabel->width())/2,0));
	information=!information;
	//TODO it shouldn't be neccesary
	informationLabel->adjustSize();
	informationLabel->update();
}

void Viewer::updateInformation()
{
	if(render->hasLoadedComic())
	{
		informationLabel->setText(render->getCurrentPagesInformation()+" - "+QTime::currentTime().toString("HH:mm"));
		informationLabel->adjustSize();
		informationLabel->update(); //TODO it shouldn't be neccesary
	}
}

void Viewer::goToFlowSwitch()
{
	goToFlow->isVisible()?animateHideGoToFlow():showGoToFlow();
}

void Viewer::translatorSwitch()
{
	translator->isVisible()?animateHideTranslator():animateShowTranslator();
}

void Viewer::showGoToFlow()
{
	if(render->hasLoadedComic())
	{
		animateShowGoToFlow();
	}
}

void Viewer::animateShowGoToFlow()
{
	if(goToFlow->isHidden() && showGoToFlowAnimation->state()!=QPropertyAnimation::Running)
	{
		disconnect(showGoToFlowAnimation,SIGNAL(finished()),goToFlow,SLOT(hide()));
		connect(showGoToFlowAnimation,SIGNAL(finished()),this,SLOT(moveCursoToGoToFlow()));
		showGoToFlowAnimation->setStartValue(QPoint((width()-goToFlow->width())/2,height()-10));
		showGoToFlowAnimation->setEndValue(QPoint((width()-goToFlow->width())/2,height()-goToFlow->height()));
		showGoToFlowAnimation->start();
		goToFlow->centerSlide(render->getIndex());
		goToFlow->setPageNumber(render->getIndex());
		goToFlow->show();
		goToFlow->setFocus(Qt::OtherFocusReason);
	}
}

void Viewer::animateHideGoToFlow()
{
	if(goToFlow->isVisible() && showGoToFlowAnimation->state()!=QPropertyAnimation::Running)
	{
		connect(showGoToFlowAnimation,SIGNAL(finished()),goToFlow,SLOT(hide()));
		disconnect(showGoToFlowAnimation,SIGNAL(finished()),this,SLOT(moveCursoToGoToFlow()));
		showGoToFlowAnimation->setStartValue(QPoint((width()-goToFlow->width())/2,height()-goToFlow->height()));
		showGoToFlowAnimation->setEndValue(QPoint((width()-goToFlow->width())/2,height()));
		showGoToFlowAnimation->start();
		goToFlow->centerSlide(render->getIndex());
		goToFlow->setPageNumber(render->getIndex());
		this->setFocus(Qt::OtherFocusReason);
	}
}

void Viewer::moveCursoToGoToFlow()
{
		//Move cursor to goToFlow widget on show (this avoid hide when mouse is moved)
		int y = goToFlow->pos().y();
		int x1 = goToFlow->pos().x();
		int x2 = x1 + goToFlow->width();
		QPoint cursorPos = mapFromGlobal(cursor().pos());
		int cursorX = cursorPos.x();
		int cursorY = cursorPos.y();

		if(cursorY <= y)
			cursorY = y + 10;
		if(cursorX <= x1)
			cursorX = x1 + 10;
		if(cursorX >= x2)
			cursorX = x2 - 10;
		cursor().setPos(mapToGlobal(QPoint(cursorX,cursorY)));
		hideCursorTimer->stop();
		showCursor();
}

void Viewer::rotateLeft()
{
	render->rotateLeft();
}
void Viewer::rotateRight()
{
	render->rotateRight();
}

//TODO
void Viewer::setBookmark(bool set)
{
    render->setBookmark();
    if(set) //add bookmark
    {
	render->setBookmark();
    }
    else //remove bookmark
    {
	render->removeBookmark();
    }
}

void Viewer::save ()
{
    if(render->hasLoadedComic())
	render->save();
}

void Viewer::doublePageSwitch()
{
    doublePage = !doublePage;
    render->doublePageSwitch();
	Configuration::getConfiguration().setDoublePage(doublePage);
}

void Viewer::resetContent()
{
    configureContent(tr("Press 'O' to open comic."));
	goToFlow->reset();
    emit reset();
}

void Viewer::setLoadingMessage()
{
	if(magnifyingGlassShowed)
	{
		hideMagnifyingGlass();
		restoreMagnifyingGlass = true;
	}
	emit(pageAvailable(false));
	configureContent(tr("Loading...please wait!"));
}

void Viewer::configureContent(QString msg)
{
    content->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    content->setScaledContents(true);
    content->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    content->setText(msg);
    content->setFont(QFont("courier new", 12));
    content->adjustSize();
	setFocus(Qt::ShortcutFocusReason);
	//emit showingText();
}

void Viewer::hideCursor()
{
#ifdef Q_WS_MAC
	setCursor(QCursor(QBitmap(1,1),QBitmap(1,1)));
#else
	setCursor(Qt::BlankCursor);
#endif
}
void Viewer::showCursor()
{
	if(drag)
		setCursor(Qt::ClosedHandCursor);
	else
		setCursor(Qt::OpenHandCursor);
}

void Viewer::updateOptions()
{
	
	goToFlow->setFlowType(Configuration::getConfiguration().getFlowType());
	updateBackgroundColor(Configuration::getConfiguration().getBackgroundColor());
	updateContentSize();
	//goToFlow->updateSize();
}

void Viewer::updateBackgroundColor(const QColor & color)
{
	QPalette palette;
	palette.setColor(backgroundRole(), color);
	setPalette(palette);
}

void Viewer::animateShowTranslator()
{
	if(translator->isHidden() && translatorAnimation->state()!=QPropertyAnimation::Running)
	{
		disconnect(translatorAnimation,SIGNAL(finished()),translator,SLOT(hide()));
		if(translatorXPos == -10000)
			translatorXPos = (width()-translator->width())/2;
		int x = qMax(0,qMin(translatorXPos,width()-translator->width()));
		if(translator->pos().x()<0)
		{
			translatorAnimation->setStartValue(QPoint(-translator->width(),translator->pos().y()));
		}
		else
		{
			translatorAnimation->setStartValue(QPoint(width()+translator->width(),translator->pos().y()));
		}
		translatorAnimation->setEndValue(QPoint(x,translator->pos().y()));
		translatorAnimation->start();
		translator->show();
		translator->setFocus(Qt::OtherFocusReason);
	}
}
void Viewer::animateHideTranslator()
{
	if(translator->isVisible() && translatorAnimation->state()!=QPropertyAnimation::Running)
	{
		connect(translatorAnimation,SIGNAL(finished()),translator,SLOT(hide()));
		translatorAnimation->setStartValue(QPoint(translatorXPos = translator->pos().x(),translator->pos().y()));
		if((translator->width()/2)+translator->pos().x() <= width()/2)
			translatorAnimation->setEndValue(QPoint(-translator->width(),translator->pos().y()));
		else
			translatorAnimation->setEndValue(QPoint(width()+translator->width(),translator->pos().y()));
		translatorAnimation->start();
		this->setFocus(Qt::OtherFocusReason);
	}
}

void Viewer::mousePressEvent ( QMouseEvent * event )
{
	drag = true;
	yDragOrigin = event->y();
	xDragOrigin = event->x();
	setCursor(Qt::ClosedHandCursor);
}
void Viewer::mouseReleaseEvent ( QMouseEvent * event )
{
	drag = false;
	setCursor(Qt::OpenHandCursor);
}