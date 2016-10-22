#ifndef COMICS_VIEW_TRANSITION_H
#define COMICS_VIEW_TRANSITION_H

#include <QWidget>

class ComicsViewTransition : public QWidget
{
    Q_OBJECT
public:
    explicit ComicsViewTransition(QWidget *parent = 0);
    QSize sizeHint();

protected:
    void paintEvent(QPaintEvent *);
};

#endif // COMICS_VIEW_TRANSITION_H
