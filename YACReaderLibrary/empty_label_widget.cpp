#include "empty_label_widget.h"

EmptyLabelWidget::EmptyLabelWidget(QWidget *parent)
    : EmptyContainerInfo(parent)
{
    setUpDefaultLayout(true);
    titleLabel->setText(tr("This label doesn't contain comics yet"));
}

void EmptyLabelWidget::setColor(YACReader::LabelColors color)
{
    currentColor = color;
    auto it = theme.emptyContainer.emptyLabelIcons.find(static_cast<int>(color));
    if (it != theme.emptyContainer.emptyLabelIcons.end()) {
        setPixmap(it.value());
    }
}

void EmptyLabelWidget::applyTheme(const Theme &theme)
{
    EmptyContainerInfo::applyTheme(theme);
    setColor(currentColor);
}
