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

    explicit YACReaderFlowConfigWidget(QWidget *parent = nullptr);
};

#endif // YACREADER_FLOW_CONFIG_WIDGET_H
