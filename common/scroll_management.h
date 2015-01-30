#ifndef SCROLLMANAGAMENT_H
#define SCROLLMANAGAMENT_H

#include <QTime>
#include <QWheelEvent>

class ScrollManagement
{
public:
    enum Movement{
        None,
        Forward,
        Backward
    };

    ScrollManagement();
    ScrollManagement::Movement getMovement(QWheelEvent * event);
    ~ScrollManagement();

private:
    QTime * wheelTimer;
    int wheelAccumulator;
};

#endif // SCROLLMANAGAMENT_H
