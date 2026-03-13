#include "viewer.h"
#include "continuous_page_widget.h"
#include "continuous_view_model.h"
#include "resize_image.h"
#include "configuration.h"
#include "magnifying_glass.h"
#include "goto_flow_widget.h"
#include "bookmarks_dialog.h"
#include "render.h"
#include "goto_dialog.h"
#include "translator.h"
#include "page_label_widget.h"
#include "notifications_label_widget.h"
#include "comic_db.h"

#include <QFile>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScrollBar>

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
      mouseHandler(std::make_unique<YACReader::MouseHandler>(this))
{
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
    continuousViewModel = new ContinuousViewModel(this);
    continuousWidget->setViewModel(continuousViewModel);
    continuousWidget->installEventFilter(this);
    //---------------------------------------
    mglass = new MagnifyingGlass(
            Configuration::getConfiguration().getMagnifyingGlassSize(),
            Configuration::getConfiguration().getMagnifyingGlassZoom(),
            this);

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

    hideCursorTimer = new QTimer();
    hideCursorTimer->setSingleShot(true);

    if (Configuration::getConfiguration().getDoublePage())
        doublePageSwitch();

    if (Configuration::getConfiguration().getDoubleMangaPage())
        doubleMangaPageSwitch();

    if (Configuration::getConfiguration().getContinuousScroll())
        setContinuousScroll(true);

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
    connect(render, &Render::pageRendered, continuousWidget, &ContinuousPageWidget::onPageAvailable);
    connect(render, &Render::pageRendered, this, &Viewer::onContinuousPageRendered);
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

    verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum());

    if (Configuration::getConfiguration().getShowInformation() && !information) {
        QTimer::singleShot(0, this, &Viewer::informationSwitch);
    }

    informationLabel->setText("...");
}

void Viewer::open(QString pathFile, int atPage)
{
    prepareForOpening();
    render->load(pathFile, atPage);
}

void Viewer::open(QString pathFile, const ComicDB &comic)
{
    prepareForOpening();
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
    if (doublePage && render->currentPageIsDoublePage()) {
        render->nextDoublePage();
    } else {
        render->nextPage();
    }
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
    if (doublePage && render->previousPageIsDoublePage()) {
        render->previousDoublePage();
    } else {
        render->previousPage();
    }
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
    if (doublePage) {
        if (!doubleMangaPage)
            currentPage = render->getCurrentDoublePage();
        else {
            currentPage = render->getCurrentDoubleMangaPage();
        }
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
        page.setDevicePixelRatio(dpr);
        content->setPixmap(page);

        emit backgroundChanges();
    }
    content->update(); // TODO, it shouldn't be neccesary
}

void Viewer::increaseZoomFactor()
{
    zoom = std::min(zoom + 10, 500);

    if (continuousScroll) {
        continuousViewModel->setZoomFactor(zoom);
    } else {
        updateContentSize();
    }
    notificationsLabel->setText(QString::number(getZoomFactor()) + "%");
    notificationsLabel->flash();

    emit zoomUpdated(zoom);
}
void Viewer::decreaseZoomFactor()
{
    zoom = std::max(zoom - 10, 30);

    if (continuousScroll) {
        continuousViewModel->setZoomFactor(zoom);
    } else {
        updateContentSize();
    }
    notificationsLabel->setText(QString::number(getZoomFactor()) + "%");
    notificationsLabel->flash();

    emit zoomUpdated(zoom);
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

    if (!event->pixelDelta().isNull()) {
        wheelEventTrackpad(event);
    } else {
        wheelEventMouse(event);
    }
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
    return content->pixmap();
}

QImage Viewer::grabMagnifiedRegion(const QPoint &viewerPos, const QSize &glassSize, float zoomLevel) const
{
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
        const QImage *centerImg = render->bufferedImage(centerPageIdx);
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
            const QImage *srcImg = render->bufferedImage(i);
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
    const QPixmap image = content->pixmap();
    if (image.isNull()) {
        QImage result(zoomW, zoomH, QImage::Format_RGB32);
        result.setDevicePixelRatio(devicePixelRatioF());
        result.fill(bgColor);
        return result;
    }

    const int iWidth = image.width();
    const int iHeight = image.height();
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
        QImage img(zoomWScaled, zoomHScaled, QImage::Format_RGB32);
        img.setDevicePixelRatio(devicePixelRatioF());
        img.fill(bgColor);
        if (zw > 0 && zh > 0) {
            QPainter painter(&img);
            painter.drawPixmap(xOffset, yOffset, image.copy(xp, yp, zw, zh));
        }
        return img;
    }

    return image.copy(xp, yp, zoomWScaled, zoomHScaled).toImage();
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
        auto displayTime = Configuration::getConfiguration().getShowTimeInInformation();
        if (displayTime) {
            informationLabel->setText(render->getCurrentPagesInformation() + " - " + QTime::currentTime().toString("HH:mm"));
        } else {
            informationLabel->setText(render->getCurrentPagesInformation());
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
    cursor().setPos(mapToGlobal(QPoint(cursorX, cursorY)));
    hideCursorTimer->stop();
    showCursor();
}

void Viewer::rotateLeft()
{
    render->rotateLeft();
}
void Viewer::rotateRight()
{
    render->rotateRight();
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
    if (render->hasLoadedComic())
        render->save();
}

void Viewer::doublePageSwitch()
{
    doublePage = !doublePage;
    render->doublePageSwitch();
    Configuration::getConfiguration().setDoublePage(doublePage);
}

void Viewer::setContinuousScroll(bool enabled)
{
    if (continuousScroll == enabled) {
        return;
    }
    continuousScroll = enabled;
    Configuration::getConfiguration().setContinuousScroll(continuousScroll);

    if (continuousScroll) {
        continuousViewModel->setZoomFactor(zoom);
        if (render->hasLoadedComic()) {
            continuousViewModel->setViewportSize(viewport()->width(), viewport()->height());
            continuousViewModel->setNumPages(render->numPages());
            // set the current page as model state before any layout/scroll happens
            lastCenterPage = render->getIndex();
            continuousViewModel->setAnchorPage(lastCenterPage);
            // pick up sizes of pages already in the buffer
            probeContinuousBufferedPages();
            // trigger a render cycle so new pages arrive via pageRendered signal
            render->update();
            setActiveWidget(continuousWidget);
            scrollToCurrentContinuousPage();
            continuousWidget->update();
            viewport()->update();
        }
        // if no comic is loaded, messageLabel stays as the active widget
    } else {
        lastCenterPage = -1;
        if (render->hasLoadedComic()) {
            updatePage();
        }
        // if no comic is loaded, messageLabel stays as the active widget
    }
}

void Viewer::onContinuousScroll(int value)
{
    if (!continuousScroll || !render->hasLoadedComic() || applyingContinuousModelState) {
        return;
    }

    continuousViewModel->setScrollYFromUser(value);

    int center = continuousViewModel->centerPage();

    if (center != lastCenterPage && center >= 0) {
        lastCenterPage = center;
        continuousViewModel->setAnchorPage(center);
        syncingRenderFromContinuousScroll = true;
        render->goTo(center);
        syncingRenderFromContinuousScroll = false;
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

    const QImage *img = render->bufferedImage(absolutePageIndex);
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
        const QImage *img = render->bufferedImage(i);
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

    applyingContinuousModelState = true;

    continuousWidget->setFixedHeight(continuousViewModel->totalHeight());
    continuousWidget->updateGeometry();

    auto *sb = verticalScrollBar();
    const int target = qBound(sb->minimum(), continuousViewModel->scrollY(), sb->maximum());
    sb->blockSignals(true);
    sb->setValue(target);
    sb->blockSignals(false);

    applyingContinuousModelState = false;

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

void Viewer::onNumPagesReady(unsigned int numPages)
{
    if (continuousScroll && numPages > 0) {
        setActiveWidget(continuousWidget);

        continuousViewModel->setViewportSize(viewport()->width(), viewport()->height());
        continuousViewModel->setNumPages(numPages);
        probeContinuousBufferedPages();

        int page = lastCenterPage;
        if (page < 0) {
            page = render->getIndex();
        }
        page = qBound(0, page, static_cast<int>(numPages) - 1);
        lastCenterPage = page;
        continuousViewModel->setAnchorPage(page);

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

void Viewer::setMangaWithoutStoringSetting(bool manga)
{
    doubleMangaPage = manga;
    render->setManga(manga);
    goToFlow->setFlowRightToLeft(doubleMangaPage);
}

void Viewer::doubleMangaPageSwitch()
{
    doubleMangaPage = !doubleMangaPage;
    render->doubleMangaPageSwitch();
    Configuration &config = Configuration::getConfiguration();
    config.setDoubleMangaPage(doubleMangaPage);
    goToFlow->setFlowRightToLeft(doubleMangaPage);
    goToFlow->updateConfig(config.getSettings());
}

void Viewer::resetContent()
{
    configureContent(tr("Press 'O' to open comic."));
    goToFlow->reset();
    continuousViewModel->reset();
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

void Viewer::setActiveWidget(QWidget *w)
{
    if (widget() == w) {
        return;
    }
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
    zoom = ratio;
    if (continuousScroll) {
        continuousViewModel->setZoomFactor(zoom);
    } else {
        updateContentSize();
    }
}

bool Viewer::getIsMangaMode()
{
    return doubleMangaPage;
}

void Viewer::updateConfig(QSettings *settings)
{
    goToFlow->updateConfig(settings);

    QPalette palette;
    palette.setColor(backgroundRole(), Configuration::getConfiguration().getBackgroundColor(theme.viewer.defaultBackgroundColor));
    setPalette(palette);
}

// deprecated
void Viewer::updateImageOptions()
{
    render->reload();
}

void Viewer::updateFilters(int brightness, int contrast, int gamma)
{
    render->updateFilters(brightness, contrast, gamma);
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

    if (doubleMangaPage) {
        render->previousPage();
    } else {
        render->nextPage();
    }

    updateInformation();
}

void Viewer::offsetDoublePageToTheRight()
{
    if (!doublePage) {
        return;
    }

    if (doubleMangaPage) {
        render->nextPage();
    } else {
        render->previousPage();
    }

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
    return render->getIndex() + 1;
}

int Viewer::getCurrentPageNumber()
{
    return render->getIndex();
}

void Viewer::updateComic(ComicDB &comic)
{
    if (render->hasLoadedComic()) {
        // set currentPage
        if (!doublePage || (doublePage && render->currentPageIsDoublePage() == false)) {
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
