#include "yacreader_deleting_progress.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>
#include <QPainter>

YACReaderDeletingProgress::YACReaderDeletingProgress(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *contentLayout = new QVBoxLayout(this);

    QLabel *iconLabel = new QLabel();
    QPixmap icon(":/images/deleting_progress/icon.png");
    iconLabel->setPixmap(icon);
    iconLabel->setStyleSheet("QLabel {padding:0px; margin:0px;}");

    textMessage = new QLabel(tr("Please wait, deleting in progress..."));

    textMessage->setStyleSheet("QLabel {color:#ABABAB; padding:0 0 0 0px; margin:0px; font-size:18px; font-weight:bold;}");

    QProgressBar *progressBar = new QProgressBar();

    progressBar->setTextVisible(false);
    progressBar->setFixedHeight(6);
    progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    progressBar->setRange(0, 10);
    progressBar->setValue(5);
    progressBar->setStyleSheet(
            "QProgressBar { border: none;  border-radius: 3px; background: #ABABAB; margin:0; margin-left:16; margin-right:16px;}"
            "QProgressBar::chunk {background-color: #FFC745; border: none;  border-radius: 3px;}");

    QPushButton *button = new QPushButton(tr("cancel"));

    button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    contentLayout->addSpacing(16);
    contentLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    contentLayout->addSpacing(11);
    contentLayout->addWidget(textMessage, 0, Qt::AlignHCenter);
    contentLayout->addSpacing(13);
    contentLayout->addWidget(progressBar);
    contentLayout->addSpacing(13);
    contentLayout->addWidget(button, 0, Qt::AlignHCenter);
    contentLayout->addSpacing(18);

    contentLayout->setMargin(0);

    setLayout(contentLayout);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    resize(sizeHint());
}

void YACReaderDeletingProgress::paintEvent(QPaintEvent *event)
{
    int borderTop, borderRight, borderBottom, borderLeft;

    QPixmap pL(":/images/deleting_progress/imgTopLeft.png");
    QPixmap pM(":/images/deleting_progress/imgTopMiddle.png");
    QPixmap pR(":/images/deleting_progress/imgTopRight.png");

    QPixmap pLM(":/images/deleting_progress/imgLeftMiddle.png");

    QPixmap pRM(":/images/deleting_progress/imgRightMiddle.png");

    QPixmap pBL(":/images/deleting_progress/imgBottomLeft.png");
    QPixmap pBM(":/images/deleting_progress/imgBottomMiddle.png");
    QPixmap pBR(":/images/deleting_progress/imgBottomRight.png");

    borderTop = pL.height();
    borderRight = pRM.width();
    borderBottom = pBM.height();
    borderLeft = pLM.width();

    int width = this->width() - borderRight - borderLeft;
    int height = this->height() - borderTop - borderBottom;

    QPainter painter(this);

    //corners
    painter.drawPixmap(0, 0, pL);
    painter.drawPixmap(this->width() - borderRight, 0, pR);
    painter.drawPixmap(0, this->height() - pBL.height(), pBL);
    painter.drawPixmap(this->width() - pBR.width(), this->height() - borderBottom, pBR);

    //middle
    painter.drawPixmap(borderRight, 0, width, borderTop, pM);
    painter.drawPixmap(0, borderTop, borderLeft, height, pLM);
    painter.drawPixmap(width + borderLeft, borderTop, borderRight, height, pRM);
    painter.drawPixmap(pBR.width(), height + borderTop, this->width() - pBR.width() - pBL.width(), pBR.height(), pBM);

    //center
    painter.fillRect(borderLeft, borderTop, width, height, QColor("#FAFAFA"));

    QWidget::paintEvent(event);
}

QSize YACReaderDeletingProgress::sizeHint() const
{
    return QSize(textMessage->sizeHint().width() + 120, 185);
}
