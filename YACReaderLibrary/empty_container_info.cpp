#include "empty_container_info.h"

EmptyContainerInfo::EmptyContainerInfo(QWidget *parent)
    : QWidget(parent), iconLabel(new QLabel()), titleLabel(new QLabel())
{
#ifdef Q_OS_MAC
    backgroundColor = "#FFFFFF";
    titleLabel->setStyleSheet("QLabel {color:#888888; font-size:24px;font-family:Arial;font-weight:bold;}");
#else
    backgroundColor = "#2A2A2A";
    titleLabel->setStyleSheet("QLabel {color:#CCCCCC; font-size:24px;font-family:Arial;font-weight:bold;}");
#endif

    iconLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setAlignment(Qt::AlignCenter);
}

void EmptyContainerInfo::setPixmap(const QPixmap &pixmap)
{
    iconLabel->setPixmap(pixmap);
}

void EmptyContainerInfo::setText(const QString &text)
{
    titleLabel->setText(text);
}

QVBoxLayout *EmptyContainerInfo::setUpDefaultLayout(bool addStretch)
{
    auto layout = new QVBoxLayout;

    layout->addSpacing(100);
    layout->addWidget(iconLabel);
    layout->addSpacing(30);
    layout->addWidget(titleLabel);
    if (addStretch)
        layout->addStretch();

    setLayout(layout);
    return layout;
}

void EmptyContainerInfo::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), QColor(backgroundColor));
}
