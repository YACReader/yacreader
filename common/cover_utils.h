#ifndef COVER_UTILS_H
#define COVER_UTILS_H

#include <QImage>
#include <QList>
#include <QString>
#include <QUrl>

namespace YACReader {
bool saveCover(const QString &path, const QImage &image);
QString droppedImagePath(const QList<QUrl> &urls);
}
#endif // COVER_UTILS_H
