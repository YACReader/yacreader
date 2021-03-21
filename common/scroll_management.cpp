#include "scroll_management.h"

ScrollManagement::ScrollManagement()
{
    wheelTimer.start();
    wheelAccumulator = 0;
}

ScrollManagement::Movement ScrollManagement::getMovement(QWheelEvent *event)
{
    /*QLOG_DEBUG() << "WheelEvent angle delta : " << event->angleDelta();
    QLOG_DEBUG() << "WheelEvent pixel delta : " << event->pixelDelta();*/

    int tooFast = 1;
    int timeThrottle = 16;
    int minimumMove = 70;

    const auto elapsedMs = wheelTimer.elapsed();

    //avoid any events overflood
    if (elapsedMs < tooFast) {
        event->setAccepted(true);
        return None;
    }

    // Accumulate the delta
    if ((event->delta() < 0) != (wheelAccumulator < 0)) //different sign means change in direction
        wheelAccumulator = 0;

    wheelAccumulator += event->delta();

    //Do not process events too fast
    if (elapsedMs < timeThrottle) {
        event->setAccepted(true);
        return None;
    }

    //small intervals are ignored until with have enough acumulated delta
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
    //Clean up
    wheelAccumulator = 0;
    wheelTimer.start();

    return m;
}

ScrollManagement::~ScrollManagement()
{
}
