#include "empty_label_widget.h"

EmptyLabelWidget::EmptyLabelWidget(QWidget *parent) :
    QWidget(parent)
{
#ifdef Q_OS_MAC
    backgroundColor = "#FFFFFF";
#else
    backgroundColor = "#2A2A2A";
#endif

    QVBoxLayout * layout = new QVBoxLayout;

    iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap(":/images/empty_label.png"));
    iconLabel->setAlignment(Qt::AlignCenter);

    //titleLabel->setText(tr("This label doesn't contain comics yet") + QString("<p style='color:rgb(150,150,150);font-size:14px;font-weight:normal;'>%1</p>").arg(tr("Drag and drop folders and comics here")));
    titleLabel = new QLabel(("This label doesn't contain comics yet"));
    titleLabel->setAlignment(Qt::AlignCenter);

#ifdef Q_OS_MAC
    titleLabel->setStyleSheet("QLabel {color:#888888; font-size:24px;font-family:Arial;font-weight:bold;}");
#else
    titleLabel->setStyleSheet("QLabel {color:#CCCCCC; font-size:24px;font-family:Arial;font-weight:bold;}");
#endif

    layout->addSpacing(100);
    layout->addWidget(iconLabel);
    layout->addSpacing(30);
    layout->addWidget(titleLabel);
    layout->addStretch();

    setLayout(layout);
}

void EmptyLabelWidget::setColor(YACReader::LabelColors color)
{
    QPixmap p(":/images/empty_label.png");
    QImage img = p.toImage().convertToFormat(QImage::Format_ARGB32);
    YACReader::colorize(img,QColor(YACReader::labelColorToRGBString(color)));
    iconLabel->setPixmap(QPixmap::fromImage(img));
}

void EmptyLabelWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter (this);
    painter.fillRect(0,0,width(),height(),QColor(backgroundColor));
}
