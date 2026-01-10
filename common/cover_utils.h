#ifndef COVER_UTILS_H
#define COVER_UTILS_H

#include <QImage>
#include <QByteArray>

namespace YACReader {
bool saveCover(const QString &path, const QImage &image);
QImage loadImageFromData(const QByteArray &data);
}
#endif // COVER_UTILS_H
