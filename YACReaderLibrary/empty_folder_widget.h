#ifndef EMPTY_FOLDER_WIDGET_H
#define EMPTY_FOLDER_WIDGET_H

#include "empty_container_info.h"

class EmptyFolderWidget : public EmptyContainerInfo
{
    Q_OBJECT
public:
    explicit EmptyFolderWidget(QWidget *parent = nullptr);

protected:
    void applyTheme(const Theme &theme) override;
};

#endif // EMPTY_FOLDER_WIDGET_H
