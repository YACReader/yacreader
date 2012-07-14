#ifndef __OPTIONS_DIALOG_H
#define __OPTIONS_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QPushButton>
#include <QRadioButton>
#include <QColorDialog>

class OptionsDialog : public QDialog
{
Q_OBJECT
	public:
		OptionsDialog(QWidget * parent = 0);
	private:
		//QLabel * pathLabel;
		QLineEdit * pathEdit;
		QPushButton * pathFindButton;

		QLabel * magGlassSizeLabel;

		QLabel * zoomLevel;

		//QLabel * slideSizeLabel;
		QSlider * slideSize;

		QPushButton * accept;
		QPushButton * cancel;

		QRadioButton *radio1; 
		QRadioButton *radio2;
		QRadioButton *radio3;

		//QLabel * fitToWidthRatioLabel;
		QSlider * fitToWidthRatioS;

		QLabel * backgroundColor;
		QPushButton * selectBackgroundColorButton;

		QColorDialog * colorDialog;

	public slots:
		void saveOptions();
		void restoreOptions();
		void findFolder();
		void updateColor(const QColor & color);

signals:
		void changedOptions();

};


#endif
