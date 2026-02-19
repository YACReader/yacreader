#ifndef COMICS_VIEW_TRANSITION_H
#define COMICS_VIEW_TRANSITION_H

#include <QWidget>

#include "themable.h"

class ComicsViewTransition : public QWidget, protected Themable
{
    Q_OBJECT
public:
    explicit ComicsViewTransition(QWidget *parent = nullptr);

protected:
    void applyTheme(const Theme &theme) override;
    void paintEvent(QPaintEvent *) override;
};

#endif // COMICS_VIEW_TRANSITION_H
