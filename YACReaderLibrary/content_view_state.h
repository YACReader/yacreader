#ifndef CONTENT_VIEW_STATE_H
#define CONTENT_VIEW_STATE_H

#include "comic_item.h"

#include <QtGlobal>

struct ContentItemRef {
    enum Kind {
        None,
        Comic,
        Folder,
        Header
    };

    Kind kind = None;
    qulonglong folderId = 0;
    ComicRowId comic;
};

struct ContentViewState {
    ContentItemRef topItem;
    int fallbackComicRow = -1;
    qreal offset = 0;
    qreal itemExtent = 0;
    ContentItemRef currentItem;
};

#endif // CONTENT_VIEW_STATE_H
