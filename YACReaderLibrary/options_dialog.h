#ifndef __OPTIONS_DIALOG_H
#define __OPTIONS_DIALOG_H

#include "yacreader_options_dialog.h"

#include "yacreader_global.h"

using namespace YACReader;

class OptionsDialog : public YACReaderOptionsDialog
{
Q_OBJECT
	public:
		OptionsDialog(QWidget * parent = 0);
signals:
        void editShortcuts();
};


#endif
