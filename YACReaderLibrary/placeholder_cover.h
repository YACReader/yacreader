#ifndef PLACEHOLDER_COVER_H
#define PLACEHOLDER_COVER_H

#include <QString>

#include <optional>

class QImage;

// writes `image` to a new cover file and gives its key; each image gets a new key, so no view shows a cached
// older image. The caller stores the key, then removes the old file after the commit, or this one on a failure
std::optional<QString> savePlaceholderCover(const QString &libraryDataPath, const QImage &image);

#endif
