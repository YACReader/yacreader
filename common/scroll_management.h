#ifndef SCROLLMANAGAMENT_H
#define SCROLLMANAGAMENT_H

#include <QElapsedTimer>
#include <QWheelEvent>

class ScrollManagement
{
public:
    enum Movement {
        None,
        Forward,
        Backward
    };

    ScrollManagement();
    ScrollManagement::Movement getMovement(QWheelEvent *event,
                                           Qt::Orientations orientations = Qt::Horizontal | Qt::Vertical);
    ~ScrollManagement();

private:
    QElapsedTimer wheelTimer;
    int wheelAccumulator;
};

#endif // SCROLLMANAGAMENT_H
