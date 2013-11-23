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
	QVBoxLayout * layout = new QVBoxLayout;

	QHBoxLayout * switchFlowType = new QHBoxLayout;
	switchFlowType->addStretch();
	switchFlowType->addWidget(useGL);



	QHBoxLayout * buttons = new QHBoxLayout();
	buttons->addStretch();
	buttons->addWidget(accept);
	buttons->addWidget(cancel);

	layout->addWidget(sw);
	layout->addWidget(gl);
	layout->addLayout(switchFlowType);
	layout->addLayout(buttons);

	sw->hide();

	setLayout(layout);
	//restoreOptions(settings); //load options
	//resize(200,0);
	setModal (true);
	setWindowTitle(tr("Options"));

	this->layout()->setSizeConstraint(QLayout::SetFixedSize);
}


