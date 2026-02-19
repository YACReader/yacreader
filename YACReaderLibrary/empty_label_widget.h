#ifndef EMPTY_LABEL_WIDGET_H
#define EMPTY_LABEL_WIDGET_H

#include <QtWidgets>
#include "empty_container_info.h"
#include "yacreader_global_gui.h"

class EmptyLabelWidget : public EmptyContainerInfo
{
    Q_OBJECT
public:
    explicit EmptyLabelWidget(QWidget *parent = nullptr);
    void setColor(YACReader::LabelColors color);

protected:
    void applyTheme(const Theme &theme) override;

private:
    YACReader::LabelColors currentColor = YACReader::YRed;
};

#endif // EMPTY_LABEL_WIDGET_H
