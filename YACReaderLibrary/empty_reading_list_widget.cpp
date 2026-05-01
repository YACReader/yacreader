#include "empty_reading_list_widget.h"

EmptyReadingListWidget::EmptyReadingListWidget(QWidget *parent)
    : EmptyContainerInfo(parent)
{
    setUpDefaultLayout(true);
    setPixmap(theme.emptyContainer.emptyReadingListIcon);
    setText(tr("This reading list does not contain any comics yet"));
}

void EmptyReadingListWidget::applyTheme(const Theme &theme)
{
    EmptyContainerInfo::applyTheme(theme);
    setPixmap(theme.emptyContainer.emptyReadingListIcon);
}
