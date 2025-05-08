#include "cover_utils.h"

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
