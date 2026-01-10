#ifndef IMAGE_DECODERS_H
#define IMAGE_DECODERS_H

#include <QImage>
#include <QByteArray>

bool isAvif(const QByteArray &data);
bool isJxl(const QByteArray &data);
QImage decodeAvif(const QByteArray &data);
QImage decodeJxl(const QByteArray &data);

#endif // IMAGE_DECODERS_H
