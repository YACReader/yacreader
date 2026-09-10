#include "goto_flow_widget.h"

#include "configuration.h"
#include "goto_flow_toolbar.h"

#include <QApplication>
#include <QBuffer>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QIcon>
#include <QImageReader>
#include <QKeyEvent>
#include <QListWidget>
#include <QPainter>
#include <QPalette>
#include <QSettings>
#include <QShowEvent>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QWheelEvent>

#include <utility>

namespace {
constexpr int softwareFlowPadding = 12;
constexpr int softwareFlowItemSpacing = 12;

class SoftwareFlowItemDelegate final : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
        if (icon.isNull())
            return;

        const QPixmap pixmap = icon.pixmap(option.decorationSize);
        const QSize pixmapSize = pixmap.deviceIndependentSize().toSize();
        QRect pixmapRect(QPoint(), pixmapSize);
        pixmapRect.moveCenter(option.rect.center());
        painter->drawPixmap(pixmapRect.topLeft(), pixmap);
    }
};
}

GoToFlowWidget::GoToFlowWidget(QWidget *parent, FlowType flowType)
    : QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    toolBar = new GoToFlowToolBar(this);

    setLayout(mainLayout);

    imageSize = Configuration::getConfiguration().getGotoSlideSize();
    softwareRendering = Configuration::getConfiguration().getUseSoftwareGoToFlow();

    if (softwareRendering) {
        softwareFlow = new QListWidget(this);
        softwareFlow->setViewMode(QListView::IconMode);
        softwareFlow->setFlow(QListView::LeftToRight);
        softwareFlow->setWrapping(false);
        softwareFlow->setMovement(QListView::Static);
        softwareFlow->setResizeMode(QListView::Adjust);
        softwareFlow->setItemAlignment(Qt::AlignCenter);
        softwareFlow->setSelectionMode(QAbstractItemView::NoSelection);
        softwareFlow->setFocusPolicy(Qt::NoFocus);
        softwareFlow->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        softwareFlow->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        softwareFlow->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        softwareFlow->setFrameShape(QFrame::NoFrame);
        softwareFlow->setSpacing(softwareFlowItemSpacing);
        softwareFlow->setItemDelegate(new SoftwareFlowItemDelegate(softwareFlow));
        softwareFlow->viewport()->installEventFilter(this);
        softwareFlow->viewport()->setAutoFillBackground(true);
        setAutoFillBackground(true);
        mainLayout->setContentsMargins(0, softwareFlowPadding, 0, toolBar->height() + softwareFlowPadding);
        updateSoftwareFlowMetrics();

        connect(softwareFlow, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
            const int page = softwareFlow->row(item) - 1;
            if (page >= 0 && page < softwareImages.size()) {
                centerSlide(page);
                emit goToPage(static_cast<unsigned int>(page));
            }
        });
    } else {
        rhiFlow = new YACReaderPageFlow3D(this);
        rhiFlow->setShowMarks(false);
        rhiFlow->setSlideSize(imageSize);
        connect(rhiFlow, &YACReaderPageFlow3D::centerIndexChanged, this, &GoToFlowWidget::setPageNumber);
        connect(rhiFlow, &YACReaderPageFlow3D::selected, this, &GoToFlowWidget::goToPage);
        setFlowType(flowType);
    }

    connect(toolBar, &GoToFlowToolBar::goToPage, this, &GoToFlowWidget::goToPage);
    connect(toolBar, &GoToFlowToolBar::setCenter, this, [this](unsigned int page) {
        centerSlide(static_cast<int>(page));
    });

    mainLayout->addWidget(softwareRendering ? static_cast<QWidget *>(softwareFlow) : static_cast<QWidget *>(rhiFlow));
    toolBar->raise();

    const int flowHeight = softwareRendering
            ? imageSize.height() + toolBar->height() + 2 * softwareFlowPadding
            : toolBar->height() + static_cast<int>(imageSize.height() * 1.7);
    resize(static_cast<int>(5 * imageSize.width()), flowHeight);

    this->setCursor(QCursor(Qt::ArrowCursor));

    initTheme(this);
}

void GoToFlowWidget::applyTheme(const Theme &theme)
{
    auto goToFlowTheme = theme.goToFlowWidget;

    if (rhiFlow != nullptr) {
        rhiFlow->setBackgroundColor(goToFlowTheme.flowBackgroundColor);
        rhiFlow->setTextColor(goToFlowTheme.flowTextColor);
    } else {
        QPalette palette = softwareFlow->palette();
        palette.setColor(QPalette::Base, goToFlowTheme.flowBackgroundColor);
        palette.setColor(QPalette::Window, goToFlowTheme.flowBackgroundColor);
        palette.setColor(QPalette::Text, goToFlowTheme.flowTextColor);
        softwareFlow->setPalette(palette);
        softwareFlow->viewport()->setPalette(palette);

        QPalette widgetPalette = this->palette();
        widgetPalette.setColor(QPalette::Window, goToFlowTheme.flowBackgroundColor);
        setPalette(widgetPalette);
    }
}

GoToFlowWidget::~GoToFlowWidget()
{
}

void GoToFlowWidget::reset()
{
    if (rhiFlow != nullptr) {
        rhiFlow->reset();
    } else {
        clearSoftwareThumbnailQueue();
        softwareDecodedThumbnails.clear();
        softwareImages.clear();
        softwareFlow->clear();
        softwareCurrentPage = -1;
    }
}

void GoToFlowWidget::centerSlide(int slide)
{
    if (rhiFlow != nullptr) {
        if (rhiFlow->centerIndex() != slide)
            rhiFlow->setCenterIndex(slide);
        return;
    }

    if (slide < 0 || slide >= softwareImages.size())
        return;

    const int row = slide + 1;
    softwareCurrentPage = slide;
    setPageNumber(slide);
    softwareFlow->scrollToItem(softwareFlow->item(row), QAbstractItemView::PositionAtCenter);
    updateSoftwareThumbnailWindow();
}

void GoToFlowWidget::setPageNumber(int page)
{
    toolBar->setPage(page);
}

void GoToFlowWidget::setFlowType(FlowType flowType)
{
    if (rhiFlow == nullptr)
        return;

    if (flowType == CoverFlowLike)
        rhiFlow->setPreset(presetYACReaderFlowClassicConfig);
    else if (flowType == Strip)
        rhiFlow->setPreset(presetYACReaderFlowStripeConfig);
    else if (flowType == StripOverlapped)
        rhiFlow->setPreset(presetYACReaderFlowOverlappedStripeConfig);
    else
        rhiFlow->setPreset(defaultYACReaderFlowConfig);
}

void GoToFlowWidget::setNumSlides(unsigned int slides)
{
    if (rhiFlow != nullptr) {
        rhiFlow->populate(slides);
    } else {
        clearSoftwareThumbnailQueue();
        softwareDecodedThumbnails.clear();
        softwareFlow->clear();
        softwareImages.clear();
        softwareImages.resize(static_cast<int>(slides));

        auto *leadingSpace = new QListWidgetItem(softwareFlow);
        leadingSpace->setFlags(Qt::NoItemFlags);
        for (unsigned int page = 0; page < slides; ++page) {
            auto *item = new QListWidgetItem(softwareFlow);
            item->setToolTip(QString::number(page + 1));
        }
        auto *trailingSpace = new QListWidgetItem(softwareFlow);
        trailingSpace->setFlags(Qt::NoItemFlags);

        updateSoftwareFlowMetrics();
        if (slides > 0)
            centerSlide(0);
    }
    toolBar->setTop(slides);
}

void GoToFlowWidget::setImageReady(int index, const QByteArray &imageData)
{
    if (rhiFlow != nullptr) {
        rhiFlow->rawImages[index] = imageData;
        rhiFlow->imagesReady[index] = true;
    } else if (index >= 0 && index < softwareImages.size()) {
        softwareImages[index] = imageData;
        if (isVisible() && index >= softwareThumbnailFirst && index <= softwareThumbnailLast)
            enqueueSoftwareThumbnail(index);
    }
}

void GoToFlowWidget::updateSize()
{
    if (Configuration::getConfiguration().getQuickNaviMode() && parentWidget() != nullptr)
        resize(parentWidget()->width(), height());
}

void GoToFlowWidget::updateConfig(QSettings *settings)
{
    toolBar->updateOptions();

    Performance performance = medium;
    switch (settings->value(PERFORMANCE).toInt()) {
    case 0:
        performance = low;
        break;
    case 1:
        performance = medium;
        break;
    case 2:
        performance = high;
        break;
    case 3:
        performance = ultraHigh;
        break;
    }

    imageSize = Configuration::getConfiguration().getGotoSlideSize();
    const int flowHeight = softwareRendering
            ? imageSize.height() + toolBar->height() + 2 * softwareFlowPadding
            : toolBar->height() + static_cast<int>(imageSize.height() * 1.7);
    resize(5 * imageSize.width(), flowHeight);
    updateSize();

    if (rhiFlow == nullptr) {
        clearSoftwareThumbnailQueue();
        for (int index : std::as_const(softwareDecodedThumbnails))
            softwareFlow->item(index + 1)->setIcon(QIcon());
        softwareDecodedThumbnails.clear();
        updateSoftwareFlowMetrics();
        updateSoftwareThumbnailWindow();
        return;
    }

    rhiFlow->setPerformance(performance);

    switch (settings->value(FLOW_TYPE_GL).toInt()) {
    case FlowType::CoverFlowLike:
        rhiFlow->setPreset(presetYACReaderFlowClassicConfig);
        break;
    case FlowType::Strip:
        rhiFlow->setPreset(presetYACReaderFlowStripeConfig);
        break;
    case FlowType::StripOverlapped:
        rhiFlow->setPreset(presetYACReaderFlowOverlappedStripeConfig);
        break;
    case FlowType::Modern:
        rhiFlow->setPreset(defaultYACReaderFlowConfig);
        break;
    case FlowType::Roulette:
        rhiFlow->setPreset(pressetYACReaderFlowDownConfig);
        break;
    case FlowType::Custom:
        rhiFlow->setCF_RX(settings->value(X_ROTATION).toInt());
        rhiFlow->setCF_Y(settings->value(Y_POSITION).toInt());
        rhiFlow->setX_Distance(settings->value(COVER_DISTANCE).toInt());
        rhiFlow->setCenter_Distance(settings->value(CENTRAL_DISTANCE).toInt());
        rhiFlow->setCF_Z(settings->value(ZOOM_LEVEL).toInt());
        rhiFlow->setY_Distance(settings->value(Y_COVER_OFFSET).toInt());
        rhiFlow->setZ_Distance(settings->value(Z_COVER_OFFSET).toInt());
        rhiFlow->setRotation(settings->value(COVER_ROTATION).toInt());
        rhiFlow->setFadeOutDist(settings->value(FADE_OUT_DIST).toInt());
        rhiFlow->setLightStrenght(settings->value(LIGHT_STRENGTH).toInt());
        rhiFlow->setMaxAngle(settings->value(MAX_ANGLE).toInt());
        break;
    }
    if (Configuration::getConfiguration().getQuickNaviMode())
        rhiFlow->setFadeOutDist(20);
}

void GoToFlowWidget::setFlowRightToLeft(bool b)
{
    rightToLeft = b;
    if (rhiFlow != nullptr)
        rhiFlow->setFlowRightToLeft(b);
    else
        softwareFlow->setLayoutDirection(b ? Qt::RightToLeft : Qt::LeftToRight);
}

void GoToFlowWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
        if (rhiFlow != nullptr) {
            QApplication::sendEvent(rhiFlow, event);
        } else {
            int step = event->key() == Qt::Key_Right ? 1 : -1;
            if (rightToLeft)
                step = -step;
            centerSlide(softwareCurrentPage + step);
        }
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        toolBar->goTo();
        toolBar->centerSlide();
        break;
    case Qt::Key_Space:
        toolBar->centerSlide();
        break;
    case Qt::Key_S:
        QCoreApplication::sendEvent(this->parent(), event);
        break;
    }

    event->accept();
}

bool GoToFlowWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (softwareFlow != nullptr && watched == softwareFlow->viewport() && event->type() == QEvent::Wheel) {
        auto *wheelEvent = static_cast<QWheelEvent *>(event);
        const QPoint angleDelta = wheelEvent->angleDelta();
        const QPoint pixelDelta = wheelEvent->pixelDelta();
        const bool usesPixels = angleDelta.isNull();
        const QPoint delta = usesPixels ? pixelDelta : angleDelta;
        const int dominantDelta = qAbs(delta.y()) >= qAbs(delta.x()) ? delta.y() : delta.x();

        wheelDeltaAccumulator -= dominantDelta;
        const int threshold = usesPixels ? 40 : 120;
        if (qAbs(wheelDeltaAccumulator) >= threshold) {
            const int step = wheelDeltaAccumulator > 0 ? 1 : -1;
            wheelDeltaAccumulator = 0; // Never advance multiple pages for a single wheel event.
            centerSlide(softwareCurrentPage + step);
        }

        wheelEvent->accept();
        return true;
    }

    return QWidget::eventFilter(watched, event);
}

void GoToFlowWidget::updateSoftwareFlowMetrics()
{
    if (softwareFlow == nullptr)
        return;

    softwareFlow->setIconSize(imageSize);
    softwareFlow->setGridSize(QSize());
    const QSize itemSize(imageSize.width() + 2 * softwareFlowPadding, imageSize.height());
    for (int row = 1; row + 1 < softwareFlow->count(); ++row)
        softwareFlow->item(row)->setSizeHint(itemSize);
    updateSoftwareFlowEdgeSpacing();
}

void GoToFlowWidget::updateSoftwareFlowEdgeSpacing()
{
    if (softwareFlow == nullptr || softwareFlow->count() < 2)
        return;

    // A full viewport of inert content on either side guarantees that the first and last pages
    // have enough scroll range to occupy the same center position as every other page.
    const int edgeWidth = softwareFlow->viewport()->width();
    const QSize edgeSize(edgeWidth, imageSize.height());
    softwareFlow->item(0)->setSizeHint(edgeSize);
    softwareFlow->item(softwareFlow->count() - 1)->setSizeHint(edgeSize);
}

void GoToFlowWidget::updateSoftwareThumbnail(int index)
{
    if (softwareFlow == nullptr || index < 0 || index >= softwareImages.size() || softwareImages[index].isEmpty())
        return;

    QBuffer buffer(&softwareImages[index]);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    reader.setDecideFormatFromContent(true);
    const QSize sourceSize = reader.size();
    if (sourceSize.isValid())
        reader.setScaledSize(sourceSize.scaled(imageSize, Qt::KeepAspectRatio));

    const QImage image = reader.read();
    if (!image.isNull()) {
        softwareFlow->item(index + 1)->setIcon(QPixmap::fromImage(image));
        softwareDecodedThumbnails.insert(index);
    }
}

void GoToFlowWidget::updateSoftwareThumbnailWindow()
{
    if (softwareFlow == nullptr || !isVisible() || softwareCurrentPage < 0 || softwareImages.isEmpty())
        return;

    const int itemWidth = imageSize.width() + 2 * softwareFlowPadding + softwareFlowItemSpacing;
    const int visiblePages = qMax(1, (softwareFlow->viewport()->width() + itemWidth - 1) / itemWidth);
    constexpr int prefetchPages = 2;
    softwareThumbnailFirst = qMax(0, softwareCurrentPage - visiblePages / 2 - prefetchPages);
    softwareThumbnailLast = qMin(softwareImages.size() - 1, softwareCurrentPage + (visiblePages - 1) / 2 + prefetchPages);

    for (auto it = softwareDecodedThumbnails.begin(); it != softwareDecodedThumbnails.end();) {
        if (*it < softwareThumbnailFirst || *it > softwareThumbnailLast) {
            softwareFlow->item(*it + 1)->setIcon(QIcon());
            it = softwareDecodedThumbnails.erase(it);
        } else {
            ++it;
        }
    }

    softwareThumbnailQueue.clear();
    softwareQueuedThumbnails.clear();
    const int farthestPage = qMax(softwareCurrentPage - softwareThumbnailFirst, softwareThumbnailLast - softwareCurrentPage);
    for (int distance = 0; distance <= farthestPage; ++distance) {
        enqueueSoftwareThumbnail(softwareCurrentPage - distance);
        if (distance != 0)
            enqueueSoftwareThumbnail(softwareCurrentPage + distance);
    }
}

void GoToFlowWidget::enqueueSoftwareThumbnail(int index)
{
    if (index < softwareThumbnailFirst || index > softwareThumbnailLast || index < 0 || index >= softwareImages.size() || softwareImages[index].isEmpty() || softwareDecodedThumbnails.contains(index) || softwareQueuedThumbnails.contains(index))
        return;

    softwareThumbnailQueue.enqueue(index);
    softwareQueuedThumbnails.insert(index);
    if (!softwareThumbnailDecodeScheduled) {
        softwareThumbnailDecodeScheduled = true;
        QTimer::singleShot(0, this, &GoToFlowWidget::processNextSoftwareThumbnail);
    }
}

void GoToFlowWidget::processNextSoftwareThumbnail()
{
    softwareThumbnailDecodeScheduled = false;
    if (softwareFlow == nullptr || !isVisible()) {
        clearSoftwareThumbnailQueue();
        return;
    }

    while (!softwareThumbnailQueue.isEmpty()) {
        const int index = softwareThumbnailQueue.dequeue();
        softwareQueuedThumbnails.remove(index);
        if (index >= softwareThumbnailFirst && index <= softwareThumbnailLast && !softwareDecodedThumbnails.contains(index) && !softwareImages[index].isEmpty()) {
            updateSoftwareThumbnail(index);
            break;
        }
    }

    if (!softwareThumbnailQueue.isEmpty()) {
        softwareThumbnailDecodeScheduled = true;
        QTimer::singleShot(0, this, &GoToFlowWidget::processNextSoftwareThumbnail);
    }
}

void GoToFlowWidget::clearSoftwareThumbnailQueue()
{
    softwareThumbnailQueue.clear();
    softwareQueuedThumbnails.clear();
    softwareThumbnailFirst = -1;
    softwareThumbnailLast = -1;
}

void GoToFlowWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateSoftwareThumbnailWindow();
}

void GoToFlowWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    clearSoftwareThumbnailQueue();
}

void GoToFlowWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    toolBar->move(0, event->size().height() - toolBar->height());
    toolBar->setFixedWidth(width());
    toolBar->raise();

    if (softwareFlow != nullptr) {
        QTimer::singleShot(0, this, [this] {
            updateSoftwareFlowEdgeSpacing();
            softwareFlow->doItemsLayout();
            if (softwareCurrentPage >= 0) {
                centerSlide(softwareCurrentPage);
            }
        });
    }
}
