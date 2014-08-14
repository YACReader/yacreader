#include "options_dialog.h"

#include "yacreader_flow_gl.h"
#include "yacreader_flow_config_widget.h"
#include "yacreader_gl_flow_config_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QTextStream>
#include <QCoreApplication>
#include <QFile>
#include <QMessageBox>
#include <QCheckBox>

FlowType flowType = Strip;

OptionsDialog::OptionsDialog(QWidget * parent)
:YACReaderOptionsDialog(parent)
{
    QTabWidget * tabWidget = new QTabWidget();

    QVBoxLayout * layout = new QVBoxLayout(this);

    QVBoxLayout * flowLayout = new QVBoxLayout;
    QVBoxLayout * generalLayout = new QVBoxLayout();

    QHBoxLayout * switchFlowType = new QHBoxLayout;
    switchFlowType->addStretch();
    switchFlowType->addWidget(useGL);

    QHBoxLayout * buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(accept);
    buttons->addWidget(cancel);

    flowLayout->addWidget(sw);
    flowLayout->addWidget(gl);
    flowLayout->addLayout(switchFlowType);

    sw->hide();

    QWidget * comicFlowW = new QWidget;
    comicFlowW->setLayout(flowLayout);

    QWidget * generalW = new QWidget;
    generalW->setLayout(generalLayout);
    generalLayout->addWidget(shortcutsBox);
    generalLayout->addStretch();

    tabWidget->addTab(comicFlowW,tr("Comic Flow"));
    tabWidget->addTab(generalW,tr("General"));

    layout->addWidget(tabWidget);
    layout->addLayout(buttons);
    setLayout(layout);
    //restoreOptions(settings); //load options
    //resize(200,0);
    setModal (true);
    setWindowTitle(tr("Options"));

    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

}



