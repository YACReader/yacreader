#include "viewer_gesture_handler.h"

#include <QtWidgets>
#include <QTouchEvent>

ViewerGestureHandler::ViewerGestureHandler(QObject *parent)
    : QObject { parent }, oneFingerSwiperGestureRecognizer(new OneFingerSwipeGestureRecognizer()), twoFingerSwiperGestureRecognizer(new TwoFingerSwipeGestureRecognizer())
{
}

bool ViewerGestureHandler::handleEvent(QEvent *event)
{
    // GESTURE HANDLING
    if (event->type() == QEvent::Gesture) {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent *>(event);
        // three fingers swipe, the default in Qt
        if (QGesture *swipe = gestureEvent->gesture(Qt::SwipeGesture)) {
            QSwipeGesture *swipeGesture = static_cast<QSwipeGesture *>(swipe);
            if (swipeGesture->state() == Qt::GestureFinished) {
                qDebug() << "Qt Native swipe";
                if (swipeGesture->horizontalDirection() == QSwipeGesture::Left) {
                    // emit swipeLeft();
                    return true;
                } else if (swipeGesture->horizontalDirection() == QSwipeGesture::Right) {
                    // emit swipeRight();
                    return true;
                }
            }
        }

        if (QGesture *tap = gestureEvent->gesture(Qt::TapGesture)) {
            QTapGesture *tapGesture = static_cast<QTapGesture *>(tap);
            if (tapGesture->state() == Qt::GestureFinished) {
                // emit tapGestureDetected();
                qDebug() << "TAP yay";
                return true;
            }
        }

        if (QGesture *oneFingersSwipe = gestureEvent->gesture(oneFingerSwiperGestureRecognizerType)) {
            SwipeGesture *oneFingersSwipeGesture = static_cast<SwipeGesture *>(oneFingersSwipe);

            if (oneFingersSwipeGesture->state() == Qt::GestureFinished) {
                qDebug() << "yay 1";
                return true;
            }
        }

        if (QGesture *twoFingersSwipe = gestureEvent->gesture(twoFingerSwiperGestureRecognizerType)) {
            SwipeGesture *twoFingersSwipeGesture = static_cast<SwipeGesture *>(twoFingersSwipe);

            if (twoFingersSwipeGesture->state() == Qt::GestureFinished) {
                qDebug() << "yay 2";
                return true;
            }
        }
    }

    // TOUCH HANDLING

    return false;
}

void ViewerGestureHandler::setupGestureHandler(Viewer *widget)
{
    // widget->setAttribute(Qt::WA_AcceptTouchEvents);
    widget->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);

    // widget->grabGesture(Qt::SwipeGesture);
    // widget->grabGesture(Qt::TapGesture);

    // oneFingerSwiperGestureRecognizerType = QGestureRecognizer::registerRecognizer(oneFingerSwiperGestureRecognizer);
    twoFingerSwiperGestureRecognizerType = QGestureRecognizer::registerRecognizer(twoFingerSwiperGestureRecognizer);

    // widget->grabGesture(oneFingerSwiperGestureRecognizerType);
    widget->viewport()->grabGesture(twoFingerSwiperGestureRecognizerType);
}

//--

SwipeGesture::SwipeGesture(QObject *parent)
    : QGesture(parent), swipeGesturePrivate(new SwipeGesturePrivate)
{
    swipeGesturePrivate->gestureType = Qt::SwipeGesture;
}

/*!
    Destructor.
*/
SwipeGesture::~SwipeGesture()
{
}

SwipeGesture::SwipeDirection SwipeGesture::horizontalDirection() const
{
    auto d = swipeGesturePrivate;
    if (d->swipeAngle < 0 || d->swipeAngle == 90 || d->swipeAngle == 270)
        return SwipeGesture::NoDirection;
    else if (d->swipeAngle < 90 || d->swipeAngle > 270)
        return SwipeGesture::Right;
    else
        return SwipeGesture::Left;
}

SwipeGesture::SwipeDirection SwipeGesture::verticalDirection() const
{
    auto d = swipeGesturePrivate;
    if (d->swipeAngle <= 0 || d->swipeAngle == 180)
        return SwipeGesture::NoDirection;
    else if (d->swipeAngle < 180)
        return SwipeGesture::Up;
    else
        return SwipeGesture::Down;
}

qreal SwipeGesture::swipeAngle() const
{
    return swipeGesturePrivate->swipeAngle;
}

void SwipeGesture::setSwipeAngle(qreal value)
{
    swipeGesturePrivate->swipeAngle = value;
}

OneFingerSwipeGestureRecognizer::OneFingerSwipeGestureRecognizer()
    : QGestureRecognizer()
{
}

QGesture *OneFingerSwipeGestureRecognizer::create(QObject *target)
{
    return new SwipeGesture(target);
}

QGestureRecognizer::Result OneFingerSwipeGestureRecognizer::recognize(QGesture *state,
                                                                      QObject *,
                                                                      QEvent *event)
{
    SwipeGesture *q = static_cast<SwipeGesture *>(state);
    SwipeGesturePrivate *d = q->swipeGesturePrivate;

    QGestureRecognizer::Result result = QGestureRecognizer::Ignore;

    switch (event->type()) {
    case QEvent::TouchBegin: {
        d->velocityValue = 1;
        d->time.start();
        d->state = SwipeGesturePrivate::Started;
        result = QGestureRecognizer::MayBeGesture;
        break;
    }
    case QEvent::TouchEnd: {
        if (q->state() != Qt::NoGesture) {
            result = QGestureRecognizer::FinishGesture;
        } else {
            result = QGestureRecognizer::CancelGesture;
        }
        break;
    }
    case QEvent::TouchUpdate: {
        const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
        if (d->state == SwipeGesturePrivate::NoGesture)
            result = QGestureRecognizer::CancelGesture;
        else if (ev->points().size() == 1) {
            d->state = SwipeGesturePrivate::PointsReached;
            const QEventPoint &p1 = ev->points().at(0);

            if (d->lastPositions[0].isNull()) {
                d->lastPositions[0] = p1.globalPressPosition().toPoint();
            }
            d->hotSpot = p1.globalPosition();
            d->isHotSpotSet = true;

            int xDistance = p1.globalPosition().x() - d->lastPositions[0].x();
            int yDistance = p1.globalPosition().y() - d->lastPositions[0].y();

            const int distance = xDistance >= yDistance ? xDistance : yDistance;
            int elapsedTime = d->time.restart();
            if (!elapsedTime)
                elapsedTime = 1;
            d->velocityValue = 0.9 * d->velocityValue + (qreal)distance / elapsedTime;
            d->swipeAngle = QLineF(p1.globalPressPosition(), p1.globalPosition()).angle();

            static const int MoveThreshold = 50;
            static const int directionChangeThreshold = MoveThreshold / 8;
            if (qAbs(xDistance) > MoveThreshold || qAbs(yDistance) > MoveThreshold) {
                // measure the distance to check if the direction changed
                d->lastPositions[0] = p1.globalPosition().toPoint();
                result = QGestureRecognizer::TriggerGesture;
                // QTBUG-46195, small changes in direction should not cause the gesture to be canceled.
                if (d->verticalDirection == SwipeGesture::NoDirection || qAbs(yDistance) > directionChangeThreshold) {
                    const SwipeGesture::SwipeDirection vertical = yDistance > 0
                            ? SwipeGesture::Down
                            : SwipeGesture::Up;
                    if (d->verticalDirection != SwipeGesture::NoDirection && d->verticalDirection != vertical)
                        result = QGestureRecognizer::CancelGesture;
                    d->verticalDirection = vertical;
                }
                if (d->horizontalDirection == SwipeGesture::NoDirection || qAbs(xDistance) > directionChangeThreshold) {
                    const SwipeGesture::SwipeDirection horizontal = xDistance > 0
                            ? SwipeGesture::Right
                            : SwipeGesture::Left;
                    if (d->horizontalDirection != SwipeGesture::NoDirection && d->horizontalDirection != horizontal)
                        result = QGestureRecognizer::CancelGesture;
                    d->horizontalDirection = horizontal;
                }
            } else {
                if (q->state() != Qt::NoGesture)
                    result = QGestureRecognizer::TriggerGesture;
                else
                    result = QGestureRecognizer::MayBeGesture;
            }
        } else if (ev->points().size() > 1) {
            result = QGestureRecognizer::CancelGesture;
        } else { // less than 1 touch points, so it wont happen?
            switch (d->state) {
            case SwipeGesturePrivate::NoGesture:
                result = QGestureRecognizer::MayBeGesture;
                break;
            case SwipeGesturePrivate::Started:
                result = QGestureRecognizer::Ignore;
                break;
            case SwipeGesturePrivate::PointsReached:
                result = (ev->touchPointStates() & QEventPoint::State::Pressed)
                        ? QGestureRecognizer::CancelGesture
                        : QGestureRecognizer::Ignore;
                break;
            }
        }
        break;
    }
    default:
        break;
    }
    return result;
}

void OneFingerSwipeGestureRecognizer::reset(QGesture *state)
{
    SwipeGesture *q = static_cast<SwipeGesture *>(state);
    SwipeGesturePrivate *d = q->swipeGesturePrivate;

    d->verticalDirection = d->horizontalDirection = SwipeGesture::NoDirection;
    d->swipeAngle = 0;

    d->lastPositions[0] = QPoint();
    d->state = SwipeGesturePrivate::NoGesture;
    d->velocityValue = 0;
    d->time.invalidate();

    QGestureRecognizer::reset(state);
}

TwoFingerSwipeGestureRecognizer::TwoFingerSwipeGestureRecognizer()
    : QGestureRecognizer()
{
}

QGesture *TwoFingerSwipeGestureRecognizer::create(QObject *target)
{
    return new SwipeGesture(target);
}

QGestureRecognizer::Result TwoFingerSwipeGestureRecognizer::recognize(QGesture *state,
                                                                      QObject *,
                                                                      QEvent *event)
{
    // qDebug() << "TWO" << state;
    SwipeGesture *q = static_cast<SwipeGesture *>(state);
    SwipeGesturePrivate *d = q->swipeGesturePrivate;

    QGestureRecognizer::Result result = QGestureRecognizer::Ignore;

    switch (event->type()) {
    case QEvent::TouchBegin: {
        qDebug() << "TOUCH BEGIN";
    }

    case QEvent::TouchEnd: {
        qDebug() << "TOUCH END";
    }

    case QEvent::TouchUpdate: {
        qDebug() << "TOUCH UPDATE";
    }

    default: {
        // qDebug() << event->type();
        break;
    }
    }

    return result;

    switch (event->type()) {
    case QEvent::TouchBegin: {
        qDebug() << "TOUCH BEGIN";
        d->velocityValue = 1;
        d->time.start();
        d->state = SwipeGesturePrivate::Started;
        result = QGestureRecognizer::MayBeGesture;
        break;
    }
    case QEvent::TouchEnd: {
        qDebug() << "TOUCH END";

        if (d->state != SwipeGesturePrivate::NoGesture) {
            result = QGestureRecognizer::FinishGesture;
        } else {
            result = QGestureRecognizer::CancelGesture;
        }

        break;
    }
    case QEvent::TouchUpdate: {
        qDebug() << "TOUCH UPDATE";
        const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
        if (d->state == SwipeGesturePrivate::NoGesture) {
            qDebug() << "NoGesture";

            d->velocityValue = 1;
            d->time.start();
            d->state = SwipeGesturePrivate::Started;

            result = QGestureRecognizer::CancelGesture;
        } else if (ev->points().size() == 2) {
            qDebug() << "HMMMMMM";
            d->state = SwipeGesturePrivate::PointsReached;
            const QEventPoint &p1 = ev->points().at(0);
            const QEventPoint &p2 = ev->points().at(1);

            if (d->lastPositions[0].isNull()) {
                d->lastPositions[0] = p1.globalPressPosition().toPoint();
                d->lastPositions[1] = p2.globalPressPosition().toPoint();
            }
            d->hotSpot = p1.globalPosition();
            d->isHotSpotSet = true;

            int xDistance = (p1.globalPosition().x() - d->lastPositions[0].x() +
                             p2.globalPosition().x() - d->lastPositions[1].x()) /
                    2;
            int yDistance = (p1.globalPosition().y() - d->lastPositions[0].y() +
                             p2.globalPosition().y() - d->lastPositions[1].y()) /
                    2;

            const int distance = xDistance >= yDistance ? xDistance : yDistance;
            int elapsedTime = d->time.restart();
            if (!elapsedTime)
                elapsedTime = 1;
            d->velocityValue = 0.9 * d->velocityValue + (qreal)distance / elapsedTime;
            d->swipeAngle = QLineF(p1.globalPressPosition(), p1.globalPosition()).angle();

            static const int MoveThreshold = 50;
            static const int directionChangeThreshold = MoveThreshold / 8;
            if (qAbs(xDistance) > MoveThreshold || qAbs(yDistance) > MoveThreshold) {
                qDebug() << "distance " << qAbs(xDistance) << " - " << qAbs(yDistance);
                // measure the distance to check if the direction changed
                d->lastPositions[0] = p1.globalPosition().toPoint();
                d->lastPositions[1] = p2.globalPosition().toPoint();
                result = QGestureRecognizer::TriggerGesture;
                // QTBUG-46195, small changes in direction should not cause the gesture to be canceled.
                if (d->verticalDirection == SwipeGesture::NoDirection || qAbs(yDistance) > directionChangeThreshold) {
                    const SwipeGesture::SwipeDirection vertical = yDistance > 0
                            ? SwipeGesture::Down
                            : SwipeGesture::Up;
                    if (d->verticalDirection != SwipeGesture::NoDirection && d->verticalDirection != vertical)
                        result = QGestureRecognizer::CancelGesture;
                    d->verticalDirection = vertical;
                }
                if (d->horizontalDirection == SwipeGesture::NoDirection || qAbs(xDistance) > directionChangeThreshold) {
                    const SwipeGesture::SwipeDirection horizontal = xDistance > 0
                            ? SwipeGesture::Right
                            : SwipeGesture::Left;
                    if (d->horizontalDirection != SwipeGesture::NoDirection && d->horizontalDirection != horizontal)
                        result = QGestureRecognizer::CancelGesture;
                    d->horizontalDirection = horizontal;
                }

                result = QGestureRecognizer::MayBeGesture;
            } else {
                qDebug() << "not enough distance " << qAbs(xDistance) << " - " << qAbs(yDistance);
                if (d->state != SwipeGesturePrivate::NoGesture)
                    result = QGestureRecognizer::TriggerGesture;
                else
                    result = QGestureRecognizer::MayBeGesture;
            }
        } else if (ev->points().size() > 2) {
            qDebug() << "Cancel gesture";
            result = QGestureRecognizer::CancelGesture;
        } else { // less than 2 touch points
            qDebug() << "less than 2";
            switch (d->state) {
            case SwipeGesturePrivate::NoGesture:
                result = QGestureRecognizer::MayBeGesture;
                break;
            case SwipeGesturePrivate::Started:
                result = QGestureRecognizer::Ignore;
                break;
            case SwipeGesturePrivate::PointsReached:
                result = (ev->touchPointStates() & QEventPoint::State::Pressed)
                        ? QGestureRecognizer::CancelGesture
                        : QGestureRecognizer::Ignore;
                break;
            }
        }
        break;
    }
    default:
        break;
    }
    return result;
}

void TwoFingerSwipeGestureRecognizer::reset(QGesture *state)
{
    qDebug() << "Reset";
    SwipeGesture *q = static_cast<SwipeGesture *>(state);
    SwipeGesturePrivate *d = q->swipeGesturePrivate;

    d->verticalDirection = d->horizontalDirection = SwipeGesture::NoDirection;
    d->swipeAngle = 0;

    d->lastPositions[0] = d->lastPositions[1] = QPoint();
    d->state = SwipeGesturePrivate::NoGesture;
    d->velocityValue = 0;
    d->time.invalidate();

    QGestureRecognizer::reset(state);
}
