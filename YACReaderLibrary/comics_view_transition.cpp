#include "comics_view_transition.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
#include <QSettings>
#include <QTimer>
#include <QSizePolicy>
#include <QPainter>

#include "yacreader_global_gui.h"

ComicsViewTransition::ComicsViewTransition(const QString &backgroundColor, QWidget *parent)
    : QWidget(parent), backgroundColor(backgroundColor), painter(this)
{
    setStyleSheet(QString("QWidget {background:%1").arg(backgroundColor));
}

QSize ComicsViewTransition::sizeHint()
{
    return QSize(450, 350);
}

void ComicsViewTransition::paintEvent(QPaintEvent *)
{
    painter.fillRect(0, 0, width(), height(), backgroundColor);
}
