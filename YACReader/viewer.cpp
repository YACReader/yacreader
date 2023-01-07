#include "viewer.h"
#include "magnifying_glass.h"
#include "configuration.h"
#include "magnifying_glass.h"
#include "goto_flow.h"
#ifndef NO_OPENGL
#include "goto_flow_gl.h"
#else
#include <QtWidgets>
#endif
#include "bookmarks_dialog.h"
#include "render.h"
#include "goto_dialog.h"
#include "translator.h"
#include "page_label_widget.h"
#include "notifications_label_widget.h"
#include "comic_db.h"
#include "shortcuts_manager.h"

#include "opengl_checker.h"

#include <QFile>
#include <QKeyEvent>

#include <QsLog.h>

Viewer::Viewer(QWidget *parent)
    : QScrollArea(parent),
      fullscreen(false),
      information(false),
      doublePage(false),
      doubleMangaPage(false),
      zoom(100),
      currentPage(nullptr),
      wheelStop(false),
      direction(1),
      drag(false),
      shouldOpenNext(false),
      shouldOpenPrevious(false),
      magnifyingGlassShown(false),
      restoreMagnifyingGlass(false)
{
    translator = new YACReaderTranslator(this);
    translator->hide();
    translatorAnimation = new QPropertyAnimation(translator, "pos");
    translatorAnimation->setDuration(150);
    translatorXPos = -10000;
    translator->move(-translator->width(), 10);
    // current comic page
    content = new QLabel(this);
    configureContent(tr("Press 'O' to open comic."));
    // scroll area configuration
    setBackgroundRole(QPalette::Dark);
    setWidget(content);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setAlignment(Qt::AlignCenter);

    QPalette palette;
    palette.setColor(backgroundRole(), Configuration::getConfiguration().getBackgroundColor());
    setPalette(palette);
    //---------------------------------------
    mglass = new MagnifyingGlass(Configuration::getConfiguration().getMagnifyingGlassSize(), this);
    mglass->hide();
    content->setMouseTracking(true);
    setMouseTracking(true);

    showCursor();

    goToDialog = new GoToDialog(this);

    QSettings *settings = new QSettings(YACReader::getSettingsPath() + "/YACReader.ini", QSettings::IniFormat);

    // CONFIG GOTO_FLOW--------------------------------------------------------
#ifndef NO_OPENGL

    OpenGLChecker openGLChecker;
    bool openGLAvailable = openGLChecker.hasCompatibleOpenGLVersion();

    if (openGLAvailable && !settings->contains(USE_OPEN_GL))
        settings->setValue(USE_OPEN_GL, 2);
    else if (!openGLAvailable)
        settings->setValue(USE_OPEN_GL, 0);

    if ((settings->value(USE_OPEN_GL).toBool() == true))
        goToFlow = new GoToFlowGL(this, Configuration::getConfiguration().getFlowType());
    else
        goToFlow = new GoToFlow(this, Configuration::getConfiguration().getFlowType());
#else
    goToFlow = new GoToFlow(this, Configuration::getConfiguration().getFlowType());
#endif
    goToFlow->setFocusPolicy(Qt::StrongFocus);
    goToFlow->hide();
    showGoToFlowAnimation = new QPropertyAnimation(goToFlow, "pos");
    showGoToFlowAnimation->setDuration(150);

    bd = new BookmarksDialog(this->parentWidget());

    render = new Render();

    hideCursorTimer = new QTimer();
    hideCursorTimer->setSingleShot(true);

    if (Configuration::getConfiguration().getDoublePage())
        doublePageSwitch();

    if (Configuration::getConfiguration().getDoubleMangaPage())
        doubleMangaPageSwitch();

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
}

Viewer::~Viewer()
{
    delete render;
    delete goToFlow;
    delete translator;
    delete translatorAnimation;
    delete content;
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
    connect(render, &Render::processingPage, this, &Viewer::setLoadingMessage);
    connect(render, &Render::currentPageIsBookmark, this, &Viewer::pageIsBookmark);
    connect(render, &Render::pageChanged, this, &Viewer::updateInformation);

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
    if (doubleMangaPage) {
        next();
    } else {
        prev();
    }
}

void Viewer::right()
{
    if (doubleMangaPage) {
        prev();
    } else {
        next();
    }
}

void Viewer::prev()
{
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
    render->goTo(page);
}

void Viewer::updatePage()
{
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
        // apply scaling
        content->resize(pagefit);

        // TODO: updtateContentSize should only scale the pixmap once
        if (devicePixelRatioF() > 1) // only in HDPI displays
        {
            QPixmap page = currentPage->scaled(content->width() * devicePixelRatioF(), content->height() * devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            page.setDevicePixelRatio(devicePixelRatioF());
            content->setPixmap(page);
        }

        emit backgroundChanges();
    }
    content->update(); // TODO, it shouldn't be neccesary
}

void Viewer::increaseZoomFactor()
{
    zoom = std::min(zoom + 10, 500);

    updateContentSize();
    notificationsLabel->setText(QString::number(getZoomFactor()) + "%");
    notificationsLabel->flash();

    emit zoomUpdated(zoom);
}
void Viewer::decreaseZoomFactor()
{
    zoom = std::max(zoom - 10, 30);

    updateContentSize();
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
    if (direction > 0)
        verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum());
    else
        verticalScrollBar()->setSliderPosition(verticalScrollBar()->maximum());
}

void Viewer::scrollDown()
{
    if (verticalScrollBar()->sliderPosition() == verticalScrollBar()->maximum()) {
        next();
    } else {
        int currentPos = verticalScrollBar()->sliderPosition();
        verticalScroller->setDuration(250);
        verticalScroller->setStartValue(currentPos);
        verticalScroller->setEndValue(nextPos);

        verticalScroller->start();

        emit backgroundChanges();
    }
}

void Viewer::scrollUp()
{
    if (verticalScrollBar()->sliderPosition() == verticalScrollBar()->minimum()) {
        prev();
    } else {
        int currentPos = verticalScrollBar()->sliderPosition();
        verticalScroller->setDuration(250);
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
    horizontalScroller->setDuration(250);
    horizontalScroller->setStartValue(horizontalScrollBar()->sliderPosition());
    horizontalScroller->setEndValue(x);
    verticalScroller->setDuration(250);
    verticalScroller->setStartValue(verticalScrollBar()->sliderPosition());
    verticalScroller->setEndValue(y);
    groupScroller->start();
    emit backgroundChanges();
}

void Viewer::moveView(Qt::Key directionKey)
{
    QKeyEvent event(QEvent::KeyPress, directionKey, Qt::NoModifier);
    QAbstractScrollArea::keyPressEvent(&event);
    emit backgroundChanges();
}

static void animateScroll(QPropertyAnimation &scroller, const QScrollBar &scrollBar, int delta)
{
    int deltaNotFinished = 0;
    if (scroller.state() == QAbstractAnimation::Running) {
        deltaNotFinished = scroller.startValue().toInt() - scroller.endValue().toInt();
        scroller.stop();
    }

    const int currentPos = scrollBar.sliderPosition();
    scroller.setDuration(250);
    scroller.setStartValue(currentPos);
    scroller.setEndValue(currentPos - delta - deltaNotFinished);

    scroller.start();
}

void Viewer::wheelEvent(QWheelEvent *event)
{
    auto delta = event->angleDelta();

    if (render->hasLoadedComic()) {
        if (delta.x() != 0) {
            animateScroll(*horizontalScroller, *horizontalScrollBar(), delta.x());
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
}

void Viewer::resizeEvent(QResizeEvent *event)
{
    updateContentSize();
    goToFlow->updateSize();
    goToFlow->move((width() - goToFlow->width()) / 2, height() - goToFlow->height());
    informationLabel->updatePosition();
    QScrollArea::resizeEvent(event);
}

void Viewer::mouseMoveEvent(QMouseEvent *event)
{
    showCursor();
    hideCursorTimer->start(2500);

    if (magnifyingGlassShown)
        mglass->move(static_cast<int>(event->x() - float(mglass->width()) / 2), static_cast<int>(event->y() - float(mglass->height()) / 2));

    if (render->hasLoadedComic()) {
        if (showGoToFlowAnimation->state() != QPropertyAnimation::Running) {
            if (Configuration::getConfiguration().getDisableShowOnMouseOver() == false) {
                if (goToFlow->isVisible()) {
                    QPoint gtfPos = goToFlow->mapFrom(this, event->pos());
                    if (gtfPos.y() < 0 || gtfPos.x() < 0 || gtfPos.x() > goToFlow->width()) // TODO this extra check is for Mavericks (mouseMove over goToFlowGL seems to be broken)
                        animateHideGoToFlow();
                    // goToFlow->hide();
                } else {
                    int umbral = (width() - goToFlow->width()) / 2;
                    if ((event->y() > height() - 15) && (event->x() > umbral) && (event->x() < width() - umbral)) {

                        animateShowGoToFlow();
                        hideCursorTimer->stop();
                    }
                }
            }
        }

        if (drag) {
            int currentPosY = verticalScrollBar()->sliderPosition();
            int currentPosX = horizontalScrollBar()->sliderPosition();
            verticalScrollBar()->setSliderPosition(currentPosY + (yDragOrigin - event->y()));
            horizontalScrollBar()->setSliderPosition(currentPosX + (xDragOrigin - event->x()));
            yDragOrigin = event->y();
            xDragOrigin = event->x();
        }
    }
}

QPixmap Viewer::pixmap() const
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return content->pixmap();
#else
    return content->pixmap(Qt::ReturnByValue);
#endif
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
        informationLabel->setText(render->getCurrentPagesInformation() + " - " + QTime::currentTime().toString("HH:mm"));
        informationLabel->adjustSize();
        informationLabel->update(); // TODO it shouldn't be neccesary
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
    emit reset();
}

void Viewer::setLoadingMessage()
{
    if (magnifyingGlassShown) {
        hideMagnifyingGlass();
        restoreMagnifyingGlass = true;
    }
    emit pageAvailable(false);
    configureContent(tr("Loading...please wait!"));
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
    content->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    if (!(devicePixelRatioF() > 1))
        content->setScaledContents(true);
    content->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    content->setText(msg);
    content->setFont(QFont("courier new", 12));
    content->adjustSize();
    setFocus(Qt::ShortcutFocusReason);
    // emit showingText();
}

void Viewer::hideCursor()
{
#ifdef Q_OS_MAC
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
    updateBackgroundColor(Configuration::getConfiguration().getBackgroundColor());
    updateContentSize();
}

void Viewer::updateBackgroundColor(const QColor &color)
{
    QPalette palette;
    palette.setColor(backgroundRole(), color);
    setPalette(palette);
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
    if (event->button() == Qt::LeftButton) {
        drag = true;
        yDragOrigin = event->y();
        xDragOrigin = event->x();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
}

void Viewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        drag = false;
        setCursor(Qt::OpenHandCursor);
        event->accept();
        return;
    }

    if (event->button() == Qt::ForwardButton) {
        right();
        event->accept();
        return;
    }

    if (event->button() == Qt::BackButton) {
        left();
        event->accept();
        return;
    }
}

void Viewer::updateZoomRatio(int ratio)
{
    zoom = ratio;
    updateContentSize();
}

bool Viewer::getIsMangaMode()
{
    return doubleMangaPage;
}

void Viewer::updateConfig(QSettings *settings)
{
    goToFlow->updateConfig(settings);

    QPalette palette;
    palette.setColor(backgroundRole(), Configuration::getConfiguration().getBackgroundColor());
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
