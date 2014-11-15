#ifndef COMICS_VIEW_TRANSITION_H
#define COMICS_VIEW_TRANSITION_H

#include <QWidget>

class QMovie;
class QSettings;
class QLabel;

class ComicsViewTransition : public QWidget
{
    Q_OBJECT
public:
    explicit ComicsViewTransition(QWidget *parent = 0);
    QSize sizeHint();

signals:
    void transitionFinished();

public slots:
    void startMovie();

protected:
    QMovie * movie;
    QSettings * settings;
    QLabel * movieLabel;

    void paintEvent(QPaintEvent *);
};

#endif // COMICS_VIEW_TRANSITION_H
