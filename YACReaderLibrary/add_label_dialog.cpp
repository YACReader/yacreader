#include "add_label_dialog.h"

AddLabelDialog::AddLabelDialog(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout * layout = new QVBoxLayout;

    layout->addWidget(new QLabel(tr("Label name:")));
    layout->addWidget(edit = new QLineEdit());

    layout->addWidget(new QLabel(tr("Choose a color:")));
    layout->addWidget(list = new QListWidget() );

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

    //this->palette().background().color().
    list->setStyleSheet("QListWidget {border : none; background-color:#BCBCBCBC;}");

    setLayout(layout);
}
