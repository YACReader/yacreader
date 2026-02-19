#include "comics_view_transition.h"

#include <QPainter>

ComicsViewTransition::ComicsViewTransition(QWidget *parent)
    : QWidget(parent)
{
    initTheme(this);
}

void ComicsViewTransition::applyTheme(const Theme &theme)
{
    setStyleSheet(QString("QWidget {background:%1}").arg(theme.defaultContentBackgroundColor.name()));
    update();
}

void ComicsViewTransition::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), theme.defaultContentBackgroundColor);
}
