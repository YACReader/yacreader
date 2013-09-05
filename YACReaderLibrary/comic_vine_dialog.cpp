#include "comic_vine_dialog.h"

ComicVineDialog::ComicVineDialog(QWidget *parent) :
	QDialog(parent)
{
	setStyleSheet(""
		"QDialog {background-color: #404040; }"
		"");
	setFixedSize(672,529);
}
