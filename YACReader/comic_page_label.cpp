#include "comic_page_label.h"

#include "book_fold_shadow.h"

#include <QPaintEvent>
#include <QPainter>

ComicPageLabel::ComicPageLabel(QWidget *parent)
    : QLabel(parent)
{
}

void ComicPageLabel::setBookFoldShadow(bool enabled, qreal seamRatio, Qt::Orientation seamOrientation)
{
    foldShadowEnabled = enabled;
    foldSeamRatio = seamRatio;
    foldSeamOrientation = seamOrientation;
    update();
}

void ComicPageLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    if (!foldShadowEnabled || pixmap().isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setClipRegion(event->region());
    drawBookFoldShadow(painter, contentsRect(), foldSeamRatio, foldSeamOrientation);
}
