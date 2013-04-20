#ifndef __OPTIONS_DIALOG_H
#define __OPTIONS_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QSettings>
#include "pictureflow.h"

#include "custom_widgets.h"

extern FlowType flowType;

class OptionsDialog : public YACReaderOptionsDialog
{
Q_OBJECT
	public:
		OptionsDialog(QWidget * parent = 0);
};


#endif
