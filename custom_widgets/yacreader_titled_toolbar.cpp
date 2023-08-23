#include "yacreader_titled_toolbar.h"

#include "yacreader_global.h"
#include "yacreader_busy_widget.h"

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>

DropShadowLabel::DropShadowLabel(QWidget *parent)
    : QLabel(parent)
{
}

void DropShadowLabel::drawText(QPainter *painter,
                               QPoint offset)
{
    Q_ASSERT(painter != 0);

    // Draw shadow.
    painter->setPen(QPen(textColor));
    painter->drawText(rect().translated(offset),
                      alignment(), text());
}
void DropShadowLabel::drawTextEffect(QPainter *painter,
                                     QPoint offset)
{
    Q_ASSERT(painter != 0);

    // Draw shadow.
    painter->setPen(QPen(dropShadowColor));
    painter->drawText(rect().translated(offset),
                      alignment(), text());
}

void DropShadowLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setFont(font());
#ifndef Y_MAC_UI
    drawTextEffect(&painter, QPoint(contentsMargins().left(), 1));
#endif
    drawText(&painter, QPoint(contentsMargins().left(), 0));
}

void DropShadowLabel::setColor(const QColor &color)
{
    textColor = color;
}

void DropShadowLabel::setDropShadowColor(const QColor &color)
{
    dropShadowColor = color;
}

YACReaderTitledToolBar::YACReaderTitledToolBar(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QString styleSheet = "QWidget {border:0px;}";
    setStyleSheet(styleSheet);

    nameLabel = new DropShadowLabel(this);
    busyIndicator = new BusyIndicator(this, 12);
    connect(busyIndicator, &BusyIndicator::clicked, this, &YACReaderTitledToolBar::cancelOperationRequested);
    busyIndicator->setIndicatorStyle(BusyIndicator::StyleArc);
#ifdef Y_MAC_UI
    busyIndicator->setColor(QColor("#808080"));
#else
    busyIndicator->setColor(Qt::white);
#endif
    busyIndicator->setHidden(true);

    nameLabel->setText(title);
#ifdef Y_MAC_UI
    QString nameLabelStyleSheet = "QLabel {padding:0 0 0 10px; margin:0px; font-size:11px; font-weight:bold;}";
    nameLabel->setColor(QColor("#808080"));
    // nameLabel->setDropShadowColor(QColor("#F9FAFB"));
#else
    QString nameLabelStyleSheet = "QLabel {padding:0 0 0 10px; margin:0px; font-size:11px; font-weight:bold;}";
    nameLabel->setColor(QColor("#BDBFBF"));
    nameLabel->setDropShadowColor(QColor("#000000"));
#endif
    nameLabel->setStyleSheet(nameLabelStyleSheet);

    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(busyIndicator);
    mainLayout->addStretch();

    setLayout(mainLayout);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    setMinimumHeight(25);
}

void YACReaderTitledToolBar::addAction(QAction *action)
{
    QHBoxLayout *mainLayout = dynamic_cast<QHBoxLayout *>(layout());

// fix for QToolButton and retina support in OSX
#ifdef Q_OS_MACOS // TODO_Y_MAC_UI
    QPushButton *pb = new QPushButton(this);
    pb->setCursor(QCursor(Qt::ArrowCursor));
    pb->setIcon(action->icon());
    pb->addAction(action);

    connect(pb, &QPushButton::clicked, action, &QAction::triggered);

    mainLayout->addWidget(pb);
#else
    QToolButton *tb = new QToolButton(this);
    tb->setCursor(QCursor(Qt::ArrowCursor));
    tb->setDefaultAction(action);
    tb->setIconSize(QSize(16, 16));
    tb->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    // tb->setStyleSheet("QToolButton:hover {background-color:#C5C5C5;}");

    mainLayout->addWidget(tb);
#endif
}

void YACReaderTitledToolBar::addSpacing(int spacing)
{
    QHBoxLayout *mainLayout = dynamic_cast<QHBoxLayout *>(layout());

    mainLayout->addSpacing(spacing);
}

void YACReaderTitledToolBar::addSepartor()
{
    QHBoxLayout *mainLayout = dynamic_cast<QHBoxLayout *>(layout());

    QWidget *w = new QWidget(this);
    w->setFixedSize(1, 14);
#ifdef Y_MAC_UI
    w->setStyleSheet("QWidget {background-color:#AFAFAF;}");
#else
    w->setStyleSheet("QWidget {background-color:#6F6F6F;}");
#endif

    mainLayout->addSpacing(10);
    mainLayout->addWidget(w);
    mainLayout->addSpacing(10);
}

void YACReaderTitledToolBar::showBusyIndicator()
{
    busyIndicator->setHidden(false);
}

void YACReaderTitledToolBar::hideBusyIndicator()
{
    busyIndicator->setHidden(true);
}
