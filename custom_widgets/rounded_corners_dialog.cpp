#include "rounded_corners_dialog.h"

#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QWidget>
#include <QtMath>

namespace {

constexpr int cornerRadius = 14;

QMargins marginsForEffect(const QGraphicsEffect &effect, const QSize &contentSize)
{
    const QRectF contentBounds { QPointF(0, 0), QSizeF(contentSize) };
    const QRectF effectBounds = effect.boundingRectFor(contentBounds);

    return QMargins(qMax(0, qCeil(contentBounds.left() - effectBounds.left())),
                    qMax(0, qCeil(contentBounds.top() - effectBounds.top())),
                    qMax(0, qCeil(effectBounds.right() - contentBounds.right())),
                    qMax(0, qCeil(effectBounds.bottom() - contentBounds.bottom())));
}

}

YACReader::RoundedCornersDialog::RoundedCornersDialog(QWidget *parent)
    : QDialog(parent), m_dialogSurface(new QWidget(this))
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_dialogSurface);

    m_dialogSurface->setObjectName("roundedCornersDialogSurface");
    m_dialogSurface->setAttribute(Qt::WA_StyledBackground);

    auto shadow = new QGraphicsDropShadowEffect(m_dialogSurface);
    shadow->setBlurRadius(36);
    shadow->setColor(QColor(0, 0, 0, 180));
    shadow->setOffset(3, 6);
    m_dialogSurface->setGraphicsEffect(shadow);

    setBackgroundColor(Qt::white);
}

QWidget *YACReader::RoundedCornersDialog::dialogSurface() const
{
    return m_dialogSurface;
}

void YACReader::RoundedCornersDialog::setContentFixedSize(const QSize &size)
{
    m_dialogSurface->setFixedSize(size);

    const auto effect = m_dialogSurface->graphicsEffect();
    const QMargins shadowMargins = effect != nullptr ? marginsForEffect(*effect, size) : QMargins();
    layout()->setContentsMargins(shadowMargins);
    setFixedSize(size + QSize(shadowMargins.left() + shadowMargins.right(), shadowMargins.top() + shadowMargins.bottom()));
}

void YACReader::RoundedCornersDialog::setBackgroundColor(const QColor &color)
{
    m_dialogSurface->setStyleSheet(QString("QWidget#roundedCornersDialogSurface { background-color: rgba(%1, %2, %3, %4); border-radius: %5px; }")
                                           .arg(color.red())
                                           .arg(color.green())
                                           .arg(color.blue())
                                           .arg(color.alpha())
                                           .arg(cornerRadius));
}
