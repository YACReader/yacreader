#include "yacreader_titled_toolbar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QToolButton>
#include <QGraphicsDropShadowEffect>
#include <QPainter>



DropShadowLabel::DropShadowLabel(QWidget* parent) :
    QLabel(parent)
{ }
 
void DropShadowLabel::drawTextEffect(QPainter *painter,
                                 QPoint offset)
{
    Q_ASSERT(painter != 0);
 
    // Draw shadow.
    painter->setPen(QPen(dropShadowColor));
    painter->drawText(rect().translated(offset),
                      alignment(), text());
}
 
void DropShadowLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setFont(font());
	//TODO find where is the '3' comming from?
	drawTextEffect(&painter, QPoint(contentsMargins().left()+3, 1));
	QLabel::paintEvent(event);
}

void DropShadowLabel::setDropShadowColor(const QColor & color)
{
	dropShadowColor = color;
}



YACReaderTitledToolBar::YACReaderTitledToolBar(const QString & title, QWidget *parent) :
    QWidget(parent)
{
	QHBoxLayout * mainLayout = new QHBoxLayout;
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);

	QString styleSheet = "QWidget {border:0px;}";
	setStyleSheet(styleSheet);

	DropShadowLabel * nameLabel = new DropShadowLabel(this);
	nameLabel->setText(title);
#ifdef Q_OS_MAC
    QString nameLabelStyleSheet = "QLabel {color:#707E8C; padding:0 0 0 7px; margin:0px; font-size:11px; font-weight:bold;}";
	nameLabel->setDropShadowColor(QColor("#F9FAFB"));
#else
    QString nameLabelStyleSheet = "QLabel {color:#BDBFBF; padding:0 0 0 7px; margin:0px; font-size:11px; font-weight:bold;}";
	nameLabel->setDropShadowColor(QColor("#000000"));
#endif
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
	//tb->setStyleSheet("QToolButton:hover {background-color:#C5C5C5;}");

	mainLayout->addWidget(tb);
}
