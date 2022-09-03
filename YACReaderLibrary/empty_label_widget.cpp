#include "empty_label_widget.h"

EmptyLabelWidget::EmptyLabelWidget(QWidget *parent)
    : EmptyContainerInfo(parent)
{
    setUpDefaultLayout(true);

    iconLabel->setPixmap(QPixmap(":/images/empty_label.png"));

    // titleLabel->setText(tr("This label doesn't contain comics yet") + QString("<p style='color:rgb(150,150,150);font-size:14px;font-weight:normal;'>%1</p>").arg(tr("Drag and drop folders and comics here")));
    titleLabel->setText(tr("This label doesn't contain comics yet"));
}

void EmptyLabelWidget::setColor(YACReader::LabelColors color)
{
    QPixmap p(":/images/empty_label.png");
    QImage img = p.toImage().convertToFormat(QImage::Format_ARGB32);
    QColor destColor(YACReader::labelColorToRGBString(color));
    YACReader::colorize(img, destColor);
    iconLabel->setPixmap(QPixmap::fromImage(img));
}
