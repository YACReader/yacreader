#include "import_widget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QScrollBar>
#include <QGraphicsItemAnimation>
#include <QTimeLine>
//TODO: is QGLWidget needed here???
//#include <QGLWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QToolButton>
#include <QResizeEvent>

#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class YACReaderActivityIndicatorWidget : public QWidget
{
public:
	YACReaderActivityIndicatorWidget(QWidget * parent = 0);
public slots:

private:
	QLabel * normal;
	QLabel * glow;
};

YACReaderActivityIndicatorWidget::YACReaderActivityIndicatorWidget(QWidget * parent)
	:QWidget(parent)
{
	QPixmap line(":/images/noLibrariesLine.png");
	QPixmap glowLine(":/images/glowLine.png");
	normal = new QLabel(this);
	glow = new QLabel(this);

	normal->setPixmap(line);
	glow->setPixmap(glowLine);

	QHBoxLayout * layout = new QHBoxLayout();

	layout->addWidget(normal,0,Qt::AlignVCenter);

	setLayout(layout);

	layout->setMargin(4);
	layout->setSpacing(0);

	//setFixedHeight(3);
	//resize(579,3);
	glow->setGeometry(4,4,glowLine.width(),glowLine.height());
	//normal->setGeometry(0,1,579,1);

	QGraphicsOpacityEffect * effect = new QGraphicsOpacityEffect();
	//effect->setOpacity(1.0);


	QPropertyAnimation * animation = new QPropertyAnimation(effect,"opacity");

	animation->setDuration(1000);
	animation->setStartValue(1);
	animation->setEndValue(0);
	//animation->setEasingCurve(QEasingCurve::InQuint);

	QPropertyAnimation * animation2 = new QPropertyAnimation(effect,"opacity");

	animation2->setDuration(1000);
	animation2->setStartValue(0);
	animation2->setEndValue(1);
	//animation2->setEasingCurve(QEasingCurve::InQuint);

	glow->setGraphicsEffect(effect);

	connect(animation,SIGNAL(finished()),animation2,SLOT(start()));
	connect(animation2,SIGNAL(finished()),animation,SLOT(start()));

	animation->start();
}




ImportWidget::ImportWidget(QWidget *parent) :
	QWidget(parent)
{
		setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	QPalette p(palette());
	p.setColor(QPalette::Background, QColor(250,250,250));
	setAutoFillBackground(true);
	setPalette(p);

	QPixmap icon(":/images/importingIcon.png");
	iconLabel  = new QLabel();
	iconLabel->setPixmap(icon);

	/*QPixmap line(":/images/noLibrariesLine.png");
	QLabel * lineLabel = new QLabel();
	lineLabel->setPixmap(line);*/

	YACReaderActivityIndicatorWidget * activityIndicator = new YACReaderActivityIndicatorWidget();

	text = new QLabel();//"<font color=\"#495252\">"+tr("Importing comics")+"</font>");
	text->setStyleSheet("QLabel {font-size:25px;font-weight:bold;}");
	textDescription = new QLabel();//"<font color=\"#565959\">"+tr("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.")+"</font>");
	textDescription->setWordWrap(true);
	textDescription->setMaximumWidth(330);
	currentComicLabel = new QLabel("<font color=\"#565959\">...</font>");

	coversViewContainer = new QWidget(this);
	QVBoxLayout * coversViewLayout = new QVBoxLayout;
	coversViewContainer->setLayout(coversViewLayout);
    coversViewContainer->setMaximumHeight(316);
    coversViewContainer->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Maximum);

	coversView = new QGraphicsView();
	//coversView->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
	coversView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	coversView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	coversView->setMaximumHeight(300);
	coversView->setStyleSheet("QGraphicsView {background-color: #E6E6E6;border:none;}");

    coversScene = new QGraphicsScene();
	coversView->setAlignment(Qt::AlignLeft);
    coversView->setScene(coversScene);
    coversView->setFixedHeight(300);

    coversView->setInteractive(false);

    scrollAnimation = new QPropertyAnimation(coversView->horizontalScrollBar(), "value");

	QLabel * topDecorator = new QLabel();
	QLabel * bottomDecorator = new QLabel();
	QPixmap top(":/images/importTopCoversDecoration.png");
	QPixmap bottom(":/images/importBottomCoversDecoration.png");
	topDecorator->setPixmap(top);
	bottomDecorator->setPixmap(bottom);
	topDecorator->setScaledContents(true);
	bottomDecorator->setScaledContents(true);
	topDecorator->setFixedHeight(top.height());
	bottomDecorator->setFixedHeight(bottom.height());

	coversViewLayout->addWidget(topDecorator,0);
	coversViewLayout->addWidget(coversView,1);
	coversViewLayout->addWidget(bottomDecorator,0);
	coversViewLayout->setMargin(0);
	coversViewLayout->setSpacing(0);

	QPushButton * stop = new QPushButton(tr("stop"));
	stop->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    stopButton = stop;

	QVBoxLayout * layout = new QVBoxLayout(this);
	QHBoxLayout * buttonLayout = new QHBoxLayout();
	QHBoxLayout * topLayout = new QHBoxLayout();
	QVBoxLayout * textLayout = new QVBoxLayout();

	QWidget * topWidget = new QWidget();
	topWidget->setFixedWidth(650);
	textLayout->addStretch();
	textLayout->addWidget(text);
	textLayout->addSpacing(12);
	textLayout->addWidget(textDescription);
	textLayout->addStretch();

	topLayout->addStretch();
	topLayout->addWidget(iconLabel,0,Qt::AlignVCenter);
	topLayout->addSpacing(30);
	topLayout->addLayout(textLayout,1);
	topLayout->addStretch();
	topLayout->setMargin(0);

	topWidget->setLayout(topLayout);

	layout->setAlignment(Qt::AlignHCenter);

	buttonLayout->addSpacing(250);
	buttonLayout->addWidget(stop);
	buttonLayout->addSpacing(250);

	layout->addSpacing(50);
	layout->addWidget(topWidget,0,Qt::AlignHCenter);
	layout->addSpacing(20);
	layout->addWidget(activityIndicator,0,Qt::AlignHCenter);
	layout->addSpacing(10);
	layout->addLayout(buttonLayout,0);
	layout->addSpacing(10);
	layout->addStretch();
    coversLabel = new QLabel("<font color=\"#565959\">"+tr("Some of the comics being added...")+"</font>");
	
	hideButton = new QToolButton(this);
	hideButton->setFixedSize(25,18);
	hideButton->setStyleSheet("QToolButton {background: url(\":/images/shownCovers.png\"); border:none;}"
		"  QToolButton:checked {background:url(\":/images/hiddenCovers.png\"); border:none;}");
	hideButton->setCheckable(true);

	connect(hideButton,SIGNAL(toggled(bool)),this,SLOT(showCovers(bool)));

    layout->addWidget(coversLabel,0,Qt::AlignHCenter);
	layout->addWidget(coversViewContainer);
	//layout->addStretch();
	layout->addWidget(currentComicLabel,0,Qt::AlignHCenter);
	layout->setContentsMargins(0,layout->contentsMargins().top(),0,layout->contentsMargins().bottom());

	connect(stop,SIGNAL(clicked()),this,SIGNAL(stop()));
	//connect(stop,SIGNAL(clicked()),this,SLOT(addCoverTest()));

    previousWidth = 0;
	updatingCovers = false;
	elapsedTimer = new QElapsedTimer();
    elapsedTimer->start();
}

void ImportWidget::newComic(const QString & path, const QString & coverPath)
{
    if(!this->isVisible())
        return;

    currentComicLabel->setText("<font color=\"#565959\">"+path+"</font>");

    if( ((elapsedTimer->elapsed()>=1100) || ((previousWidth < coversView->width()) && (elapsedTimer->elapsed()>=500))) && scrollAnimation->state() != QAbstractAnimation::Running)//todo elapsed time
    {
        updatingCovers = true;
        elapsedTimer->start();

        QPixmap p(coverPath);
        p = p.scaledToHeight(300,Qt::SmoothTransformation);

        QGraphicsPixmapItem * item = new QGraphicsPixmapItem(p);
        item->setPos(previousWidth, 0);
        coversScene->addItem(item);

        previousWidth += 10 + p.width();

        foreach(QGraphicsItem * itemToRemove, coversScene->items())
        {
            QGraphicsPixmapItem * last = dynamic_cast<QGraphicsPixmapItem *>(itemToRemove);

            if((last->pos().x()+last->pixmap().width()) < coversView->horizontalScrollBar()->value()) //TODO check this
            {
                coversScene->removeItem(last);
                delete last;
            }
        }

        QScrollBar * scrollBar = coversView->horizontalScrollBar();

        float speedFactor = 2.5;
        int origin = scrollBar->value();
        int dest = origin + 10 + p.width();

        scrollAnimation->setDuration((dest-origin)*speedFactor);
        scrollAnimation->setStartValue(origin);
        scrollAnimation->setEndValue(dest);
        QEasingCurve easing(QEasingCurve::OutQuad);
        scrollAnimation->setEasingCurve(easing);
        scrollAnimation->start();
    }
}

void ImportWidget::newCover(const QPixmap & image)
{
	Q_UNUSED(image)
}
static int i = 1;
static int previousWidth = 10;
static int j = 0;
void ImportWidget::addCoverTest()
{
		QPixmap p(QString("c:/temp/%1.jpg").arg(i));
		p = p.scaledToHeight(300,Qt::SmoothTransformation);
		QGraphicsPixmapItem * item = new QGraphicsPixmapItem(p);
		item->setPos(previousWidth,0);
		item->setZValue(i/10000.0);
		previousWidth += 10 + p.width();
		coversScene->addItem(item);
		if(previousWidth >= coversView->width())
		{
			QGraphicsItem * last = coversScene->items().last();
			int width = p.width();
			if(j>=1)
			{
				coversScene->removeItem(last);
				delete last;
			}
			else
				j++;

			foreach(QGraphicsItem * itemToMove, coversScene->items())
			{

				QTimeLine *timer = new QTimeLine(/*350*/1000);
				timer->setFrameRange(0, 60);

				QGraphicsItemAnimation *animation = new QGraphicsItemAnimation;
				animation->setItem(itemToMove);
				animation->setTimeLine(timer);

				QPointF point = itemToMove->scenePos();
				float step = (width+10)/60.0;
				for (int i = 0; i < 60; ++i)
					animation->setPosAt(i / 60.0, QPointF(point.x()-((i+1)*step), point.y()));

				timer->start();
			}
			previousWidth -= 10+width;
		}

		i++;
}

void ImportWidget::clear()
{
    previousWidth = 0;
	
	//nos aseguramos de que las animaciones han finalizado antes de borrar
	QList<QGraphicsItem*> all = coversScene->items();
	for (int i = 0; i < all.size(); i++)
	{
		QGraphicsItem *gi = all[i];
		if(gi->parentItem()==NULL)
			delete gi;
	}
	coversScene->clear();

    delete coversScene;
    coversScene = new QGraphicsScene;

    coversView->setScene(coversScene);

    updatingCovers = false;

	currentComicLabel->setText("<font color=\"#565959\">...</font>");

	this->i = 0;
}

void ImportWidget::setImportLook()
{
	iconLabel->setPixmap(QPixmap(":/images/importingIcon.png"));
	text->setText("<font color=\"#495252\">"+tr("Importing comics")+"</font>");
	textDescription->setText("<font color=\"#565959\">"+tr("<p>YACReaderLibrary is now creating a new library.</p><p>Create a library could take several minutes. You can stop the process and update the library later for completing the task.</p>")+"</font>");

    stopButton->setVisible(true);
    coversLabel->setVisible(true);
    coversViewContainer->setVisible(true);
    hideButton->setVisible(true);
}

void ImportWidget::setUpdateLook()
{
	iconLabel->setPixmap(QPixmap(":/images/updatingIcon.png"));
	text->setText("<font color=\"#495252\">"+tr("Updating the library")+"</font>");
	textDescription->setText("<font color=\"#565959\">"+tr("<p>The current library is being updated. For faster updates, please, update your libraries frequently.</p><p>You can stop the process and continue updating this library later.</p>")+"</font>");

    stopButton->setVisible(true);
    coversLabel->setVisible(true);
    coversViewContainer->setVisible(true);
    hideButton->setVisible(true);
}

void ImportWidget::setUpgradeLook()
{
    iconLabel->setPixmap(QPixmap(":/images/updatingIcon.png"));
    text->setText("<font color=\"#495252\">"+tr("Upgrading the library")+"</font>");
    textDescription->setText("<font color=\"#565959\">"+tr("<p>The current library is being upgraded, please wait.</p>")+"</font>");

    stopButton->setVisible(false);
    coversLabel->setVisible(false);
    coversViewContainer->setVisible(false);
    hideButton->setVisible(false);
}

void ImportWidget::clearScene()
{


}

void ImportWidget::showCovers(bool hide)
{
    coversLabel->setHidden(hide);
    coversViewContainer->setHidden(hide);
}

void ImportWidget::resizeEvent(QResizeEvent * event)
{
	hideButton->move(event->size().width()-hideButton->width()- (currentComicLabel->height()/2),event->size().height()-hideButton->height()- (currentComicLabel->height()/2));

	QWidget::resizeEvent(event);
}
