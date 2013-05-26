#include "goto_flow_widget.h"

#include <QSettings>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QCoreApplication>

#include "goto_flow_toolbar.h"
#include "goto_flow_decorationbar.h"

GoToFlowWidget::GoToFlowWidget(QWidget * parent)
	:QWidget(parent)
{
	mainLayout = new QVBoxLayout;

	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);

	topBar = new GoToFlowDecorationBar(this);
	toolBar = new GoToFlowToolBar(this);

	mainLayout->addWidget(topBar);
	mainLayout->addWidget(toolBar);

	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);

	setLayout(mainLayout);
}

void GoToFlowWidget::setPageNumber(int page)
{
	toolBar->setPage(page);
}

void GoToFlowWidget::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Return: case Qt::Key_Enter:
		toolBar->goTo();
		toolBar->centerSlide();
		break;
	case Qt::Key_Space:
		toolBar->centerSlide();
		break;
	case Qt::Key_S:
		QCoreApplication::sendEvent(this->parent(),event);
		break;
	case Qt::Key_Left: case Qt::Key_Right:
		//if(event->modifiers() == Qt::ControlModifier)
		//flow->keyPressEvent(event);
			//QCoreApplication::sendEvent(flow,event);
		break;
	}

	event->accept();
}
