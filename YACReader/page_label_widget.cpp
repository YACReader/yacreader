#include "page_label_widget.h"

#include <QtWidgets>

PageLabelWidget::PageLabelWidget(QWidget *parent)
    : QWidget(parent)
{
    animation = new QPropertyAnimation(this, "pos");
    animation->setDuration(150);
    animation->setEndValue(QPoint((parent->geometry().size().width() - this->width()), -this->height()));

    QScreen *screen = parent != nullptr ? parent->window()->screen() : nullptr;
    if (screen == nullptr) {
        screen = QApplication::screens().constFirst();
    }

    int verticalRes = screen != nullptr ? screen->size().height() : 600;

    auto layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    textLabel = new QLabel(this);
    textLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    textLabel->setWordWrap(true); // Allow wrapping

    int contentMargin = 0;
    if (verticalRes <= 1024) {
        textLabel->setStyleSheet("QLabel { color : white; font-size:12px; }");
        contentMargin = 12;
    } else if (verticalRes <= 1200) {
        textLabel->setStyleSheet("QLabel { color : white; font-size:16px; }");
        contentMargin = 16;
    } else {
        textLabel->setStyleSheet("QLabel { color : white; font-size:20px; }");
        contentMargin = 20;
    }

    setContentsMargins(contentMargin * 2.3, contentMargin / 2.3, contentMargin * 2.3, contentMargin / 2.3);

    // Instead of fixed size, allow dynamic sizing
    textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    layout->addWidget(textLabel, 0, Qt::AlignCenter);
    setLayout(layout);

    adjustSize(); // Resize to fit content

    if (parent != nullptr)
        move(QPoint((parent->geometry().size().width() - this->width()), -this->height()));
}

void PageLabelWidget::show()
{
    if (this->pos().y() <= 0 && animation->state() != QPropertyAnimation::Running) {
        auto parent = dynamic_cast<QWidget *>(this->parent());
        if (parent == nullptr) {
            return;
        }

        QWidget::show();
        // connect(animation,SIGNAL(finished()),this,SLOT(QWidget::hide()));
        animation->disconnect();

        animation->setStartValue(QPoint((parent->geometry().size().width() - this->width()), -this->height()));
        animation->setEndValue(QPoint((parent->geometry().size().width() - this->width()), 0));
        animation->start();
    }
}

void PageLabelWidget::hide()
{
    if (this->pos().y() >= 0 && animation->state() != QPropertyAnimation::Running) {
        auto parent = dynamic_cast<QWidget *>(this->parent());
        if (parent == nullptr) {
            return;
        }
        // connect(animation,SIGNAL(finished()),this,SLOT(setHidden()));
        animation->setStartValue(QPoint((parent->geometry().size().width() - this->width()), 0));
        animation->setEndValue(QPoint((parent->geometry().size().width() - this->width()), -this->height()));
        animation->start();
    }
}

void PageLabelWidget::setText(const QString &text)
{
    textLabel->setText(text);
}

void PageLabelWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.fillRect(0, 0, width(), height(), QColor(0xBB000000));
}

void PageLabelWidget::updatePosition()
{
    auto parent = dynamic_cast<QWidget *>(this->parent());
    if (parent == nullptr) {
        return;
    }

    animation->stop();
    if (animation->endValue().toPoint().y() == 0)
        move(QPoint((parent->geometry().size().width() - this->width()), 0));
    else
        move(QPoint((parent->geometry().size().width() - this->width()), -this->height()));
}
