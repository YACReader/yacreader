#include "empty_folder_widget.h"

EmptyFolderWidget::EmptyFolderWidget(QWidget *parent)
    : EmptyContainerInfo(parent)
{
    setUpDefaultLayout(true);
    setPixmap(theme.emptyContainer.emptyFolderIcon);
    setText(tr("This folder doesn't contain comics yet"));
}

void EmptyFolderWidget::applyTheme(const Theme &theme)
{
    EmptyContainerInfo::applyTheme(theme);
    setPixmap(theme.emptyContainer.emptyFolderIcon);
}
