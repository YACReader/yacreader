#ifndef __OPTIONS_DIALOG_H
#define __OPTIONS_DIALOG_H

#include "yacreader_options_dialog.h"

#include "yacreader_global.h"

extern FlowType flowType;

class OptionsDialog : public YACReaderOptionsDialog
{
Q_OBJECT
	public:
		OptionsDialog(QWidget * parent = 0);
};


#endif
