#ifndef VOLUME_SEARCH_QUERY_H
#define VOLUME_SEARCH_QUERY_H

#include <QString>

struct VolumeSearchQuery {
    QString volume;
    int page;
    bool exactMatch;
};

#endif // VOLUME_SEARCH_QUERY_H
