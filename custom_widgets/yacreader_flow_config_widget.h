#ifndef YACREADER_FLOW_CONFIG_WIDGET_H
#define YACREADER_FLOW_CONFIG_WIDGET_H

#include <QWidget>

class QRadioButton;

class YACReaderFlowConfigWidget : public QWidget
{
	Q_OBJECT
public:
	QRadioButton *radio1; 
	QRadioButton *radio2;
	QRadioButton *radio3;

	YACReaderFlowConfigWidget(QWidget * parent = 0);
};

#endif // YACREADER_FLOW_CONFIG_WIDGET_H