#include "options_dialog.h"

#ifndef NO_OPENGL
#include "yacreader_flow_gl.h"
#include "yacreader_gl_flow_config_widget.h"
#endif
#include "yacreader_flow_config_widget.h"
#include "api_key_dialog.h"

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
#include <QtWidgets>


FlowType flowType = Strip;

OptionsDialog::OptionsDialog(QWidget * parent)
:YACReaderOptionsDialog(parent)
{
    QTabWidget * tabWidget = new QTabWidget();

    QVBoxLayout * layout = new QVBoxLayout(this);

    QVBoxLayout * flowLayout = new QVBoxLayout;
    QVBoxLayout * generalLayout = new QVBoxLayout();

    QHBoxLayout * switchFlowType = new QHBoxLayout();
    switchFlowType->addStretch();
#ifndef NO_OPENGL
    switchFlowType->addWidget(useGL);
#endif
    QHBoxLayout * buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(accept);
    buttons->addWidget(cancel);

    flowLayout->addWidget(sw);
#ifndef NO_OPENGL
    flowLayout->addWidget(gl);
#endif
    flowLayout->addLayout(switchFlowType);

#ifndef NO_OPENGL
    sw->hide();
#endif

    QVBoxLayout * apiKeyLayout = new QVBoxLayout();
    QPushButton * apiKeyButton = new QPushButton(tr("Edit Comic Vine API key"));
    apiKeyLayout->addWidget(apiKeyButton);

    QGroupBox * apiKeyBox = new QGroupBox(tr("Comic Vine API key"));
    apiKeyBox->setLayout(apiKeyLayout);

    connect(apiKeyButton,SIGNAL(clicked()),this,SLOT(editApiKey()));

    QWidget * comicFlowW = new QWidget;
    comicFlowW->setLayout(flowLayout);

    QWidget * generalW = new QWidget;
    generalW->setLayout(generalLayout);
    generalLayout->addWidget(shortcutsBox);
    generalLayout->addWidget(apiKeyBox);
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

void OptionsDialog::editApiKey()
{
    ApiKeyDialog d;
    d.exec();
}



