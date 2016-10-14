#include "empty_reading_list_widget.h"

EmptyReadingListWidget::EmptyReadingListWidget(QWidget *parent)
    :EmptyContainerInfo(parent)
{
    setUpDefaultLayout(true);
    setPixmap(QPixmap(":/images/empty_reading_list"));
    setText(tr("This reading list doesn't cotain comics yet"));
}
