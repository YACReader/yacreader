#ifndef EMPTY_LIST_FOLDER_WIDGET_H
#define EMPTY_LIST_FOLDER_WIDGET_H

#include "empty_container_info.h"

// a folder of reading lists or smart lists with nothing in it
class EmptyListFolderWidget : public EmptyContainerInfo
{
    Q_OBJECT
public:
    explicit EmptyListFolderWidget(QWidget *parent = nullptr);

protected:
    void applyTheme(const Theme &theme) override;
};

#endif
