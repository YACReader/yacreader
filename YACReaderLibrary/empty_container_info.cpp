#include "empty_container_info.h"

#include "yacreader_global.h"

EmptyContainerInfo::EmptyContainerInfo(QWidget *parent)
    : QWidget(parent), iconLabel(new QLabel()), titleLabel(new QLabel())
{
    iconLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setAlignment(Qt::AlignCenter);

    initTheme(this);
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
    painter.fillRect(0, 0, width(), height(), theme.emptyContainer.backgroundColor);
}

void EmptyContainerInfo::applyTheme(const Theme &theme)
{
    titleLabel->setStyleSheet(theme.emptyContainer.titleLabelQSS);
    update(); // Trigger repaint for background color
}
