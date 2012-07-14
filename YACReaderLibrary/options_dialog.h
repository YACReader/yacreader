#ifndef __OPTIONS_DIALOG_H
#define __OPTIONS_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QPushButton>
#include <QRadioButton>
#include "pictureflow.h"

extern PictureFlow::FlowType flowType;

class OptionsDialog : public QDialog
{
Q_OBJECT
	public:
		OptionsDialog(QWidget * parent = 0);
	private:
		QLabel * pathLabel;
		QLineEdit * pathEdit;
		QPushButton * pathFindButton;

		QLabel * magGlassSizeLabel;

		QLabel * zoomLevel;

		QLabel * slideSizeLabel;
		QSlider * slideSize;

		QPushButton * accept;
		QPushButton * cancel;

		QRadioButton *radio1; 
		QRadioButton *radio2;
		QRadioButton *radio3;

	public slots:
		void saveOptions();
		void restoreOptions();
		void findFolder();

signals:
		void optionsChanged();

};


#endif
