#include "goto_flow_widget.h"

GoToFlowWidget::GoToFlowWidget(QWidget * parent)
	:QWidget(parent)
{
		QPalette palette;
	palette.setColor(backgroundRole(), Qt::black);
	setPalette(palette);
}
