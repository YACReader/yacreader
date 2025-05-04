#include "mouse_handler.h"

#include <QtWidgets>

#include "configuration.h"
#include "magnifying_glass.h"
#include "render.h"
#include "viewer.h"

#include "goto_flow.h"
#ifndef NO_OPENGL
#include "goto_flow_gl.h"
#else
#include <QtWidgets>
#endif

using namespace YACReader;

YACReader::MouseHandler::MouseHandler(Viewer *viewer)
    : viewer(viewer)
{
}

void YACReader::MouseHandler::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        viewer->drag = true;
        dragOrigin = dragLatestPosition = event->position();
        viewer->setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
}

void YACReader::MouseHandler::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        viewer->drag = false;
        viewer->setCursor(Qt::OpenHandCursor);
        event->accept();
        return;
    }

    if (event->button() == Qt::ForwardButton) {
        viewer->right();
        event->accept();
        return;
    }

    if (event->button() == Qt::BackButton) {
        viewer->left();
        event->accept();
        return;
    }
}

void YACReader::MouseHandler::mouseMoveEvent(QMouseEvent *event)
{
    viewer->showCursor();
    viewer->hideCursorTimer->start(2500);

    auto position = event->position();

    if (viewer->magnifyingGlassShown)
        viewer->mglass->move(static_cast<int>(position.x() - float(viewer->mglass->width()) / 2), static_cast<int>(position.y() - float(viewer->mglass->height()) / 2));

    if (viewer->render->hasLoadedComic()) {
        if (viewer->showGoToFlowAnimation->state() != QPropertyAnimation::Running) {
            if (Configuration::getConfiguration().getDisableShowOnMouseOver() == false) {
                if (viewer->goToFlow->isVisible()) {
                    QPoint gtfPos = viewer->goToFlow->mapFrom(this->viewer, event->pos());
                    if (gtfPos.y() < 0 || gtfPos.x() < 0 || gtfPos.x() > viewer->goToFlow->width()) // TODO this extra check is for Mavericks (mouseMove over goToFlowGL seems to be broken)
                        viewer->animateHideGoToFlow();
                    // goToFlow->hide();
                } else {
                    int umbral = (viewer->width() - viewer->goToFlow->width()) / 2;
                    if ((position.y() > viewer->height() - 15) && (position.x() > umbral) && (position.x() < viewer->width() - umbral)) {

                        viewer->animateShowGoToFlow();
                        viewer->hideCursorTimer->stop();
                    }
                }
            }
        }

        if (viewer->drag) {
            int currentPosY = viewer->verticalScrollBar()->sliderPosition();
            int currentPosX = viewer->horizontalScrollBar()->sliderPosition();
            viewer->verticalScrollBar()->setSliderPosition(currentPosY + (dragLatestPosition.y() - position.y()));
            viewer->horizontalScrollBar()->setSliderPosition(currentPosX + (dragLatestPosition.x() - position.x()));
            dragLatestPosition = position;
        }
    }
}
