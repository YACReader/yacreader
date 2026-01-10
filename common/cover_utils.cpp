#include "cover_utils.h"
#include "image_decoders.h"

QImage YACReader::loadImageFromData(const QByteArray &data)
{
    // Try AVIF first
    if (isAvif(data)) {
        return decodeAvif(data);
    }
    // Try JXL
    if (isJxl(data)) {
        return decodeJxl(data);
    }
    // Fall back to Qt's built-in loaders (JPEG, PNG, etc.)
    QImage image;
    image.loadFromData(data);
    return image;
}

bool YACReader::saveCover(const QString &path, const QImage &cover)
{
    QImage scaled;
    if (cover.width() > cover.height()) {
        scaled = cover.scaledToWidth(640, Qt::SmoothTransformation);
    } else {
        auto aspectRatio = static_cast<double>(cover.width()) / static_cast<double>(cover.height());
        auto maxAllowedAspectRatio = 0.5;
        if (aspectRatio < maxAllowedAspectRatio) { // cover is too tall, e.g. webtoon
            scaled = cover.scaledToHeight(960, Qt::SmoothTransformation);
        } else {
            scaled = cover.scaledToWidth(480, Qt::SmoothTransformation);
        }
    }
    return scaled.save(path, 0, 75);
}
