#include "comics_view_transition.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
#include <QSettings>
#include <QTimer>
#include <QSizePolicy>
#include <QPainter>

#include "yacreader_global_gui.h"

ComicsViewTransition::ComicsViewTransition(QWidget *parent)
    : QWidget(parent)
{
#ifdef Q_OS_MAC
    setStyleSheet("QWidget {background:#FFFFFF}");
#else
    setStyleSheet("QWidget {background:#2A2A2A}");
#endif
}

QSize ComicsViewTransition::sizeHint()
{
    return QSize(450, 350);
}

void ComicsViewTransition::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

#ifdef Q_OS_MAC
    painter.fillRect(0, 0, width(), height(), QColor("#FFFFFF"));
#else
    painter.fillRect(0, 0, width(), height(), QColor("#2A2A2A"));
#endif
}
