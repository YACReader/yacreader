#ifndef EMPTY_LABEL_WIDGET_H
#define EMPTY_LABEL_WIDGET_H

#include <QtWidgets>
#include "yacreader_global.h"

class EmptyLabelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EmptyLabelWidget(QWidget *parent = 0);
    void setColor(YACReader::LabelColors color);
    void paintEvent(QPaintEvent *event);

signals:

public slots:

protected:
    QLabel * iconLabel;
    QLabel * titleLabel;
    QString backgroundColor;

};

#endif // EMPTY_LABEL_WIDGET_H
