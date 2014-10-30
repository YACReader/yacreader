#ifndef YACREADER_READING_LISTS_VIEW_H
#define YACREADER_READING_LISTS_VIEW_H

#include "yacreader_treeview.h"

#include <QtWidgets>

class YACReaderReadingListsView : public YACReaderTreeView
{
    Q_OBJECT
public:
    explicit YACReaderReadingListsView(QWidget * parent = 0);
};

#endif // YACREADER_READING_LISTS_VIEW_H
