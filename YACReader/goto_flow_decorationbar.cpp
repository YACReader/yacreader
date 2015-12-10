#include "goto_flow_decorationbar.h"

#include <QHBoxLayout>
#include <qlabel.h>

GoToFlowDecorationBar::GoToFlowDecorationBar(QWidget * parent)
:QWidget(parent)
{
	QHBoxLayout * topBar = new QHBoxLayout();

	QLabel * imgTopLeft = new QLabel();
	QLabel * imgTopRight = new QLabel();
	QLabel * imgTopMiddle = new QLabel();
	QPixmap pL(":/images/imgTopLeft.png");
	QPixmap pM(":/images/imgTopMiddle.png");
	QPixmap pR(":/images/imgTopRight.png");
	imgTopLeft->setPixmap(pL);
	imgTopRight->setPixmap(pR);
	imgTopMiddle->setPixmap(pM);
	imgTopMiddle->setScaledContents(true);

	topBar->addWidget(imgTopLeft);
	topBar->addWidget(imgTopMiddle);
	topBar->addWidget(imgTopRight);
	topBar->setStretchFactor(imgTopLeft,0);
	topBar->setStretchFactor(imgTopMiddle,1);
	topBar->setStretchFactor(imgTopRight,0);

	topBar->setMargin(0);
	topBar->setSpacing(0);

	setLayout(topBar);
}