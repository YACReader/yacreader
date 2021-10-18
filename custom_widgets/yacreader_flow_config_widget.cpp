#include "yacreader_flow_config_widget.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>

YACReaderFlowConfigWidget::YACReaderFlowConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QGroupBox *groupBox = new QGroupBox(tr("How to show covers:"));

    radio1 = new QRadioButton(tr("CoverFlow look"));
    radio2 = new QRadioButton(tr("Stripe look"));
    radio3 = new QRadioButton(tr("Overlapped Stripe look"));

    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *opt1 = new QHBoxLayout;
    opt1->addWidget(radio1);
    QLabel *lOpt1 = new QLabel();
    lOpt1->setPixmap(QPixmap(":/images/flow1.png"));
    opt1->addStretch();
    opt1->addWidget(lOpt1);
    vbox->addLayout(opt1);

    QHBoxLayout *opt2 = new QHBoxLayout;
    opt2->addWidget(radio2);
    QLabel *lOpt2 = new QLabel();
    lOpt2->setPixmap(QPixmap(":/images/flow2.png"));
    opt2->addStretch();
    opt2->addWidget(lOpt2);
    vbox->addLayout(opt2);

    QHBoxLayout *opt3 = new QHBoxLayout;
    opt3->addWidget(radio3);
    QLabel *lOpt3 = new QLabel();
    lOpt3->setPixmap(QPixmap(":/images/flow3.png"));
    opt3->addStretch();
    opt3->addWidget(lOpt3);
    vbox->addLayout(opt3);

    // vbox->addStretch(1);
    groupBox->setLayout(vbox);

    layout->addWidget(groupBox);

    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);
}