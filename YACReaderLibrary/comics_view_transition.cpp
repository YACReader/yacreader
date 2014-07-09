#include "comics_view_transition.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
#include <QSettings>
#include <QTimer>
#include <QSizePolicy>
#include <QPainter>

#include "yacreader_global.h"

ComicsViewTransition::ComicsViewTransition(QWidget *parent) :
    QWidget(parent),movie(0)
{
   QVBoxLayout * layout = new QVBoxLayout;

   settings = new QSettings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat);
   settings->beginGroup("libraryConfig");

   movieLabel = new QLabel("Placeholder");
   movieLabel->setAlignment(Qt::AlignCenter);
   QLabel * textLabel = new QLabel("Switching comics view");
   textLabel->setAlignment(Qt::AlignCenter);
   textLabel->setStyleSheet("QLabel {color:#CCCCCC; font-size:24px;font-family:Arial;font-weight:bold;}");
   //movieLabel->setFixedSize(450,350);

   layout->addSpacing(100);
   layout->addWidget(movieLabel);
   layout->addSpacing(20);
   layout->addWidget(textLabel);
   layout->addStretch();
   layout->setMargin(0);
   layout->setSpacing(0);

   setContentsMargins(0,0,0,0);

   setStyleSheet("QWidget {background:#2A2A2A}");

   //QSizePolicy sp();
   setSizePolicy(QSizePolicy ::Expanding , QSizePolicy ::Expanding );
   //movieLabel->setSizePolicy(QSizePolicy ::Expanding , QSizePolicy ::Expanding );
   setLayout(layout);
}

QSize ComicsViewTransition::sizeHint()
{
    return QSize(450,350);
}

void ComicsViewTransition::startMovie()
{
    if(movie)
        delete movie;

    if(settings->value(COMICS_VIEW_STATUS) == YACReader::Flow)
        movie = new QMovie(":/images/flow_to_grid.gif");
    else
        movie = new QMovie(":/images/grid_to_flow.gif");

    connect(movie,SIGNAL(finished()),this,SIGNAL(transitionFinished()));
    //connect(movie,SIGNAL(finished()),movie,SLOT(deleteLater());
    movie->setSpeed(200);
    movie->jumpToFrame(0);
    movieLabel->setMovie(movie);

    QTimer::singleShot(100,movie,SLOT(start()));
}

void ComicsViewTransition::paintEvent(QPaintEvent *)
{
    QPainter painter (this);
    painter.fillRect(0,0,width(),height(),QColor("#2A2A2A"));
}
