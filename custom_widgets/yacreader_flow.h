#ifndef YACREADER_FLOW_H
#define YACREADER_FLOW_H

#include "pictureflow.h"

class QMouseEvent;

class YACReaderFlow : public PictureFlow
{
    Q_OBJECT
public:
    explicit YACReaderFlow(QWidget *parent, FlowType flowType = CoverFlowLike);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
    void selected(unsigned int centerIndex);
};

#endif // YACREADER_FLOW_H
