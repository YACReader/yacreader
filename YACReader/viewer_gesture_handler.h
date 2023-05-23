#ifndef VIEWERGESTUREHANDLER_H
#define VIEWERGESTUREHANDLER_H

#include <QtWidgets>
#include <QtGui>

#include "viewer.h"

class SwipeGesturePrivate;
class SwipeGesture : public QGesture
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(SwipeGesture)

    Q_PROPERTY(SwipeDirection horizontalDirection READ horizontalDirection STORED false)
    Q_PROPERTY(SwipeDirection verticalDirection READ verticalDirection STORED false)
    Q_PROPERTY(qreal swipeAngle READ swipeAngle WRITE setSwipeAngle)
    Q_PRIVATE_PROPERTY(SwipeGesture::d_func(), qreal velocity READ velocity WRITE setVelocity)

public:
    SwipeGesturePrivate *swipeGesturePrivate;

    enum SwipeDirection { NoDirection,
                          Left,
                          Right,
                          Up,
                          Down };
    Q_ENUM(SwipeDirection)

    explicit SwipeGesture(QObject *parent = nullptr);
    ~SwipeGesture();

    SwipeDirection horizontalDirection() const;
    SwipeDirection verticalDirection() const;

    qreal swipeAngle() const;
    void setSwipeAngle(qreal value);

    friend class TwoFingerSwipeGestureRecognizer;
    friend class OneFingerSwipeGestureRecognizer;
};

class GesturePrivate : public QObject
{
public:
    GesturePrivate()
        : gestureType(Qt::CustomGesture), state(Qt::NoGesture), isHotSpotSet(false), gestureCancelPolicy(0)
    {
    }

    Qt::GestureType gestureType;
    Qt::GestureState state;
    QPointF hotSpot;
    QPointF sceneHotSpot;
    uint isHotSpotSet : 1;
    uint gestureCancelPolicy : 2;
};

class SwipeGesturePrivate : public GesturePrivate
{
public:
    enum State {
        NoGesture,
        Started,
        PointsReached
    };

    SwipeGesturePrivate()
        : horizontalDirection(SwipeGesture::NoDirection),
          verticalDirection(SwipeGesture::NoDirection),
          swipeAngle(0),
          state(NoGesture),
          velocityValue(0)
    {
    }

    qreal velocity() const { return velocityValue; }
    void setVelocity(qreal value) { velocityValue = value; }

    SwipeGesture::SwipeDirection horizontalDirection;
    SwipeGesture::SwipeDirection verticalDirection;
    qreal swipeAngle;

    QPoint lastPositions[3];
    State state;
    qreal velocityValue;
    QElapsedTimer time;
};

class OneFingerSwipeGestureRecognizer : public QGestureRecognizer
{
public:
    OneFingerSwipeGestureRecognizer();
    QGesture *create(QObject *target) override;
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event) override;
    void reset(QGesture *state) override;
};

class TwoFingerSwipeGestureRecognizer : public QGestureRecognizer
{
public:
    TwoFingerSwipeGestureRecognizer();
    QGesture *create(QObject *target) override;
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event) override;
    void reset(QGesture *state) override;
};

class ViewerGestureHandler : public QObject
{
    Q_OBJECT
public:
    explicit ViewerGestureHandler(QObject *parent = nullptr);
    void setupGestureHandler(Viewer *widget);
    bool handleEvent(QEvent *event);
signals:

private:
    OneFingerSwipeGestureRecognizer *oneFingerSwiperGestureRecognizer;
    Qt::GestureType oneFingerSwiperGestureRecognizerType;
    TwoFingerSwipeGestureRecognizer *twoFingerSwiperGestureRecognizer;
    Qt::GestureType twoFingerSwiperGestureRecognizerType;
};

#endif // VIEWERGESTUREHANDLER_H
