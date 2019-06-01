#ifndef COMICS_VIEW_TRANSITION_H
#define COMICS_VIEW_TRANSITION_H

#include <QtWidgets>

class ComicsViewTransition : public QWidget
{
    Q_OBJECT
public:
    explicit ComicsViewTransition(const QString &backgroundColor, QWidget *parent = nullptr);
    QSize sizeHint();

protected:
    void paintEvent(QPaintEvent *);
    QColor backgroundColor;
    QPainter painter;
};

#endif // COMICS_VIEW_TRANSITION_H
