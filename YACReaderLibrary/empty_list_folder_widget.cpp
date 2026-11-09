#include "empty_list_folder_widget.h"

EmptyListFolderWidget::EmptyListFolderWidget(QWidget *parent)
    : EmptyContainerInfo(parent)
{
    setUpDefaultLayout(true);
    setPixmap(theme.emptyContainer.emptyFolderIcon);
    setText(tr("This folder is empty"));
}

void EmptyListFolderWidget::applyTheme(const Theme &theme)
{
    EmptyContainerInfo::applyTheme(theme);
    setPixmap(theme.emptyContainer.emptyFolderIcon);
}
