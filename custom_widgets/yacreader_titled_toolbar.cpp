#include "yacreader_titled_toolbar.h"

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

    if (!theme.isMacosNative) {
        drawTextEffect(&painter, QPoint(contentsMargins().left(), 1));
    }
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
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    QString styleSheet = "QWidget {border:0px;}";
    setStyleSheet(styleSheet);

    nameLabel = new DropShadowLabel(this);
    nameLabel->setText(title);

    QString nameLabelStyleSheet = "QLabel {padding:0 0 0 10px; margin:0px; font-size:11px; font-weight:bold;}";
    nameLabel->setColor(QColor(theme.titledToolBarTitleColor));
    nameLabel->setDropShadowColor(QColor(theme.titledToolBarTitleShadowColor));

    nameLabel->setStyleSheet(nameLabelStyleSheet);

    mainLayout->addWidget(nameLabel, Qt::AlignLeft);
    mainLayout->addStretch();

    setLayout(mainLayout);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    setMinimumHeight(25);
}

void YACReaderTitledToolBar::addAction(QAction *action)
{
   auto mainLayout = dynamic_cast<QHBoxLayout *>(layout());

    auto tb = new QToolButton(this);
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
    auto mainLayout = dynamic_cast<QHBoxLayout *>(layout());

    auto w = new QWidget(this);
    w->setFixedSize(1, 14);

    w->setStyleSheet(QString("QWidget {background-color:%1;}").arg(theme.titledToolBarSeparatorColor));

    mainLayout->addSpacing(10);
    mainLayout->addWidget(w);
    mainLayout->addSpacing(10);
}
