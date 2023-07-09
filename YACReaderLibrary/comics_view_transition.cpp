#include "comics_view_transition.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
#include <QSettings>
#include <QTimer>
#include <QSizePolicy>
#include <QPainter>

#include "yacreader_global.h"

ComicsViewTransition::ComicsViewTransition(QWidget *parent)
    : QWidget(parent)
{
#ifdef Y_MAC_UI
    setStyleSheet("QWidget {background:#FFFFFF}");
#else
    setStyleSheet("QWidget {background:#2A2A2A}");
#endif
}

void ComicsViewTransition::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

#ifdef Y_MAC_UI
    painter.fillRect(0, 0, width(), height(), QColor("#FFFFFF"));
#else
    painter.fillRect(0, 0, width(), height(), QColor("#2A2A2A"));
#endif
}
