#include "add_label_dialog.h"

#include <QLabel>
#include <QVBoxLayout>

namespace {
const char *labelColorNames[] = {
    QT_TRANSLATE_NOOP("AddLabelDialog", "red"),
    QT_TRANSLATE_NOOP("AddLabelDialog", "orange"),
    QT_TRANSLATE_NOOP("AddLabelDialog", "yellow"),
    QT_TRANSLATE_NOOP("AddLabelDialog", "green"),
    QT_TRANSLATE_NOOP("AddLabelDialog", "cyan"),
    QT_TRANSLATE_NOOP("AddLabelDialog", "blue"),
    QT_TRANSLATE_NOOP("AddLabelDialog", "violet"),
    QT_TRANSLATE_NOOP("AddLabelDialog", "purple"),
    QT_TRANSLATE_NOOP("AddLabelDialog", "pink"),
    QT_TRANSLATE_NOOP("AddLabelDialog", "white"),
    QT_TRANSLATE_NOOP("AddLabelDialog", "light"),
    QT_TRANSLATE_NOOP("AddLabelDialog", "dark")
};
}

AddLabelDialog::AddLabelDialog(QWidget *parent)
    : QDialog(parent)
{
    auto layout = new QVBoxLayout;

    layout->addWidget(new QLabel(tr("Label name:")));
    layout->addWidget(edit = new QLineEdit());

    layout->addWidget(new QLabel(tr("Choose a color:")));
    layout->addWidget(list = new QListWidget());

    for (const auto &colorName : labelColorNames) {
        list->addItem(new QListWidgetItem(tr(colorName)));
    }

    list->setMinimumHeight(225);

    setModal(true);

    setMinimumHeight(340);

    // buttons
    acceptButton = new QPushButton(tr("accept"), this);
    cancelButton = new QPushButton(tr("cancel"), this);

    auto buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(acceptButton);
    buttons->addWidget(cancelButton);

    layout->addStretch();
    layout->addLayout(buttons);

    setLayout(layout);

    // connections
    connect(edit, &QLineEdit::textChanged, this, &AddLabelDialog::validateName);
    connect(cancelButton, &QAbstractButton::clicked, this, &QWidget::close);
    connect(acceptButton, &QAbstractButton::clicked, this, &QDialog::accept);

    initTheme(this);
}

void AddLabelDialog::applyTheme(const Theme &theme)
{
    const auto &icons = theme.readingListIcons.labelIcons;

    for (int i = 0; i < list->count(); ++i) {
        const QString colorName = labelColorNames[i];
        if (icons.contains(colorName)) {
            list->item(i)->setIcon(icons[colorName]);
        }
    }

    list->setStyleSheet("QListWidget {border : none;}");
}

YACReader::LabelColors AddLabelDialog::selectedColor()
{
    return YACReader::LabelColors(list->currentRow() + 1);
}

QString AddLabelDialog::name()
{
    return edit->text();
}

int AddLabelDialog::exec()
{
    edit->clear();
    list->clearSelection();

    acceptButton->setDisabled(true);

    list->setCurrentRow(0);

    return QDialog::exec();
}

void AddLabelDialog::validateName(const QString &name)
{
    if (name.isEmpty())
        acceptButton->setDisabled(true);
    else
        acceptButton->setEnabled(true);
}
