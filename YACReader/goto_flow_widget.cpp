#include "goto_flow_widget.h"

#include <QSettings>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QCoreApplication>

#include "goto_flow_toolbar.h"

GoToFlowWidget::GoToFlowWidget(QWidget * parent)
	:QWidget(parent)
{
	mainLayout = new QVBoxLayout;

	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);

	toolBar = new GoToFlowToolBar(this);

	mainLayout->addWidget(toolBar);

	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);

	setLayout(mainLayout);

	//toolBar->installEventFilter(this);
}

GoToFlowWidget::~GoToFlowWidget() { 
	delete toolBar;
	delete mainLayout;
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
	}

	event->accept();
}

/*bool GoToFlowWidget::eventFilter(QObject * target, QEvent * event)
{
	if(event->type() == QEvent::KeyPress)
	{
		QKeyEvent * e = static_cast<QKeyEvent *>(event);
		if(e->key()==Qt::Key_S || e->key() == Qt::Key_Space)
		{
			this->keyPressEvent(e);
			return true;
		}
	}
	return QWidget::eventFilter(target,event);
}*/
