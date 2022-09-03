#include "add_label_dialog.h"

AddLabelDialog::AddLabelDialog(QWidget *parent)
    : QDialog(parent)
{
    auto layout = new QVBoxLayout;

    layout->addWidget(new QLabel(tr("Label name:")));
    layout->addWidget(edit = new QLineEdit());

    layout->addWidget(new QLabel(tr("Choose a color:")));
    layout->addWidget(list = new QListWidget());

    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_red.png"), tr("red")));
    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_orange.png"), tr("orange")));
    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_yellow.png"), tr("yellow")));
    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_green.png"), tr("green")));
    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_cyan.png"), tr("cyan")));
    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_blue.png"), tr("blue")));
    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_violet.png"), tr("violet")));
    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_purple.png"), tr("purple")));
    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_pink.png"), tr("pink")));
    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_white.png"), tr("white")));
    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_light.png"), tr("light")));
    list->addItem(new QListWidgetItem(QIcon(":/images/lists/label_dark.png"), tr("dark")));

    QColor backgroundColor = this->palette().window().color();
    list->setStyleSheet(QString("QListWidget {border : none; background-color: rgb(%1,%2,%3);}").arg(backgroundColor.red()).arg(backgroundColor.green()).arg(backgroundColor.blue()));
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
