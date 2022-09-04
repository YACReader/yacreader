#include "scroll_management.h"
#include <QtCore>
#include <QApplication>

ScrollManagement::ScrollManagement()
{
    wheelTimer = new QElapsedTimer();
    wheelTimer->start();
    wheelAccumulator = 0;
}

ScrollManagement::Movement ScrollManagement::getMovement(QWheelEvent *event)
{
    //    qDebug() << "WheelEvent angle delta : " << event->angleDelta();
    //    qDebug() << "WheelEvent pixel delta : " << event->pixelDelta();
    //    qDebug() << "Accumulator : " << wheelAccumulator;

    int delta;
    int tooFast = 1;
    int timeThrottle = 16;
    int minimumMove;

    if (event->pixelDelta().x() != 0 || event->pixelDelta().y() != 0) {
        delta = event->pixelDelta().y() + event->pixelDelta().x();
        minimumMove = 30;
    } else {
        delta = (event->angleDelta().y() / 8) + (event->angleDelta().x() / 8);
        minimumMove = 8;
    }

    wheelAccumulator += delta;

    // avoid any events overflood
    if ((wheelTimer->elapsed() < tooFast)) {
        event->setAccepted(true);
        return None;
    }

    // Do not process events too fast
    if ((wheelTimer->elapsed() < timeThrottle)) {
        event->setAccepted(true);
        return None;
    }

    // small intervals are ignored until with have enough acumulated delta
    if ((wheelAccumulator < minimumMove) && (wheelAccumulator > -minimumMove)) {
        event->setAccepted(true);
        return None;
    }

    Movement m;
    if (wheelAccumulator < 0)
        m = Forward;
    else
        m = Backward;

    event->accept();
    // Clean up
    wheelAccumulator = 0;
    wheelTimer->restart();

    return m;
}

ScrollManagement::~ScrollManagement()
{
}
