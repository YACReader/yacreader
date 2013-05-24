#include "yacreader_titled_toolbar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QToolButton>
#include <QGraphicsDropShadowEffect>

YACReaderTitledToolBar::YACReaderTitledToolBar(const QString & title, QWidget *parent) :
    QWidget(parent)
{
	QHBoxLayout * mainLayout = new QHBoxLayout;
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);

	QString styleSheet = "QWidget {border:0px;}";
	setStyleSheet(styleSheet);

	QLabel * nameLabel = new QLabel(title,this);
	
	QString nameLabelStyleSheet = "QLabel {color:#454545; padding:0 0 0 0px; margin:0px; font-size:14px; font-weight:bold;}";
	nameLabel->setStyleSheet(nameLabelStyleSheet);

	mainLayout->addWidget(nameLabel,Qt::AlignLeft);
	mainLayout->addStretch();

	setLayout(mainLayout);

	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

	setMinimumHeight(25);
}


void YACReaderTitledToolBar::addAction(QAction * action)
{
	QHBoxLayout * mainLayout = dynamic_cast<QHBoxLayout *>(layout());

	QToolButton * tb = new QToolButton(this);
	tb->setDefaultAction(action);
	tb->setIconSize(QSize(16,16));
	tb->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
	tb->setStyleSheet("QToolButton:hover {background-color:#A5A5A5;}");

	mainLayout->addWidget(tb);
}