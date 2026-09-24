#include "viewer.h"

#include "bookmarks_dialog.h"
#include "comic_db.h"
#include "configuration.h"
#include "continuous_page_provider.h"
#include "continuous_page_widget.h"
#include "continuous_view_model.h"
#include "goto_dialog.h"
#include "goto_flow_widget.h"
#include "magnifying_glass.h"
#include "notifications_label_widget.h"
#include "page_label_widget.h"
#include "render.h"
#include "resize_image.h"
#include "translator.h"

#include <QFile>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMessageBox>
#include <QPainter>
#include <QPinchGesture>
#include <QPropertyAnimation>
#include <QScrollBar>

#include <cmath>

#ifdef Q_OS_MACOS
#include <ApplicationServices/ApplicationServices.h>
#endif

namespace {
void drawBookFoldShadow(QPixmap &page, qreal seamRatio, Qt::Orientation seamOrientation, qreal devicePixelRatio)
{
    const bool verticalSeam = seamOrientation == Qt::Vertical;
    const int axisLength = verticalSeam ? page.width() : page.height();
    const int seamPosition = qRound(axisLength * seamRatio);
    const int maximumRadius = qMin(seamPosition, axisLength - seamPosition);
    const int radius = qMin(qRound(36 * devicePixelRatio), maximumRadius);
    if (radius <= 0) {
        return;
    }

    QLinearGradient gradient;
    if (verticalSeam) {
        gradient = QLinearGradient(seamPosition - radius, 0, seamPosition + radius, 0);
    } else {
        gradient = QLinearGradient(0, seamPosition - radius, 0, seamPosition + radius);
    }
    gradient.setColorAt(0.0, QColor(0, 0, 0, 0));
    gradient.setColorAt(0.32, QColor(0, 0, 0, 8));
    gradient.setColorAt(0.42, QColor(0, 0, 0, 35));
    gradient.setColorAt(0.48, QColor(0, 0, 0, 90));
    gradient.setColorAt(0.5, QColor(0, 0, 0, 125));
    gradient.setColorAt(0.54, QColor(0, 0, 0, 72));
    gradient.setColorAt(0.68, QColor(0, 0, 0, 18));
    gradient.setColorAt(1.0, QColor(0, 0, 0, 0));

    QPainter painter(&page);
    const QRect shadowRect = verticalSeam
            ? QRect(seamPosition - radius, 0, radius * 2, page.height())
            : QRect(0, seamPosition - radius, page.width(), radius * 2);
    painter.fillRect(shadowRect, gradient);

    const int creaseWidth = qMax(1, qRound(devicePixelRatio));
    const QRect creaseRect = verticalSeam
            ? QRect(seamPosition - creaseWidth / 2, 0, creaseWidth, page.height())
            : QRect(0, seamPosition - creaseWidth / 2, page.width(), creaseWidth);
    painter.fillRect(creaseRect, QColor(0, 0, 0, 145));
}

// QCursor::setPos moves the pointer by synthesizing a mouse event and injecting
// it into the HID event stream (QCocoaCursor::setPos -> CGEventPost). macOS
// gates that behind the accessibility "control this computer" permission, so on
// a machine that hasn't granted it the call is silently denied and the pointer
// never moves, while the user gets an unexplained permission request.
//
// CGWarpMouseCursorPosition repositions the pointer without injecting an event
// and needs no permission. It doesn't deliver a mouse move to the application,
// which suits the only caller here: the point is to reposition the pointer
// *without* triggering the move handling that would hide the widget again.
//
// Reported upstream as https://qt-project.atlassian.net/browse/QTBUG-148709.
// If Qt switches QCocoaCursor::setPos to the warp, this can go back to being a
// plain QCursor::setPos call once the fixed version is the minimum supported.
void moveCursorTo(const QPoint &globalPos)
{
#ifdef Q_OS_MACOS
    CGWarpMouseCursorPosition(CGPointMake(globalPos.x(), globalPos.y()));
    // Warping leaves a short interval where physical mouse movement is filtered
    // out; re-associating ends it so the next movement registers immediately.
    CGAssociateMouseAndMouseCursorPosition(true);
#else
    QCursor::setPos(globalPos);
#endif
}
}

Viewer::Viewer(QWidget *parent)
    : QScrollArea(parent),
      fullscreen(false),
      information(false),
      doublePage(false),
      doubleMangaPage(false),
      continuousScroll(false),
      zoom(100),
      currentPage(nullptr),
      wheelStop(false),
      direction(1),
      drag(false),
      shouldOpenNext(false),
      shouldOpenPrevious(false),
      magnifyingGlassShown(false),
      restoreMagnifyingGlass(false),
      pinchStartZoom(100),
      zoomAnchorNormX(0.5),
      zoomAnchorNormY(0.5),
      zoomHud(nullptr),
      zoomHudHideTimer(nullptr),
      zoomPreviewFinishTimer(nullptr),
      mouseHandler(std::make_unique<YACReader::MouseHandler>(this))
{
    grabGesture(Qt::PinchGesture);

    zoomHud = new QLabel(this);
    zoomHud->setAlignment(Qt::AlignCenter);
    zoomHud->setAttribute(Qt::WA_TransparentForMouseEvents);
    zoomHud->setTextFormat(Qt::RichText);
    zoomHud->setStyleSheet(
            "background-color: rgba(0, 0, 0, 153); border-radius: 3px;");
    zoomHud->setFixedSize(100, 60);
    zoomHud->hide();

    zoomHudHideTimer = new QTimer(this);
    zoomHudHideTimer->setSingleShot(true);
    connect(zoomHudHideTimer, &QTimer::timeout, zoomHud, &QWidget::hide);

    zoomPreviewFinishTimer = new QTimer(this);
    zoomPreviewFinishTimer->setSingleShot(true);
    zoomPreviewFinishTimer->setInterval(250);
    connect(zoomPreviewFinishTimer, &QTimer::timeout, this, &Viewer::renderFinalZoomImage);

    translator = new YACReaderTranslator(this);
    translator->hide();
    translatorAnimation = new QPropertyAnimation(translator, "pos");
    translatorAnimation->setDuration(150);
    translatorXPos = -10000;
    translator->move(-translator->width(), 10);
    // current comic page (used in non-continuous mode when a comic is open)
    content = new QLabel(this);
    content->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    if (!(devicePixelRatioF() > 1))
        content->setScaledContents(true);
    content->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    content->setMouseTracking(true);

    // dedicated widget for status messages ("Press 'O' to open comic.", "Loading...", etc.)
    messageLabel = new QLabel(this);
    messageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    messageLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    messageLabel->setText(tr("Press 'O' to open comic."));
    messageLabel->setFont(QFont("courier new", 12));
    messageLabel->setMouseTracking(true);

    setWidget(messageLabel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setAlignment(Qt::AlignCenter);

    continuousWidget = new ContinuousPageWidget();
    continuousPageProvider = new ContinuousPageProvider(this);
    continuousViewModel = new ContinuousViewModel(QWIDGETSIZE_MAX, this);
    continuousWidget->setViewModel(continuousViewModel);
    continuousWidget->setProvider(continuousPageProvider);
    continuousWidget->installEventFilter(this);
    //---------------------------------------
    mglass = new MagnifyingGlass(
            Configuration::getConfiguration().getMagnifyingGlassSize(),
            Configuration::getConfiguration().getMagnifyingGlassZoom(),
            Configuration::getConfiguration().getMagnifyingGlassCircular(),
            Configuration::getConfiguration().getMagnifyingGlassRing(),
            this);

    magnifierEdgeEase = Configuration::getConfiguration().getMagnifyingGlassEdgeEase();

    connect(mglass, &MagnifyingGlass::sizeChanged, this, [](QSize size) {
        Configuration::getConfiguration().setMagnifyingGlassSize(size);
    });
    connect(mglass, &MagnifyingGlass::zoomChanged, this, [](float zoom) {
        Configuration::getConfiguration().setMagnifyingGlassZoom(zoom);
    });

    mglass->hide();
    setMouseTracking(true);

    showCursor();

    goToDialog = new GoToDialog(this);

    // CONFIG GOTO_FLOW--------------------------------------------------------
    goToFlow = new GoToFlowWidget(this, Configuration::getConfiguration().getFlowType());

    goToFlow->setFocusPolicy(Qt::StrongFocus);
    goToFlow->hide();
    showGoToFlowAnimation = new QPropertyAnimation(goToFlow, "pos");
    showGoToFlowAnimation->setDuration(150);

    bd = new BookmarksDialog(this->parentWidget());

    render = new Render();
    continuousWidget->setRender(render);
    continuousPageProvider->setRender(render);

    hideCursorTimer = new QTimer();
    hideCursorTimer->setSingleShot(true);

    if (Configuration::getConfiguration().getDoublePage())
        doublePageSwitch();

    if (Configuration::getConfiguration().getDoubleMangaPage())
        setMangaModeImpl(true, false);

    if (Configuration::getConfiguration().getContinuousScroll())
        setContinuousScrollImpl(true, false);

    createConnections();

    hideCursorTimer->start(2500);

    setMouseTracking(true);

    // animations
    verticalScroller = new QPropertyAnimation(verticalScrollBar(), "sliderPosition");
    connect(verticalScroller, &QVariantAnimation::valueChanged, this, &Viewer::backgroundChanges);
    horizontalScroller = new QPropertyAnimation(horizontalScrollBar(), "sliderPosition");
    connect(horizontalScroller, &QVariantAnimation::valueChanged, this, &Viewer::backgroundChanges);
    groupScroller = new QParallelAnimationGroup();
    groupScroller->addAnimation(verticalScroller);
    groupScroller->addAnimation(horizontalScroller);

    notificationsLabel = new NotificationsLabelWidget(this);
    notificationsLabel->hide();

    informationLabel = new PageLabelWidget(this);

    setAcceptDrops(true);

    initTheme(this);
}

Viewer::~Viewer()
{
    delete render;
    delete goToFlow;
    delete translator;
    delete translatorAnimation;
    // messageLabel, content or continuousWidget may not be owned by the scroll area
    // (after takeWidget), so delete whichever ones are not currently set
    if (widget() != messageLabel) {
        delete messageLabel;
    }
    if (widget() != content) {
        delete content;
    }
    if (widget() != continuousWidget) {
        delete continuousWidget;
    }
    delete continuousViewModel;
    delete hideCursorTimer;
    delete informationLabel;
    delete verticalScroller;
    delete horizontalScroller;
    delete groupScroller;
    delete bd;
    delete notificationsLabel;
    delete mglass;
    if (currentPage != nullptr)
        delete currentPage;
}

QString Viewer::renderingSystemInfo() const
{
    QString text = QStringLiteral("\nRENDERING INFORMATION\n");
    text.append(QStringLiteral("Reader renderer: QWidget (Raster)\n"));
    text.append(goToFlow->renderingSystemInfo());
    return text;
}

void Viewer::createConnections()
{
    // magnifyingGlass (update mg after a background change
    connect(this, &Viewer::backgroundChanges, mglass, QOverload<>::of(&MagnifyingGlass::updateImage));

    connect(this, &Viewer::magnifyingGlassSizeUp, mglass, &MagnifyingGlass::sizeUp);
    connect(this, &Viewer::magnifyingGlassSizeDown, mglass, &MagnifyingGlass::sizeDown);
    connect(this, &Viewer::magnifyingGlassZoomIn, mglass, &MagnifyingGlass::zoomIn);
    connect(this, &Viewer::magnifyingGlassZoomOut, mglass, &MagnifyingGlass::zoomOut);
    connect(this, &Viewer::resetMagnifyingGlass, mglass, &MagnifyingGlass::reset);

    // goToDialog
    connect(goToDialog, &GoToDialog::goToPage, this, &Viewer::goTo);

    // goToFlow goTo
    connect(goToFlow, &GoToFlowWidget::goToPage, this, &Viewer::goTo);

    // current time
    auto t = new QTimer(this);
    connect(t, &QTimer::timeout, this, &Viewer::updateInformation);
    t->start(1000);

    // hide cursor
    connect(hideCursorTimer, &QTimer::timeout, this, &Viewer::hideCursor);

    // bookmarks
    connect(bd, &BookmarksDialog::goToPage, this, &Viewer::goTo);

    // render
    connect(render, QOverload<>::of(&Render::errorOpening), this, &Viewer::resetContent);
    connect(render, QOverload<>::of(&Render::errorOpening), this, QOverload<>::of(&Viewer::showMessageErrorOpening));
    connect(render, QOverload<QString>::of(&Render::errorOpening), this, QOverload<QString>::of(&Viewer::showMessageErrorOpening));
    connect(render, &Render::crcError, this, &Viewer::processCRCError);
    connect(render, QOverload<unsigned int>::of(&Render::numPages), goToFlow, &GoToFlowWidget::setNumSlides);
    connect(render, QOverload<unsigned int>::of(&Render::numPages), goToDialog, &GoToDialog::setNumPages);
    connect(render, qOverload<unsigned int>(&Render::numPages), this, &Viewer::comicLoaded);
    connect(render, QOverload<int, const QByteArray &>::of(&Render::imageLoaded), goToFlow, &GoToFlowWidget::setImageReady);
    connect(render, &Render::currentPageReady, this, &Viewer::updatePage);
    // Continuous mode is driven by the off-thread page provider, not by Render's
    // (blocking) page buffer. The provider emits pageReady when a full-resolution
    // page has been decoded on a background thread.
    connect(continuousPageProvider, &ContinuousPageProvider::pageReady, continuousWidget, &ContinuousPageWidget::onPageAvailable);
    connect(continuousPageProvider, &ContinuousPageProvider::pageReady, this, &Viewer::onContinuousPageRendered);
    connect(continuousViewModel, &ContinuousViewModel::stateChanged, this, &Viewer::onContinuousViewModelChanged);
    connect(render, qOverload<unsigned int>(&Render::numPages), this, &Viewer::onNumPagesReady);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &Viewer::onContinuousScroll);
    connect(render, &Render::processingPage, this, &Viewer::setLoadingMessage);
    connect(render, &Render::currentPageIsBookmark, this, &Viewer::pageIsBookmark);
    connect(render, &Render::pageChanged, this, &Viewer::updateInformation);
    connect(render, &Render::pageChanged, this, &Viewer::onRenderPageChanged);

    connect(render, &Render::isLast, this, &Viewer::showIsLastMessage);
    connect(render, &Render::isCover, this, &Viewer::showIsCoverMessage);

    connect(render, &Render::bookmarksUpdated, this, &Viewer::setBookmarks);
}

// Deprecated
void Viewer::prepareForOpening()
{
    if (render->hasLoadedComic())
        save();
    // bd->setBookmarks(*bm);

    goToFlow->reset();

    // render->update();

    // Drop the previous comic's decoded/scaled pages so we never flash stale
    // content while the new comic loads.
    continuousPageProvider->reset();
    continuousWidget->invalidateScaledImageCache();

    verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum());

    if (Configuration::getConfiguration().getShowInformation() && !information) {
        QTimer::singleShot(0, this, &Viewer::informationSwitch);
    }

    informationLabel->setText("...");
}

void Viewer::open(QString pathFile, int atPage)
{
    prepareForOpening();
    // No per-comic filter levels here: match Render, whose filters keep their
    // settings-driven defaults (level -1) on this load path.
    continuousPageProvider->setFilters(-1, -1, -1);
    render->load(pathFile, atPage);
}

void Viewer::open(QString pathFile, const ComicDB &comic)
{
    prepareForOpening();
    // Mirror exactly the filter levels Render::load(comicDB) derives, so the
    // decoded pages match single-page mode.
    continuousPageProvider->setFilters(comic.info.brightness == -1 ? 0 : comic.info.brightness,
                                       comic.info.contrast == -1 ? 100 : comic.info.contrast,
                                       comic.info.gamma == -1 ? 100 : comic.info.gamma);
    render->load(pathFile, comic);
}

void Viewer::showMessageErrorOpening()
{
    QMessageBox::critical(this, tr("Not found"), tr("Comic not found"));
    // resetContent(); --> not needed
}

void Viewer::showMessageErrorOpening(QString message)
{
    QMessageBox::critical(this, tr("Error opening comic"), message);
    resetContent();
}

void Viewer::processCRCError(QString message)
{
    QMessageBox::critical(this, tr("CRC Error"), message);
}

void Viewer::next()
{
    if (!render->hasLoadedComic()) {
        return;
    }

    direction = 1;
    syncRenderToContinuousReadingProgress();
    render->nextPage();
    updateInformation();
    shouldOpenPrevious = false;
}

void Viewer::left()
{
    if (!render->hasLoadedComic()) {
        return;
    }

    if (doubleMangaPage) {
        next();
    } else {
        prev();
    }
}

void Viewer::right()
{
    if (!render->hasLoadedComic()) {
        return;
    }

    if (doubleMangaPage) {
        prev();
    } else {
        next();
    }
}

void Viewer::prev()
{
    if (!render->hasLoadedComic()) {
        return;
    }

    direction = -1;
    syncRenderToContinuousReadingProgress();
    render->previousPage();
    updateInformation();
    shouldOpenNext = false;
}
void Viewer::showGoToDialog()
{
    goToDialog->open();
}
void Viewer::goToFirstPage()
{
    goTo(0);
}
void Viewer::goToLastPage()
{
    goTo(this->render->numPages() - 1);
}
void Viewer::goTo(unsigned int page)
{
    direction = 1; // in "go to" direction is always fordward

    if (continuousScroll) {
        lastCenterPage = page;
        continuousViewModel->setAnchorPage(static_cast<int>(page));
        render->goTo(page);
        scrollToCurrentContinuousPage();
        return;
    }

    render->goTo(page);
}

void Viewer::onImageOptionsChanged()
{
    if (continuousScroll) {
        continuousWidget->invalidateScaledImageCache();
    } else {
        updatePage();
    }
}

void Viewer::updatePage()
{
    if (continuousScroll) {
        return;
    }

    setActiveWidget(content);

    QPixmap *previousPage = currentPage;
    currentPageHasDoublePageSeam = false;
    if (doublePage) {
        if (!doubleMangaPage)
            currentPage = render->getCurrentDoublePage(&doublePageSeamRatio, &doublePageSeamOrientation);
        else {
            currentPage = render->getCurrentDoubleMangaPage(&doublePageSeamRatio, &doublePageSeamOrientation);
        }
        currentPageHasDoublePageSeam = currentPage != nullptr;
        if (currentPage == nullptr) {
            currentPage = render->getCurrentPage();
        }
    } else {
        currentPage = render->getCurrentPage();
    }
    content->setPixmap(*currentPage);
    updateContentSize();
    updateVerticalScrollBar();

    if (goToFlow->isHidden())
        setFocus(Qt::ShortcutFocusReason);
    else
        goToFlow->setFocus(Qt::OtherFocusReason);
    delete previousPage;

    if (currentPage->isNull())
        setPageUnavailableMessage();
    else
        emit pageAvailable(true);

    emit backgroundChanges();

    if (restoreMagnifyingGlass) {
        restoreMagnifyingGlass = false;
        showMagnifyingGlass();
    }
}

void Viewer::updateContentSize()
{
    cancelZoomPreview();

    // there is an image to resize
    if (currentPage != nullptr && !currentPage->isNull()) {
        QSize pagefit = currentPage->size();
        bool stretchImages = Configuration::getConfiguration().getEnlargeImages();
        YACReader::FitMode fitmode = Configuration::getConfiguration().getFitMode();
        switch (fitmode) {
        case YACReader::FitMode::FullRes:
            break;
        case YACReader::FitMode::ToWidth:
            if (!stretchImages && width() > pagefit.width()) {
                break;
            }
            pagefit.scale(width(), 0, Qt::KeepAspectRatioByExpanding);
            break;
        case YACReader::FitMode::ToHeight:
            if (!stretchImages && height() > pagefit.height()) {
                break;
            }
            pagefit.scale(0, height(), Qt::KeepAspectRatioByExpanding);
            break;
            // if everything fails showing the full page is a good idea
        case YACReader::FitMode::FullPage:
        default:
            pagefit.scale(size(), Qt::KeepAspectRatio);
            break;
        }

        if (zoom != 100) {
            pagefit.scale(floor(pagefit.width() * zoom / 100.0f), 0, Qt::KeepAspectRatioByExpanding);
        }
        // apply size to the container
        content->resize(pagefit);

        // scale the pixmap to physical pixels for crisp rendering on all displays
        auto dpr = devicePixelRatioF();
        QPixmap page = scalePixmap(*currentPage,
                                   qRound(content->width() * dpr),
                                   qRound(content->height() * dpr),
                                   Configuration::getConfiguration().getScalingMethod());
        if (currentPageHasDoublePageSeam && Configuration::getConfiguration().getDoublePageShadow()) {
            drawBookFoldShadow(page, doublePageSeamRatio, doublePageSeamOrientation, dpr);
        }
        page.setDevicePixelRatio(dpr);
        content->setPixmap(page);

        emit backgroundChanges();
    }
    content->update(); // TODO, it shouldn't be neccesary
}

void Viewer::increaseZoomFactor()
{
    captureZoomAnchor();
    applyZoomAtAnchor(zoom + 10);
    zoomHud->hide();
    notificationsLabel->setText(QString::number(getZoomFactor()) + "%");
    notificationsLabel->flash();
}
void Viewer::decreaseZoomFactor()
{
    captureZoomAnchor();
    applyZoomAtAnchor(zoom - 10);
    zoomHud->hide();
    notificationsLabel->setText(QString::number(getZoomFactor()) + "%");
    notificationsLabel->flash();
}

int Viewer::getZoomFactor()
{
    // this function is a placeholder for future refactoring work
    return zoom;
}

void Viewer::setZoomFactor(int z)
{
    // this function is mostly used to reset the zoom after a fitmode switch
    if (z > 500)
        zoom = 500;
    else if (z < 30)
        zoom = 30;
    else
        zoom = z;

    emit zoomUpdated(zoom);
}

void Viewer::updateVerticalScrollBar()
{
    if (direction > 0) {
        verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum());
    } else {
        verticalScrollBar()->setSliderPosition(verticalScrollBar()->maximum());
    }
}

void Viewer::scrollDown()
{
    if (verticalScrollBar()->sliderPosition() == verticalScrollBar()->maximum()) {
        if (continuousScroll) {
            shouldOpenNext = true;
            emit openNextComic();
        } else {
            next();
        }
    } else {
        int currentPos = verticalScrollBar()->sliderPosition();
        verticalScroller->setDuration(animationDuration());
        verticalScroller->setStartValue(currentPos);
        verticalScroller->setEndValue(nextPos);

        verticalScroller->start();

        emit backgroundChanges();
    }
}

void Viewer::scrollUp()
{
    if (verticalScrollBar()->sliderPosition() == verticalScrollBar()->minimum()) {
        if (continuousScroll) {
            shouldOpenPrevious = true;
            emit openPreviousComic();
        } else {
            prev();
        }
    } else {
        int currentPos = verticalScrollBar()->sliderPosition();
        verticalScroller->setDuration(animationDuration());
        verticalScroller->setStartValue(currentPos);
        verticalScroller->setEndValue(nextPos);

        verticalScroller->start();

        emit backgroundChanges();
    }
}

void Viewer::scrollForward()
{
    nextPos = verticalScrollBar()->sliderPosition() + verticalScrollStep();
    scrollDown();
}

void Viewer::scrollBackward()
{
    nextPos = verticalScrollBar()->sliderPosition() - verticalScrollStep();
    scrollUp();
}

void Viewer::scrollForwardHorizontalFirst()
{
    if (!doubleMangaPage) {
        scrollZigzag(RIGHT, DOWN, true); // right->right->lower left->right->...->next page
    } else {
        scrollZigzag(LEFT, DOWN, true); // left->left->lower right->left->...->next page
    }
}

void Viewer::scrollBackwardHorizontalFirst()
{
    if (!doubleMangaPage) {
        scrollZigzag(LEFT, UP, false); // left->left->upper right->left->...->prev page
    } else {
        scrollZigzag(RIGHT, UP, false); // right->right->upper left->right->...->prev page
    }
}

void Viewer::scrollForwardVerticalFirst()
{
    if (!doubleMangaPage) {
        scrollZigzag(DOWN, RIGHT, true); // down->down->upper right->down->...->next page
    } else {
        scrollZigzag(DOWN, LEFT, true); // down->down->upper left->down->...->next page
    }
}

void Viewer::scrollBackwardVerticalFirst()
{
    if (!doubleMangaPage) {
        scrollZigzag(UP, LEFT, false); // up->up->lower left->up->...->prev page
    } else {
        scrollZigzag(UP, RIGHT, false); // up->up->lower right->up->...->prev page
    }
}

static constexpr auto relativeScrollStep = 0.80;

int Viewer::verticalScrollStep() const
{
    return static_cast<int>(height() * relativeScrollStep);
}

int Viewer::horizontalScrollStep() const
{
    return static_cast<int>(width() * relativeScrollStep);
}

bool Viewer::isEdge(scrollDirection d)
{
    if (d == UP)
        return verticalScrollBar()->sliderPosition() == verticalScrollBar()->minimum();
    else if (d == DOWN)
        return verticalScrollBar()->sliderPosition() == verticalScrollBar()->maximum();
    else if (d == LEFT)
        return horizontalScrollBar()->sliderPosition() == horizontalScrollBar()->minimum();
    else // d == RIGHT
        return horizontalScrollBar()->sliderPosition() == horizontalScrollBar()->maximum();
}

void Viewer::scrollZigzag(scrollDirection d1, scrollDirection d2, bool forward)
{
    if (!isEdge(d1)) {
        if (d1 == UP)
            scrollTo(horizontalScrollBar()->sliderPosition(),
                     verticalScrollBar()->sliderPosition() - verticalScrollStep());
        else if (d1 == DOWN)
            scrollTo(horizontalScrollBar()->sliderPosition(),
                     verticalScrollBar()->sliderPosition() + verticalScrollStep());
        else if (d1 == LEFT)
            scrollTo(horizontalScrollBar()->sliderPosition() - horizontalScrollStep(),
                     verticalScrollBar()->sliderPosition());
        else // d1 == RIGHT
            scrollTo(horizontalScrollBar()->sliderPosition() + horizontalScrollStep(),
                     verticalScrollBar()->sliderPosition());
    } else if (!isEdge(d2)) {
        int x = 0;
        int y = 0;

        if (d1 == UP)
            y = verticalScrollBar()->maximum();
        else if (d1 == DOWN)
            y = verticalScrollBar()->minimum();
        else if (d1 == LEFT)
            x = horizontalScrollBar()->maximum();
        else // d1 == RIGHT
            x = horizontalScrollBar()->minimum();

        if (d2 == UP)
            y = std::max(verticalScrollBar()->sliderPosition() - verticalScrollStep(), verticalScrollBar()->minimum());
        else if (d2 == DOWN)
            y = std::min(verticalScrollBar()->sliderPosition() + verticalScrollStep(), verticalScrollBar()->maximum());
        else if (d2 == LEFT)
            x = std::max(horizontalScrollBar()->sliderPosition() - horizontalScrollStep(), horizontalScrollBar()->minimum());
        else // d2 == RIGHT
            x = std::min(horizontalScrollBar()->sliderPosition() + horizontalScrollStep(), horizontalScrollBar()->maximum());

        scrollTo(x, y);
    } else {
        // next or prev page's corner
        int savedPageNumber = getCurrentPageNumber();

        if (forward)
            next();
        else
            prev();

        if (savedPageNumber != getCurrentPageNumber()) {
            if (d1 == LEFT || d2 == LEFT)
                horizontalScrollBar()->setSliderPosition(horizontalScrollBar()->maximum());
            else
                horizontalScrollBar()->setSliderPosition(horizontalScrollBar()->minimum());
            emit backgroundChanges();
        }
    }
}

void Viewer::scrollTo(int x, int y)
{
    if (groupScroller->state() == QAbstractAnimation::Running)
        return;
    horizontalScroller->setDuration(animationDuration());
    horizontalScroller->setStartValue(horizontalScrollBar()->sliderPosition());
    horizontalScroller->setEndValue(x);
    verticalScroller->setDuration(animationDuration());
    verticalScroller->setStartValue(verticalScrollBar()->sliderPosition());
    verticalScroller->setEndValue(y);
    groupScroller->start();
    emit backgroundChanges();
}

int Viewer::animationDuration() const
{
    if (Configuration::getConfiguration().getDisableScrollAnimation()) {
        return 0;
    } else {
        return 250;
    }
}

void Viewer::moveView(Qt::Key directionKey)
{
    QKeyEvent event(QEvent::KeyPress, directionKey, Qt::NoModifier);
    QAbstractScrollArea::keyPressEvent(&event);
    emit backgroundChanges();
}

void Viewer::animateScroll(QPropertyAnimation &scroller, const QScrollBar &scrollBar, int delta)
{
    int deltaNotFinished = 0;
    if (scroller.state() == QAbstractAnimation::Running) {
        deltaNotFinished = scroller.startValue().toInt() - scroller.endValue().toInt();
        scroller.stop();
    }

    const int currentPos = scrollBar.sliderPosition();
    scroller.setDuration(animationDuration());
    scroller.setStartValue(currentPos);
    scroller.setEndValue(currentPos - delta - deltaNotFinished);

    scroller.start();
}

void Viewer::wheelEvent(QWheelEvent *event)
{
    if (!render->hasLoadedComic()) {
        return;
    }

    // Check the modifier before choosing the regular mouse/trackpad scroll path so
    // high-resolution devices with pixelDelta (notably on macOS) zoom as well. Qt maps
    // ControlModifier to Command on macOS unless the application opts out of that mapping.
    if (event->modifiers() == Qt::ControlModifier && event->angleDelta().y() != 0) {
        wheelEventZoom(event);
        return;
    }

    wheelZoomAccumulator = 0;

    if (!event->pixelDelta().isNull()) {
        wheelEventTrackpad(event);
    } else {
        wheelEventMouse(event);
    }
}

void Viewer::wheelEventZoom(QWheelEvent *event)
{
    static constexpr int wheelStep = 120;
    static constexpr int zoomStep = 10;
    static constexpr qint64 accumulatorResetMs = 400;
    static constexpr int hudTimeoutMs = 500;

    horizontalScroller->stop();
    verticalScroller->stop();
    wheelStop = false;

    if (!wheelZoomTimer.isValid() || wheelZoomTimer.elapsed() > accumulatorResetMs) {
        wheelZoomAccumulator = 0;
    }
    wheelZoomTimer.restart();

    wheelZoomAccumulator += event->angleDelta().y();
    const int steps = wheelZoomAccumulator / wheelStep;
    wheelZoomAccumulator -= steps * wheelStep;

    if (steps != 0) {
        captureZoomAnchor();
        if (applyZoomAtAnchor(zoom + steps * zoomStep)) {
            zoomHudHideTimer->start(hudTimeoutMs);
        }
    }

    event->accept();
}

void Viewer::wheelEventMouse(QWheelEvent *event)
{
    auto delta = event->angleDelta();

    if (delta.x() != 0) {
        animateScroll(*horizontalScroller, *horizontalScrollBar(), delta.x());
        return;
    }

    if (continuousScroll) {
        animateScroll(*verticalScroller, *verticalScrollBar(), delta.y());
        return;
    }

    auto turnPageOnScroll = !Configuration::getConfiguration().getDoNotTurnPageOnScroll();
    auto getUseSingleScrollStepToTurnPage = Configuration::getConfiguration().getUseSingleScrollStepToTurnPage();

    if ((delta.y() < 0) && (verticalScrollBar()->sliderPosition() == verticalScrollBar()->maximum()) && turnPageOnScroll) {
        if (wheelStop || getUseSingleScrollStepToTurnPage || verticalScrollBar()->maximum() == verticalScrollBar()->minimum()) {
            if (getMovement(event) == Forward) {
                next();
                verticalScroller->stop();
                event->accept();
                wheelStop = false;
            }
            return;
        } else
            wheelStop = true;
    } else {
        if ((delta.y() > 0) && (verticalScrollBar()->sliderPosition() == verticalScrollBar()->minimum()) && turnPageOnScroll) {
            if (wheelStop || getUseSingleScrollStepToTurnPage || verticalScrollBar()->maximum() == verticalScrollBar()->minimum()) {
                if (getMovement(event) == Backward) {
                    prev();
                    verticalScroller->stop();
                    event->accept();
                    wheelStop = false;
                }
                return;
            } else
                wheelStop = true;
        }
    }

    animateScroll(*verticalScroller, *verticalScrollBar(), delta.y());
}

void Viewer::wheelEventTrackpad(QWheelEvent *event)
{
    auto delta = event->pixelDelta();

    // Treat pixel-based scrolling as direct manipulation, not as an animation target.
    horizontalScroller->stop();
    verticalScroller->stop();

    // Apply delta to horizontal scrollbar
    if (delta.x() != 0) {
        int newHorizontalValue = horizontalScrollBar()->value() - delta.x();
        horizontalScrollBar()->setValue(newHorizontalValue);
    }

    // Apply delta to vertical scrollbar
    if (delta.y() != 0) {
        int newVerticalValue = verticalScrollBar()->value() - delta.y();
        verticalScrollBar()->setValue(newVerticalValue);
    }

    if (continuousScroll) {
        return;
    }

    auto turnPageOnScroll = !Configuration::getConfiguration().getDoNotTurnPageOnScroll();
    auto getUseSingleScrollStepToTurnPage = Configuration::getConfiguration().getUseSingleScrollStepToTurnPage();

    if ((delta.y() < 0) && (verticalScrollBar()->sliderPosition() == verticalScrollBar()->maximum()) && turnPageOnScroll) {
        if (wheelStop || getUseSingleScrollStepToTurnPage || verticalScrollBar()->maximum() == verticalScrollBar()->minimum()) {
            if (getMovement(event) == Forward) {
                next();
                event->accept();
                wheelStop = false;
            }
            return;
        } else {
            wheelStop = true;
        }
    } else {
        if ((delta.y() > 0) && (verticalScrollBar()->sliderPosition() == verticalScrollBar()->minimum()) && turnPageOnScroll) {
            if (wheelStop || getUseSingleScrollStepToTurnPage || verticalScrollBar()->maximum() == verticalScrollBar()->minimum()) {
                if (getMovement(event) == Backward) {
                    prev();
                    event->accept();
                    wheelStop = false;
                }
                return;
            } else {
                wheelStop = true;
            }
        }
    }
}

void Viewer::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);

    if (continuousScroll) {
        continuousViewModel->setViewportSize(viewport()->width(), viewport()->height());
    }

    updateContentSize();
    goToFlow->updateSize();
    goToFlow->move((width() - goToFlow->width()) / 2, height() - goToFlow->height());
    informationLabel->updatePosition();
}

QPixmap Viewer::pixmap() const
{
    if (currentPage != nullptr && !currentPage->isNull())
        return *currentPage;

    return content->pixmap();
}

QByteArray Viewer::rawPage(int page) const
{
    return render->getRawPage(page);
}

QList<int> Viewer::currentVisiblePages()
{
    QList<int> pages;

    if (continuousScroll && continuousViewModel->numPages() > 0) {
        int firstPage = continuousViewModel->pageAtY(continuousViewModel->scrollY());
        int lastPage = continuousViewModel->pageAtY(continuousViewModel->scrollY() + viewport()->height() - 1);
        firstPage = qBound(0, firstPage, continuousViewModel->numPages() - 1);
        lastPage = qBound(0, lastPage, continuousViewModel->numPages() - 1);
        for (int page = firstPage; page <= lastPage; ++page) {
            pages << page;
        }
        return pages;
    }

    const int currentPage = render->getIndex();
    pages << currentPage;

    if (doublePage && render->currentPageIsDoublePage() && currentPage + 1 < static_cast<int>(render->numPages()))
        pages << currentPage + 1;

    return pages;
}

namespace {
// The magnifier edge easing pushes the loupe's sampled center outward toward the viewport
// edges so edge content is reachable with less cursor travel. Crucially the push is bounded
// by the loupe's own half-size, which (a) keeps the cursor's true point inside the loupe
// view and (b) scales the effect with loupe size: a minimum-size loupe is nearly linear, a
// large one eases strongly.

// Fraction of the half-axis at which the easing reaches full displacement. Smaller = the
// effect ramps in sooner and saturates before the cursor reaches the edge, so edge content
// is reachable well before the pointer is jammed against the border; beyond this the loupe
// is already at full reach (still capped at the loupe half-size, so the cursor point stays
// in view). 1.0 would only reach full push exactly at the edge.
constexpr double edgeReach = 0.4;

// Overall strength of the push, as a fraction of the loupe half-extent (its natural cap). 1.0
// pushes the sampled center by up to a full half-loupe at the edge; lower values keep the
// content swim gentler so it tracks the cursor more closely. The cursor's point stays in view
// for any value in (0, 1].
constexpr double edgeStrength = 0.3;

// Smoothstep ramp: 0 at the viewport center (1:1 there, and zero slope so it stays linear
// near the middle), rising to 1 by edgeReach (and held there to the edge). Multiplied by the
// loupe half-extent to give the outward displacement.
double edgeRamp(double u) // u = |normalized cursor offset from center|, in [0, 1]
{
    u = qBound(0.0, u / edgeReach, 1.0);
    return u * u * (3.0 - 2.0 * u);
}

// The edge easing is canceled on an axis only once the page is letterboxed by at least this
// fraction of its own size on that axis. Below it — a thin margin, an exact fit, or a page
// that overflows the viewport — the curve still applies, so the effect stays visible for
// pages that nearly fill the view instead of vanishing the instant the page is a hair smaller
// than the viewport. Above it the background beside the page is wide enough that easing would
// mostly reveal that background, so the axis is left at 1:1.
constexpr double minLetterboxFraction = 0.10;
}

QPoint Viewer::easeViewerPos(const QPoint &viewerPos, const QSize &glassSize, bool circular) const
{
    if (!magnifierEdgeEase) {
        return viewerPos;
    }
    // Reference frame = the viewport (the visible scroll region the page is drawn in), not the
    // top-level window. The window includes the toolbar/chrome, whose height inflates the
    // frame well beyond the page and pushes the "center" off, so the loupe eases too hard. In
    // fullscreen the viewport already fills the screen, so this matches the old behavior there.
    const double vpW = viewport()->width();
    const double vpH = viewport()->height();
    if (vpW <= 1.0 || vpH <= 1.0) {
        return viewerPos;
    }

    // Normalized cursor offset from the viewport center, per axis in [-1, 1].
    const double tx = qBound(-1.0, (viewerPos.x() - vpW / 2.0) / (vpW / 2.0), 1.0);
    const double ty = qBound(-1.0, (viewerPos.y() - vpH / 2.0) / (vpH / 2.0), 1.0);

    // Easing helps on an axis where there is off-page content to bring toward the cursor. A page
    // that overflows the viewport always qualifies; a letterboxed page qualifies until the
    // margin beside it grows large enough that easing would mostly reveal background. Cancel the
    // curve on an axis only once the letterbox reaches minLetterboxFraction of the page's size on
    // that axis, so a page that nearly fills the viewport (thin margin or exact fit) still eases
    // instead of dropping the effect the instant the page is a hair smaller than the view.
    bool easeX = true;
    bool easeY = true;
    if (const QWidget *w = widget()) {
        double pageW = w->width();
        const double pageH = w->height();
        if (continuousScroll && w == continuousWidget && continuousViewModel != nullptr) {
            // The continuous widget fills the viewport width with each page centered inside it,
            // so the page under the cursor — not the widget — is the horizontal extent. The
            // document is contiguous vertically, so the widget height is the vertical extent.
            const int cwY = viewerPos.y() + verticalScrollBar()->sliderPosition();
            const int idx = qBound(0, continuousViewModel->pageAtY(cwY), continuousViewModel->numPages() - 1);
            pageW = continuousViewModel->scaledPageSize(idx).width();
        }
        // Letterbox = how far the viewport exceeds the page on the axis; keep easing until it
        // reaches minLetterboxFraction of the page dimension (overflow and exact fit stay on the
        // "ease" side).
        easeX = (vpW - pageW) < minLetterboxFraction * pageW;
        easeY = (vpH - pageH) < minLetterboxFraction * pageH;
    }

    // Outward displacement, capped per axis at the loupe half-extent (the loupe's "reach") and
    // scaled by edgeStrength. The cap is what guarantees the cursor's point never leaves the
    // loupe view, and what makes a small loupe nearly linear while a large loupe eases hard.
    double dx = easeX ? edgeStrength * (glassSize.width() / 2.0) * edgeRamp(qAbs(tx)) * (tx < 0.0 ? -1.0 : 1.0) : 0.0;
    double dy = easeY ? edgeStrength * (glassSize.height() / 2.0) * edgeRamp(qAbs(ty)) * (ty < 0.0 ? -1.0 : 1.0) : 0.0;

    if (circular) {
        // A round loupe's limit is radial (Pythagorean): the displacement vector may not
        // exceed the radius, rather than being capped independently on each axis.
        const double radius = qMax(glassSize.width(), glassSize.height()) / 2.0;
        const double mag = std::sqrt(dx * dx + dy * dy);
        if (mag > radius && mag > 0.0) {
            dx *= radius / mag;
            dy *= radius / mag;
        }
    }

    // The displacement is a translation, so add it back in the caller's (viewport) frame.
    return QPoint(qRound(viewerPos.x() + dx), qRound(viewerPos.y() + dy));
}

QImage Viewer::grabMagnifiedRegion(const QPoint &viewerPos, const QSize &glassSize, float zoomLevel) const
{
    // viewerPos is expected already eased (see MagnifyingGlass::updateImage / easeViewerPos):
    // this samples the loupe's *content*, which swims a little toward the edge relative to the
    // loupe widget (which itself follows the cursor).
    const int glassW = glassSize.width();
    const int glassH = glassSize.height();
    const int zoomW = static_cast<int>(glassW * zoomLevel);
    const int zoomH = static_cast<int>(glassH * zoomLevel);
    const QColor bgColor = Configuration::getConfiguration().getBackgroundColor(theme.viewer.defaultBackgroundColor);

    if (continuousScroll) {
        // --- continuous mode ---
        // map viewer coords to continuousWidget coords
        const int scrollPos = verticalScrollBar()->sliderPosition();
        const int cwX = viewerPos.x();
        const int cwY = viewerPos.y() + scrollPos;
        const int widgetW = continuousWidget->width();

        // use the page under the cursor to derive source-to-widget scale factors,
        // so the result image is sized at source resolution (like single-page mode)
        int centerPageIdx = continuousViewModel->pageAtY(cwY);
        centerPageIdx = qBound(0, centerPageIdx, continuousViewModel->numPages() - 1);
        const QImage *centerImg = continuousPageProvider->image(centerPageIdx);
        const QSize centerScaledSize = continuousViewModel->scaledPageSize(centerPageIdx);

        float wFactor = 1.0f, hFactor = 1.0f;
        if (centerImg && !centerImg->isNull() && !centerScaledSize.isEmpty()) {
            wFactor = static_cast<float>(centerImg->width()) / centerScaledSize.width();
            hFactor = static_cast<float>(centerImg->height()) / centerScaledSize.height();
        }

        // result image sized in source-resolution pixels (full quality)
        const int resultW = static_cast<int>(zoomW * wFactor);
        const int resultH = static_cast<int>(zoomH * hFactor);

        QImage result(resultW, resultH, QImage::Format_RGB32);
        result.fill(bgColor);

        // zoom region in widget coordinates (centered on cursor)
        const int regionLeft = cwX - zoomW / 2;
        const int regionTop = cwY - zoomH / 2;
        const int regionRight = regionLeft + zoomW;
        const int regionBottom = regionTop + zoomH;

        // find which pages overlap the zoom region
        int firstPage = continuousViewModel->pageAtY(regionTop);
        int lastPage = continuousViewModel->pageAtY(regionBottom);
        firstPage = qBound(0, firstPage, continuousViewModel->numPages() - 1);
        lastPage = qBound(0, lastPage, continuousViewModel->numPages() - 1);

        QPainter painter(&result);
        for (int i = firstPage; i <= lastPage; ++i) {
            const QImage *srcImg = continuousPageProvider->image(i);
            if (!srcImg || srcImg->isNull()) {
                continue;
            }

            const QSize scaledSize = continuousViewModel->scaledPageSize(i);
            const int pageY = continuousViewModel->yPositionForPage(i);
            int pageX = (widgetW - scaledSize.width()) / 2;
            if (pageX < 0) {
                pageX = 0;
            }

            // intersection of zoom region and page rect (widget coords)
            const int isectLeft = qMax(regionLeft, pageX);
            const int isectTop = qMax(regionTop, pageY);
            const int isectRight = qMin(regionRight, pageX + scaledSize.width());
            const int isectBottom = qMin(regionBottom, pageY + scaledSize.height());

            if (isectLeft >= isectRight || isectTop >= isectBottom) {
                continue;
            }

            // map intersection to source image coordinates (full resolution crop)
            const float pageScaleX = static_cast<float>(srcImg->width()) / scaledSize.width();
            const float pageScaleY = static_cast<float>(srcImg->height()) / scaledSize.height();

            const int srcX = static_cast<int>((isectLeft - pageX) * pageScaleX);
            const int srcY = static_cast<int>((isectTop - pageY) * pageScaleY);
            const int srcW = static_cast<int>((isectRight - isectLeft) * pageScaleX);
            const int srcH = static_cast<int>((isectBottom - isectTop) * pageScaleY);

            // destination in result image (source-resolution coordinates)
            const int dstX = static_cast<int>((isectLeft - regionLeft) * wFactor);
            const int dstY = static_cast<int>((isectTop - regionTop) * hFactor);
            const int dstW = static_cast<int>((isectRight - isectLeft) * wFactor);
            const int dstH = static_cast<int>((isectBottom - isectTop) * hFactor);

            QImage cropped = srcImg->copy(srcX, srcY, srcW, srcH);
            if (cropped.size() != QSize(dstW, dstH)) {
                cropped = cropped.scaled(dstW, dstH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
            painter.drawImage(dstX, dstY, cropped);
        }

        return result;
    }

    // --- single-page mode ---
    const QPixmap sourceImage = currentPage != nullptr ? *currentPage : QPixmap();
    if (sourceImage.isNull()) {
        QImage result(zoomW, zoomH, QImage::Format_RGB32);
        result.setDevicePixelRatio(devicePixelRatioF());
        result.fill(bgColor);
        return result;
    }

    const int iWidth = sourceImage.width();
    const int iHeight = sourceImage.height();
    const float wFactor = static_cast<float>(iWidth) / widget()->width();
    const float hFactor = static_cast<float>(iHeight) / widget()->height();
    const int zoomWScaled = static_cast<int>(zoomW * wFactor);
    const int zoomHScaled = static_cast<int>(zoomH * hFactor);

    const int scrollPos = verticalScrollBar()->sliderPosition();
    int xp, yp;
    if (verticalScrollBar()->minimum() == verticalScrollBar()->maximum()) {
        xp = static_cast<int>(((viewerPos.x() - widget()->pos().x()) * wFactor) - zoomWScaled / 2);
        yp = static_cast<int>((viewerPos.y() - widget()->pos().y() + scrollPos) * hFactor - zoomHScaled / 2);
    } else {
        xp = static_cast<int>(((viewerPos.x() - widget()->pos().x()) * wFactor) - zoomWScaled / 2);
        yp = static_cast<int>((viewerPos.y() + scrollPos) * hFactor - zoomHScaled / 2);
    }

    int xOffset = 0, yOffset = 0;
    int zw = zoomWScaled, zh = zoomHScaled;
    bool outImage = false;
    if (xp < 0) {
        xOffset = -xp;
        xp = 0;
        zw -= xOffset;
        outImage = true;
    }
    if (yp < 0) {
        yOffset = -yp;
        yp = 0;
        zh -= yOffset;
        outImage = true;
    }
    if (xp + zoomWScaled >= iWidth) {
        zw -= xp + zw - iWidth;
        outImage = true;
    }
    if (yp + zoomHScaled >= iHeight) {
        zh -= yp + zh - iHeight;
        outImage = true;
    }

    if (outImage) {
        // The magnified image is kept at source resolution (device pixel ratio 1),
        // matching the in-bounds path below. Do NOT set a >1 device pixel ratio here:
        // a QPainter draws in logical coordinates, so painting the DPR-1 source crop
        // onto a DPR>1 image would scale the content up by the ratio (and clip it),
        // producing a sudden magnification jump at the image edges on HiDPI displays.
        QImage img(zoomWScaled, zoomHScaled, QImage::Format_RGB32);
        img.fill(bgColor);
        if (zw > 0 && zh > 0) {
            QPainter painter(&img);
            painter.drawPixmap(xOffset, yOffset, sourceImage.copy(xp, yp, zw, zh));
        }
        return img;
    }

    return sourceImage.copy(xp, yp, zoomWScaled, zoomHScaled).toImage();
}

void Viewer::magnifyingGlassSwitch()
{
    magnifyingGlassShown ? hideMagnifyingGlass() : showMagnifyingGlass();
}

void Viewer::showMagnifyingGlass()
{
    if (render->hasLoadedComic()) {
        QPoint p = QPoint(cursor().pos().x(), cursor().pos().y());
        p = this->parentWidget()->mapFromGlobal(p);
        mglass->move(static_cast<int>(p.x() - float(mglass->width()) / 2), static_cast<int>(p.y() - float(mglass->height()) / 2));
        mglass->show();
        mglass->updateImage(mglass->x() + mglass->width() / 2, mglass->y() + mglass->height() / 2);
        setMagnifyingGlassShown(true);
    }
}

void Viewer::hideMagnifyingGlass()
{
    mglass->hide();
    setMagnifyingGlassShown(false);
}

void Viewer::setMagnifyingGlassShown(bool shown)
{
    if (magnifyingGlassShown != shown) {
        magnifyingGlassShown = shown;
        emit magnifyingGlassVisibilityChanged(magnifyingGlassShown);
    }
}

void Viewer::informationSwitch()
{
    informationLabel->updatePosition();
    information ? informationLabel->hide() : informationLabel->show();
    // informationLabel->move(QPoint((width()-informationLabel->width())/2,0));
    information = !information;
    Configuration::getConfiguration().setShowInformation(information);
    // TODO it shouldn't be neccesary
    informationLabel->adjustSize();
    informationLabel->update();
}

void Viewer::updateInformation()
{
    if (render->hasLoadedComic()) {
        QString pagesInformation;
        if (continuousScroll && continuousViewModel->numPages() > 0) {
            pagesInformation = QString::number(continuousViewModel->readingProgressPage() + 1) + "/" + QString::number(continuousViewModel->numPages());
        } else {
            pagesInformation = render->getCurrentPagesInformation();
        }

        auto displayTime = Configuration::getConfiguration().getShowTimeInInformation();
        if (displayTime) {
            informationLabel->setText(pagesInformation + " - " + QTime::currentTime().toString("HH:mm"));
        } else {
            informationLabel->setText(pagesInformation);
        }

        informationLabel->adjustSize();
        informationLabel->update(); // TODO it shouldn't be neccesary
        informationLabel->updatePosition();
    }
}

void Viewer::goToFlowSwitch()
{
    goToFlow->isVisible() ? animateHideGoToFlow() : showGoToFlow();
}

void Viewer::translatorSwitch()
{
    translator->isVisible() ? animateHideTranslator() : animateShowTranslator();
}

bool Viewer::translatorIsVisible() const
{
    return translator->isVisible();
}

bool Viewer::goToFlowIsVisible() const
{
    return goToFlow->isVisible();
}

void Viewer::showGoToFlow()
{
    if (render->hasLoadedComic()) {
        animateShowGoToFlow();
    }
}

void Viewer::animateShowGoToFlow()
{
    if (goToFlow->isHidden() && showGoToFlowAnimation->state() != QPropertyAnimation::Running) {
        disconnect(showGoToFlowAnimation, &QAbstractAnimation::finished, goToFlow, &QWidget::hide);
        connect(showGoToFlowAnimation, &QAbstractAnimation::finished, this, &Viewer::moveCursoToGoToFlow);
        showGoToFlowAnimation->setStartValue(QPoint((width() - goToFlow->width()) / 2, height() - 10));
        showGoToFlowAnimation->setEndValue(QPoint((width() - goToFlow->width()) / 2, height() - goToFlow->height()));
        showGoToFlowAnimation->start();
        goToFlow->show();
        goToFlow->setPageNumber(render->getIndex());
        goToFlow->centerSlide(render->getIndex());
        goToFlow->setFocus(Qt::OtherFocusReason);
    }
}

void Viewer::animateHideGoToFlow()
{
    if (goToFlow->isVisible() && showGoToFlowAnimation->state() != QPropertyAnimation::Running) {
        connect(showGoToFlowAnimation, &QAbstractAnimation::finished, goToFlow, &QWidget::hide);
        disconnect(showGoToFlowAnimation, &QAbstractAnimation::finished, this, &Viewer::moveCursoToGoToFlow);
        showGoToFlowAnimation->setStartValue(QPoint((width() - goToFlow->width()) / 2, height() - goToFlow->height()));
        showGoToFlowAnimation->setEndValue(QPoint((width() - goToFlow->width()) / 2, height()));
        showGoToFlowAnimation->start();
        goToFlow->centerSlide(render->getIndex());
        goToFlow->setPageNumber(render->getIndex());
        this->setFocus(Qt::OtherFocusReason);
    }
}

void Viewer::moveCursoToGoToFlow()
{
    if (Configuration::getConfiguration().getDisableShowOnMouseOver()) {
        return;
    }

    // Move cursor to goToFlow widget on show (this avoid hide when mouse is moved)
    int y = goToFlow->pos().y();
    int x1 = goToFlow->pos().x();
    int x2 = x1 + goToFlow->width();
    QPoint cursorPos = mapFromGlobal(cursor().pos());
    int cursorX = cursorPos.x();
    int cursorY = cursorPos.y();

    if (cursorY <= y)
        cursorY = y + 10;
    if (cursorX <= x1)
        cursorX = x1 + 10;
    if (cursorX >= x2)
        cursorX = x2 - 10;
    moveCursorTo(mapToGlobal(QPoint(cursorX, cursorY)));
    hideCursorTimer->stop();
    showCursor();
}

void Viewer::rotateLeft()
{
    render->rotateLeft();
    // Mirror Render's rotation (which it never resets) so the provider decodes
    // at the same angle; continuousRotation stays in lockstep with Render.
    continuousRotation = (continuousRotation == 0) ? 270 : continuousRotation - 90;
    continuousPageProvider->setRotation(continuousRotation);
    if (continuousScroll) {
        continuousWidget->invalidateScaledImageCache();
    }
}
void Viewer::rotateRight()
{
    render->rotateRight();
    continuousRotation = (continuousRotation + 90) % 360;
    continuousPageProvider->setRotation(continuousRotation);
    if (continuousScroll) {
        continuousWidget->invalidateScaledImageCache();
    }
}

// TODO
void Viewer::setBookmark(bool set)
{
    render->setBookmark();
    if (set) // add bookmark
    {
        render->setBookmark();
    } else // remove bookmark
    {
        render->removeBookmark();
    }
}

void Viewer::save()
{
    if (render->hasLoadedComic()) {
        // Render's current page lags the scroll position in continuous mode;
        // bring it in line before persisting reading progress.
        syncRenderToContinuousPage();
        render->save();
    }
}

void Viewer::doublePageSwitch()
{
    doublePage = !doublePage;
    render->doublePageSwitch();
    Configuration::getConfiguration().setDoublePage(doublePage);
}

void Viewer::setContinuousScrollImpl(bool enabled, bool persistSettings)
{
    if (continuousScroll == enabled) {
        return;
    }

    continuousScroll = enabled;
    if (persistSettings) {
        Configuration::getConfiguration().setContinuousScroll(continuousScroll);
    }

    if (continuousScroll) {
        continuousViewModel->setZoomFactor(zoom);
        if (render->hasLoadedComic()) {
            continuousViewModel->setViewportSize(viewport()->width(), viewport()->height());
            continuousViewModel->setNumPages(render->numPages());
            continuousPageProvider->setNumPages(render->numPages());
            continuousPageProvider->setRotation(continuousRotation);
            lastCenterPage = render->getIndex();
            continuousViewModel->setAnchorPage(lastCenterPage);
            continuousPageProvider->requestRange(lastCenterPage - 2, lastCenterPage + 4);
            setActiveWidget(continuousWidget);
            scrollToCurrentContinuousPage();
            continuousWidget->update();
            viewport()->update();
        }
    } else {
        // Leaving continuous mode: bring Render to the reading position so
        // single-page mode shows the page the user was on.
        if (render->hasLoadedComic() && lastCenterPage >= 0) {
            const int page = qBound(0, lastCenterPage, static_cast<int>(render->numPages()) - 1);
            if (static_cast<int>(render->getIndex()) != page) {
                render->goTo(page);
            }
        }
        lastCenterPage = -1;
        if (render->hasLoadedComic()) {
            updatePage();
        }
    }
}

void Viewer::setContinuousScrollWithoutStoringSetting(bool enabled)
{
    setContinuousScrollImpl(enabled, false);
}

void Viewer::setContinuousScroll(bool enabled)
{
    setContinuousScrollImpl(enabled, true);
}

void Viewer::onContinuousScroll(int value)
{
    if (!continuousScroll || !render->hasLoadedComic() || applyingContinuousModelState) {
        return;
    }

    continuousViewModel->setScrollYFromUser(value);

    int currentPage = continuousViewModel->readingProgressPage();

    if (currentPage != lastCenterPage && currentPage >= 0) {
        lastCenterPage = currentPage;
        continuousViewModel->setAnchorPage(currentPage);
        // No render->goTo() here: that drives Render's blocking page buffer and
        // would stall scrolling. The page provider decodes off-thread, driven by
        // the widget's paint. Render's current page is synced lazily (save / mode
        // switch) instead.
        updateInformation();
        emit pageAvailable(true);
    }
}

void Viewer::onContinuousViewModelChanged()
{
    if (!continuousScroll) {
        return;
    }

    applyContinuousStateToUi();
}

void Viewer::onContinuousPageRendered(int absolutePageIndex)
{
    if (!continuousScroll || !render->hasLoadedComic()) {
        return;
    }

    const QImage *img = continuousPageProvider->image(absolutePageIndex);
    if (!img || img->isNull()) {
        return;
    }

    continuousViewModel->setPageNaturalSize(absolutePageIndex, img->size());
}

void Viewer::probeContinuousBufferedPages()
{
    if (!render->hasLoadedComic()) {
        return;
    }

    const int totalPages = static_cast<int>(render->numPages());
    for (int i = 0; i < totalPages; ++i) {
        const QImage *img = continuousPageProvider->image(i);
        if (img && !img->isNull()) {
            continuousViewModel->setPageNaturalSize(i, img->size());
        }
    }
}

void Viewer::applyContinuousStateToUi()
{
    if (!continuousScroll) {
        return;
    }

    // Save/restore both guards: this can run re-entrantly (e.g. setActiveWidget's
    // setWidget triggers a viewport resize -> setViewportSize -> recompute -> here).
    // Unconditionally clearing them would defeat an outer block: setActiveWidget
    // blocks the scrollbar so setWidget's reset-to-0 won't fire onContinuousScroll;
    // if we unblock it here, that reset then snaps reading progress to the cover.
    const bool wasApplying = applyingContinuousModelState;
    applyingContinuousModelState = true;

    continuousWidget->setFixedHeight(continuousViewModel->totalHeight());
    continuousWidget->updateGeometry();

    auto *sb = verticalScrollBar();
    const int target = qBound(sb->minimum(), continuousViewModel->scrollY(), sb->maximum());
    const bool wasBlocked = sb->blockSignals(true);
    sb->setValue(target);
    sb->blockSignals(wasBlocked);

    applyingContinuousModelState = wasApplying;

    continuousWidget->update();
    viewport()->update();
}

void Viewer::scrollToCurrentContinuousPage()
{
    if (lastCenterPage < 0) {
        return;
    }

    continuousViewModel->setCurrentPage(lastCenterPage);
}

void Viewer::syncRenderToContinuousReadingProgress()
{
    if (!continuousScroll || continuousViewModel->numPages() <= 0) {
        return;
    }

    lastCenterPage = continuousViewModel->readingProgressPage();
    continuousViewModel->setAnchorPage(lastCenterPage);
    syncRenderToContinuousPage();
}

void Viewer::syncRenderToContinuousPage()
{
    if (!continuousScroll || !render->hasLoadedComic() || lastCenterPage < 0) {
        return;
    }

    const int page = qBound(0, lastCenterPage, static_cast<int>(render->numPages()) - 1);
    if (static_cast<int>(render->getIndex()) == page) {
        return;
    }

    syncingRenderFromContinuousScroll = true;
    render->goTo(page);
    syncingRenderFromContinuousScroll = false;
}

void Viewer::onNumPagesReady(unsigned int numPages)
{
    if (continuousScroll && numPages > 0) {
        setActiveWidget(continuousWidget);

        continuousViewModel->setViewportSize(viewport()->width(), viewport()->height());
        continuousViewModel->setNumPages(numPages);
        continuousPageProvider->setNumPages(numPages);

        int page = lastCenterPage;
        if (page < 0) {
            page = render->getIndex();
        }
        page = qBound(0, page, static_cast<int>(numPages) - 1);
        lastCenterPage = page;
        continuousViewModel->setAnchorPage(page);

        // Kick off decoding around the starting page so the first screens are
        // ready as soon as their raw bytes load.
        continuousPageProvider->requestRange(page - 2, page + 4);

        scrollToCurrentContinuousPage();
    }
}

void Viewer::onRenderPageChanged(int page)
{
    if (!continuousScroll || page < 0 || page == lastCenterPage || syncingRenderFromContinuousScroll) {
        return;
    }

    lastCenterPage = page;
    continuousViewModel->setAnchorPage(page);
    scrollToCurrentContinuousPage();
}

void Viewer::setMangaModeImpl(bool manga, bool persistSettings)
{
    if (doubleMangaPage == manga) {
        return;
    }

    doubleMangaPage = manga;

    if (persistSettings) {
        Configuration &config = Configuration::getConfiguration();
        config.setDoubleMangaPage(doubleMangaPage);
        goToFlow->updateConfig(config.getSettings());
    }

    render->setManga(manga);
    goToFlow->setFlowRightToLeft(doubleMangaPage);
}

void Viewer::setMangaWithoutStoringSetting(bool manga)
{
    setMangaModeImpl(manga, false);
}

void Viewer::doubleMangaPageSwitch()
{
    setMangaModeImpl(!doubleMangaPage, true);
}

void Viewer::resetContent()
{
    configureContent(tr("Press 'O' to open comic."));
    goToFlow->reset();
    continuousViewModel->reset();
    continuousPageProvider->reset();
    continuousWidget->reset();
    lastCenterPage = -1;
    emit reset();
}

void Viewer::setLoadingMessage()
{
    if (magnifyingGlassShown) {
        hideMagnifyingGlass();
        restoreMagnifyingGlass = true;
    }
    emit pageAvailable(false);
    if (!continuousScroll) {
        configureContent(tr("Loading...please wait!"));
    }
}

void Viewer::setPageUnavailableMessage()
{
    if (magnifyingGlassShown) {
        hideMagnifyingGlass();
        restoreMagnifyingGlass = true;
    }
    emit pageAvailable(false);
    configureContent(tr("Page not available!"));
}

void Viewer::configureContent(QString msg)
{
    messageLabel->setText(msg);
    setActiveWidget(messageLabel);
    setFocus(Qt::ShortcutFocusReason);
}

void Viewer::hideCursor()
{
#ifdef Q_OS_MACOS // TODO_Y_MAC_UI isn't BlankCursor supported in macos?
    setCursor(QCursor(QBitmap(1, 1), QBitmap(1, 1)));
#else
    setCursor(Qt::BlankCursor);
#endif
}
void Viewer::showCursor()
{
    if (drag)
        setCursor(Qt::ClosedHandCursor);
    else
        setCursor(Qt::OpenHandCursor);
}

void Viewer::updateOptions()
{
    goToFlow->setFlowType(Configuration::getConfiguration().getFlowType());
    updateBackgroundColor(Configuration::getConfiguration().getBackgroundColor(theme.viewer.defaultBackgroundColor));
    updateContentSize();
    updateInformation();
}

void Viewer::updateBackgroundColor(const QColor &color)
{
    QPalette palette;
    palette.setColor(backgroundRole(), color);
    setPalette(palette);
}

void Viewer::applyTheme(const Theme &theme)
{
    const auto viewerTheme = theme.viewer;

    updateBackgroundColor(Configuration::getConfiguration().getBackgroundColor(viewerTheme.defaultBackgroundColor));

    const QString textColor = viewerTheme.defaultTextColor.name(QColor::HexArgb);
    messageLabel->setStyleSheet(QStringLiteral("QLabel { color : %1; background: transparent; }").arg(textColor));
    content->setStyleSheet(QStringLiteral("QLabel { background: transparent; }"));
}

void Viewer::animateShowTranslator()
{
    if (translator->isHidden() && translatorAnimation->state() != QPropertyAnimation::Running) {
        disconnect(translatorAnimation, &QAbstractAnimation::finished, translator, &QWidget::hide);
        if (translatorXPos == -10000)
            translatorXPos = (width() - translator->width()) / 2;
        int x = qMax(0, qMin(translatorXPos, width() - translator->width()));
        if (translator->pos().x() < 0) {
            translatorAnimation->setStartValue(QPoint(-translator->width(), translator->pos().y()));
        } else {
            translatorAnimation->setStartValue(QPoint(width() + translator->width(), translator->pos().y()));
        }
        translatorAnimation->setEndValue(QPoint(x, translator->pos().y()));
        translatorAnimation->start();
        translator->show();
        translator->setFocus(Qt::OtherFocusReason);
    }
}
void Viewer::animateHideTranslator()
{
    if (translator->isVisible() && translatorAnimation->state() != QPropertyAnimation::Running) {
        connect(translatorAnimation, &QAbstractAnimation::finished, translator, &QWidget::hide);
        translatorAnimation->setStartValue(QPoint(translatorXPos = translator->pos().x(), translator->pos().y()));
        if ((translator->width() / 2) + translator->pos().x() <= width() / 2)
            translatorAnimation->setEndValue(QPoint(-translator->width(), translator->pos().y()));
        else
            translatorAnimation->setEndValue(QPoint(width() + translator->width(), translator->pos().y()));
        translatorAnimation->start();
        this->setFocus(Qt::OtherFocusReason);
    }
}

void Viewer::mousePressEvent(QMouseEvent *event)
{
    mouseHandler->mousePressEvent(event);
}

void Viewer::mouseReleaseEvent(QMouseEvent *event)
{
    mouseHandler->mouseReleaseEvent(event);
}

void Viewer::mouseMoveEvent(QMouseEvent *event)
{
    mouseHandler->mouseMoveEvent(event);
}

bool Viewer::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == continuousWidget && event->type() == QEvent::MouseMove) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        // Map position from continuousWidget coords to Viewer coords so the
        // go-to-flow proximity check and cursor management work correctly.
        QPointF viewerPos = mapFromGlobal(mouseEvent->globalPosition().toPoint());
        QMouseEvent mappedEvent(mouseEvent->type(),
                                viewerPos,
                                mouseEvent->globalPosition(),
                                mouseEvent->button(),
                                mouseEvent->buttons(),
                                mouseEvent->modifiers());
        mouseHandler->mouseMoveEvent(&mappedEvent);
        // Consume this event so we don't process the same drag movement again
        // via Viewer::mouseMoveEvent() after bubbling.
        event->accept();
        return true;
    }
    return QScrollArea::eventFilter(obj, event);
}

bool Viewer::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        return gestureEvent(static_cast<QGestureEvent *>(event));
    }
    return QScrollArea::event(event);
}

void Viewer::captureZoomAnchor()
{
    zoomAnchorViewport = viewport()->mapFromGlobal(QCursor::pos());
    if (content->width() > 0 && content->height() > 0) {
        const QPoint cursorInContent = content->mapFrom(viewport(), zoomAnchorViewport);
        zoomAnchorNormX = std::clamp(double(cursorInContent.x()) / content->width(), 0.0, 1.0);
        zoomAnchorNormY = std::clamp(double(cursorInContent.y()) / content->height(), 0.0, 1.0);
    } else {
        zoomAnchorNormX = 0.5;
        zoomAnchorNormY = 0.5;
    }
}

bool Viewer::applyZoomAtAnchor(int newZoom)
{
    newZoom = std::clamp(newZoom, 30, 500);
    if (newZoom == zoom) {
        return false;
    }

    if (continuousScroll) {
        zoom = newZoom;
        continuousViewModel->setZoomFactor(zoom);
        continuousWidget->invalidateScaledImageCache();
    } else {
        const int previousZoom = zoom;
        zoom = newZoom;

        if (!zoomPreviewActive) {
            // Reuse the current high-quality pixmap while the label follows the requested
            // geometry. The normal renderer replaces it after the interaction pauses.
            scaledContentsBeforeZoomPreview = content->hasScaledContents();
            zoomPreviewBaseSize = content->size();
            zoomPreviewBaseZoom = previousZoom;
            content->setScaledContents(true);
            zoomPreviewActive = true;
        }

        const double scale = static_cast<double>(newZoom) / zoomPreviewBaseZoom;
        content->resize(std::max(1, qRound(zoomPreviewBaseSize.width() * scale)),
                        std::max(1, qRound(zoomPreviewBaseSize.height() * scale)));
        restoreZoomAnchor();
        zoomPreviewFinishTimer->start();
    }

    zoomHud->setText(QStringLiteral("<span style=\"color:white; font-size:12px;\">%1%</span>").arg(zoom));
    positionZoomHud();
    zoomHud->show();

    emit zoomUpdated(zoom);
    return true;
}

void Viewer::restoreZoomAnchor()
{
    const int alignX = std::max(0, (viewport()->width() - content->width()) / 2);
    const int alignY = std::max(0, (viewport()->height() - content->height()) / 2);
    const int targetH = std::lround(zoomAnchorNormX * content->width()) + alignX - zoomAnchorViewport.x();
    const int targetV = std::lround(zoomAnchorNormY * content->height()) + alignY - zoomAnchorViewport.y();
    horizontalScrollBar()->setValue(targetH);
    verticalScrollBar()->setValue(targetV);
}

void Viewer::cancelZoomPreview()
{
    if (!zoomPreviewActive) {
        return;
    }

    zoomPreviewFinishTimer->stop();
    content->setScaledContents(scaledContentsBeforeZoomPreview);
    zoomPreviewActive = false;
}

void Viewer::renderFinalZoomImage()
{
    if (!zoomPreviewActive) {
        return;
    }

    cancelZoomPreview();
    updateContentSize();
    restoreZoomAnchor();
}

void Viewer::positionZoomHud()
{
    const int margin = 16;
    zoomHud->move(width() - zoomHud->width() - margin,
                  height() - zoomHud->height() - margin);
    zoomHud->raise();
}

bool Viewer::gestureEvent(QGestureEvent *event)
{
    if (QGesture *g = event->gesture(Qt::PinchGesture)) {
        auto *pinch = static_cast<QPinchGesture *>(g);
        if (!render->hasLoadedComic()) {
            event->accept(pinch);
            return true;
        }
        if (pinch->state() == Qt::GestureStarted) {
            zoomHudHideTimer->stop();
            pinchStartZoom = zoom;
            captureZoomAnchor();
        }
        int newZoom = std::clamp<int>(std::lround(pinchStartZoom * pinch->totalScaleFactor()), 30, 500);
        applyZoomAtAnchor(newZoom);
        if (pinch->state() == Qt::GestureFinished || pinch->state() == Qt::GestureCanceled) {
            renderFinalZoomImage();
            zoomHud->hide();
        }
        event->accept(pinch);
        return true;
    }
    return QScrollArea::event(event);
}

void Viewer::setActiveWidget(QWidget *w)
{
    if (widget() == w) {
        return;
    }
    cancelZoomPreview();
    verticalScrollBar()->blockSignals(true);
    takeWidget();
    const bool isContinuous = (w == continuousWidget);
    setWidgetResizable(isContinuous);
    setVerticalScrollBarPolicy(isContinuous ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    setWidget(w);
    verticalScrollBar()->blockSignals(false);
}

void Viewer::updateZoomRatio(int ratio)
{
    captureZoomAnchor();
    applyZoomAtAnchor(ratio);
    zoomHud->hide();
}

bool Viewer::getIsMangaMode()
{
    return doubleMangaPage;
}

void Viewer::updateConfig(QSettings *settings)
{
    goToFlow->updateConfig(settings);

    mglass->setCircular(Configuration::getConfiguration().getMagnifyingGlassCircular());
    mglass->setRing(Configuration::getConfiguration().getMagnifyingGlassRing());
    magnifierEdgeEase = Configuration::getConfiguration().getMagnifyingGlassEdgeEase();

    QPalette palette;
    palette.setColor(backgroundRole(), Configuration::getConfiguration().getBackgroundColor(theme.viewer.defaultBackgroundColor));
    setPalette(palette);
}

// deprecated
void Viewer::updateImageOptions()
{
    render->reload();
    // Settings-driven image options may have changed; force the provider to
    // re-decode with the current settings.
    continuousPageProvider->invalidate();
    if (continuousScroll) {
        continuousWidget->invalidateScaledImageCache();
    }
}

void Viewer::updateFilters(int brightness, int contrast, int gamma)
{
    render->updateFilters(brightness, contrast, gamma);
    continuousPageProvider->setFilters(brightness, contrast, gamma);
    if (continuousScroll) {
        continuousWidget->invalidateScaledImageCache();
    }
}

void Viewer::setBookmarks()
{
    bd->setBookmarks(*render->getBookmarks());
}

void Viewer::offsetDoublePageToTheLeft()
{
    if (!doublePage) {
        return;
    }

    render->offsetDoublePage(doubleMangaPage ? -1 : 1);

    updateInformation();
}

void Viewer::offsetDoublePageToTheRight()
{
    if (!doublePage) {
        return;
    }

    render->offsetDoublePage(doubleMangaPage ? 1 : -1);

    updateInformation();
}

void Viewer::showIsCoverMessage()
{
    if (!shouldOpenPrevious) {
        notificationsLabel->setText(tr("Cover!"));
        notificationsLabel->flash();
        shouldOpenPrevious = true;
    } else {
        shouldOpenPrevious = false;
        emit openPreviousComic();
    }

    shouldOpenNext = false; // single page comic
}

void Viewer::showIsLastMessage()
{
    if (!shouldOpenNext) {
        notificationsLabel->setText(tr("Last page!"));
        notificationsLabel->flash();
        shouldOpenNext = true;
    } else {
        shouldOpenNext = false;
        emit openNextComic();
    }

    shouldOpenPrevious = false; // single page comic
}

unsigned int Viewer::getIndex()
{
    if (continuousScroll && continuousViewModel->numPages() > 0) {
        return continuousViewModel->readingProgressPage() + 1;
    }
    return render->getIndex() + 1;
}

int Viewer::getCurrentPageNumber()
{
    if (continuousScroll && continuousViewModel->numPages() > 0) {
        return continuousViewModel->readingProgressPage();
    }
    return render->getIndex();
}

void Viewer::updateComic(ComicDB &comic)
{
    if (render->hasLoadedComic()) {
        // set currentPage
        if (continuousScroll && continuousViewModel->numPages() > 0) {
            comic.info.currentPage = getCurrentPageNumber() + 1;
        } else if (!doublePage || (doublePage && render->currentPageIsDoublePage() == false)) {
            comic.info.currentPage = render->getIndex() + 1;
        } else {
            if (doublePage && render->currentPageIsDoublePage() && (render->getIndex() + 2 >= render->numPages())) {
                comic.info.currentPage = std::min(render->numPages(), render->getIndex() + 2);
            } else {
                comic.info.currentPage = std::min(render->numPages(), render->getIndex() + 1);
            }
        }
        // set bookmarks
        Bookmarks *boomarks = render->getBookmarks();
        QList<int> boomarksList = boomarks->getBookmarkPages();
        int numBookmarks = boomarksList.size();
        if (numBookmarks > 0)
            comic.info.bookmark1 = boomarksList[0];
        if (numBookmarks > 1)
            comic.info.bookmark2 = boomarksList[1];
        if (numBookmarks > 2)
            comic.info.bookmark3 = boomarksList[2];
        // set filters
        // TODO: avoid use settings for this...
        QSettings settings(YACReader::getSettingsPath() + "/YACReader.ini", QSettings::IniFormat);
        int brightness = settings.value(BRIGHTNESS, 0).toInt();
        int contrast = settings.value(CONTRAST, 100).toInt();
        int gamma = settings.value(GAMMA, 100).toInt();

        if (brightness != 0 || comic.info.brightness != -1)
            comic.info.brightness = brightness;
        if (contrast != 100 || comic.info.contrast != -1)
            comic.info.contrast = contrast;
        if (gamma != 100 || comic.info.gamma != -1)
            comic.info.gamma = gamma;
    }
}
