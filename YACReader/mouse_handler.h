#ifndef MOUSE_HANDLER_H
#define MOUSE_HANDLER_H

#include <QMouseEvent>

class Viewer;

namespace YACReader {
class MouseHandler
{
public:
    MouseHandler(Viewer *viewer);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    Viewer *viewer;
};
}

#endif // MOUSE_HANDLER_H
