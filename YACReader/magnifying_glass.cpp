#include "magnifying_glass.h"

#include "viewer.h"

MagnifyingGlass::MagnifyingGlass(int w, int h, float zoomLevel, QWidget *parent)
    : QLabel(parent), zoomLevel(zoomLevel)
{
    setup(QSize(w, h));
}

MagnifyingGlass::MagnifyingGlass(const QSize &size, float zoomLevel, QWidget *parent)
    : QLabel(parent), zoomLevel(zoomLevel)
{
    setup(size);
}

void MagnifyingGlass::setup(const QSize &size)
{
    resize(size);
    setScaledContents(true);
    setMouseTracking(true);
    setCursor(QCursor(QBitmap(1, 1), QBitmap(1, 1)));
}

void MagnifyingGlass::mouseMoveEvent(QMouseEvent *event)
{
    updateImage();
    event->accept();
}

void MagnifyingGlass::updateImage(int x, int y)
{
    auto *const viewer = qobject_cast<const Viewer *>(parentWidget());
    QImage img = viewer->grabMagnifiedRegion(QPoint(x, y), size(), zoomLevel);
    setPixmap(QPixmap::fromImage(img));
    move(static_cast<int>(x - float(width()) / 2), static_cast<int>(y - float(height()) / 2));
}

void MagnifyingGlass::updateImage()
{
    if (isVisible()) {
        QPoint p = QPoint(cursor().pos().x(), cursor().pos().y());
        p = this->parentWidget()->mapFromGlobal(p);
        updateImage(p.x(), p.y());
    }
}
void MagnifyingGlass::wheelEvent(QWheelEvent *event)
{
    switch (event->modifiers()) {
    // size
    case Qt::NoModifier:
        if (event->angleDelta().y() < 0)
            sizeUp();
        else
            sizeDown();
        break;
    // size height
    case Qt::ControlModifier:
        if (event->angleDelta().y() < 0)
            heightUp();
        else
            heightDown();
        break;
    // size width
    case Qt::AltModifier: // alt modifier can actually modify the behavior of the event delta, so let's check both x & y
        if (event->angleDelta().y() < 0 || event->angleDelta().x() < 0)
            widthUp();
        else
            widthDown();
        break;
    // zoom level
    case Qt::ShiftModifier:
        if (event->angleDelta().y() < 0)
            zoomIn();
        else
            zoomOut();
        break;
    default:
        break; // Never propagate a wheel event to the parent widget, even if we ignore it.
    }
    event->setAccepted(true);
}
void MagnifyingGlass::zoomIn()
{
    if (zoomLevel > 0.2f) {
        zoomLevel -= 0.025f;
        emit zoomChanged(zoomLevel);
        updateImage();
    }
}

void MagnifyingGlass::zoomOut()
{
    if (zoomLevel < 0.9f) {
        zoomLevel += 0.025f;
        emit zoomChanged(zoomLevel);
        updateImage();
    }
}

void MagnifyingGlass::sizeUp()
{
    auto w = width();
    auto h = height();
    if (growWidth(w) | growHeight(h)) // bitwise OR prevents short-circuiting
        resizeAndUpdate(w, h);
}

void MagnifyingGlass::sizeDown()
{
    auto w = width();
    auto h = height();
    if (shrinkWidth(w) | shrinkHeight(h)) // bitwise OR prevents short-circuiting
        resizeAndUpdate(w, h);
}

void MagnifyingGlass::heightUp()
{
    auto h = height();
    if (growHeight(h))
        resizeAndUpdate(width(), h);
}

void MagnifyingGlass::heightDown()
{
    auto h = height();
    if (shrinkHeight(h))
        resizeAndUpdate(width(), h);
}

void MagnifyingGlass::widthUp()
{
    auto w = width();
    if (growWidth(w))
        resizeAndUpdate(w, height());
}

void MagnifyingGlass::widthDown()
{
    auto w = width();
    if (shrinkWidth(w))
        resizeAndUpdate(w, height());
}

void MagnifyingGlass::reset()
{
    zoomLevel = 0.5f;
    emit zoomChanged(zoomLevel);
    resizeAndUpdate(350, 175);
}

void MagnifyingGlass::resizeAndUpdate(int w, int h)
{
    resize(w, h);
    emit sizeChanged(size());
    updateImage();
}

static constexpr auto maxRelativeDimension = 0.9;
static constexpr auto widthStep = 30;
static constexpr auto heightStep = 15;

bool MagnifyingGlass::growWidth(int &w) const
{
    const auto maxWidth = parentWidget()->width() * maxRelativeDimension;
    if (w >= maxWidth)
        return false;
    w += widthStep;
    return true;
}

bool MagnifyingGlass::shrinkWidth(int &w) const
{
    constexpr auto minWidth = 175;
    if (w <= minWidth)
        return false;
    w -= widthStep;
    return true;
}

bool MagnifyingGlass::growHeight(int &h) const
{
    const auto maxHeight = parentWidget()->height() * maxRelativeDimension;
    if (h >= maxHeight)
        return false;
    h += heightStep;
    return true;
}

bool MagnifyingGlass::shrinkHeight(int &h) const
{
    constexpr auto minHeight = 80;
    if (h <= minHeight)
        return false;
    h -= heightStep;
    return true;
}
