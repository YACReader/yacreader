#include <QUrl>
#include <Phonon/MediaObject>
#include <Phonon/MediaSource>
#include <QPushButton>
#include <QPalette>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWebView>
#include <QFile>
#include <QPoint>
#include <QWidget>

#include "translator.h"

YACReaderTranslator::YACReaderTranslator(QWidget * parent)
:QWidget(parent)
{
	this->setCursor(QCursor(Qt::ArrowCursor));
	this->setAutoFillBackground(true);
	this->setBackgroundRole(QPalette::Window);
	QPalette p(this->palette());
	p.setColor(QPalette::Window, QColor(96,96,96));
	this->setPalette(p);

	QVBoxLayout *layout = new QVBoxLayout(this);
	QWebView * view = new QWebView();
	QFile f(":/files/translator.html");
	f.open(QIODevice::ReadOnly);
	QTextStream txtS(&f);
	QString contentHTML = txtS.readAll();
	view->setHtml(contentHTML);
	view->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
	connect(view->page(),SIGNAL(linkClicked(QUrl)),this,SLOT(play(QUrl)));

	QHBoxLayout * buttonBar = new QHBoxLayout();
	QPushButton * close = new QPushButton(QIcon(QPixmap(":/images/close.png")),"");
	close->setFlat(true);
	buttonBar->addStretch();
	close->resize(18,18);
	buttonBar->addWidget(close);
	buttonBar->setMargin(0);
	connect(close,SIGNAL(clicked()),this->parent(),SLOT(animateHideTranslator()));

	layout->setMargin(0);
	layout->setSpacing(0);

	layout->addLayout(buttonBar);
	layout->addWidget(view);

	resize(view->size().width()/1.60,view->size().height());

	music = createPlayer(MusicCategory);

	show();
}

void YACReaderTranslator::play(const QUrl & url)
{
	MediaSource src(url);
	src.setAutoDelete(true);
	 music->setCurrentSource(src);
	 music->play();
}

YACReaderTranslator::~YACReaderTranslator()
{
	delete music;
}

void YACReaderTranslator::mousePressEvent(QMouseEvent *event)
{
	drag = true;
	click = event->pos();
}

void YACReaderTranslator::mouseReleaseEvent(QMouseEvent *event)
{
	drag = false;
}

void YACReaderTranslator::mouseMoveEvent(QMouseEvent * event)
{
	if(drag)
		this->move(QPoint(mapToParent(event->pos())-click));
	
}

