#ifndef YACREADER_TOOL_BAR_STRETCH_H
#define YACREADER_TOOL_BAR_STRETCH_H

#include <QWidget>
#include <QHBoxLayout>

class QToolBarStretch : public QWidget
{
public:
	QToolBarStretch(QWidget * parent=0):QWidget(parent)
	{
		QHBoxLayout * l= new QHBoxLayout();
		l->addStretch();
		setLayout(l);
	}
};

#endif // YACREADER_TOOL_BAR_STRETCH_H
