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
    drawTextEffect(&painter, QPoint(contentsMargins().left(), 1));
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
    busyIndicator->setHidden(true);

    nameLabel->setText(title);
    QString nameLabelStyleSheet = "QLabel {padding:0 0 0 10px; margin:0px; font-size:11px; font-weight:bold;}";
    nameLabel->setStyleSheet(nameLabelStyleSheet);

    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(busyIndicator);
    mainLayout->addStretch();

    setLayout(mainLayout);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    setMinimumHeight(25);

    initTheme(this);
}

void YACReaderTitledToolBar::addAction(QAction *action)
{
    QHBoxLayout *mainLayout = dynamic_cast<QHBoxLayout *>(layout());

    QToolButton *tb = new QToolButton(this);
    tb->setCursor(QCursor(Qt::ArrowCursor));
    tb->setDefaultAction(action);
    tb->setIconSize(QSize(16, 16));
    tb->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    mainLayout->addWidget(tb);
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
    w->setStyleSheet(QString("QWidget {background-color:%1;}").arg(theme.sidebar.separatorColor.name()));
    separators.append(w);

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

void YACReaderTitledToolBar::setTitle(const QString &title)
{
    nameLabel->setText(title);
}

void YACReaderTitledToolBar::applyTheme(const Theme &theme)
{
    auto sidebarTheme = theme.sidebar;

    nameLabel->setColor(sidebarTheme.titleTextColor);
    nameLabel->setDropShadowColor(sidebarTheme.titleDropShadowColor);
    nameLabel->update();

    busyIndicator->setColor(sidebarTheme.busyIndicatorColor);

    QString qss = QString("QWidget {background-color:%1;}").arg(sidebarTheme.separatorColor.name());
    for (auto separator : separators) {
        separator->setStyleSheet(qss);
    }
}
