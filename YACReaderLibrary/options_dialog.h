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

extern PictureFlow::FlowType flowType;

class OptionsDialog : public QDialog
{
Q_OBJECT
	public:
		OptionsDialog(QWidget * parent = 0);
	private:
		
		QPushButton * accept;
		QPushButton * cancel;
		QCheckBox * useGL;

		//SW.........................
		QRadioButton *radio1; 
		QRadioButton *radio2;
		QRadioButton *radio3;

		//GL.........................
		QRadioButton *radioClassic; 
		QRadioButton *radioStripe;
		QRadioButton *radioOver;
		QRadioButton *radionModern;
		QRadioButton *radioDown;

		YACReaderSpinSliderWidget * xRotation;
		YACReaderSpinSliderWidget * yPosition;
		YACReaderSpinSliderWidget * coverDistance;
		YACReaderSpinSliderWidget * centralDistance;
		YACReaderSpinSliderWidget * zoomLevel;
		YACReaderSpinSliderWidget * yCoverOffset;
		YACReaderSpinSliderWidget * zCoverOffset;
		YACReaderSpinSliderWidget * coverRotation;
		YACReaderSpinSliderWidget * fadeOutDist;
		YACReaderSpinSliderWidget * lightStrength;
		YACReaderSpinSliderWidget * maxAngle;

		QSlider * performanceSlider;

		QWidget * sw;
		QWidget * gl;

		QLayout * setupLayoutSW();
		QLayout * setupLayoutGL();

		QSettings * settings;
		QSettings * previousSettings;

	private slots:
			void savePerformance(int value);
			void saveUseGL(int b);
			void saveXRotation(int value);
			void saveYPosition(int value);
			void saveCoverDistance(int value);
			void saveCentralDistance(int value);
			void saveZoomLevel(int value);
			void saveYCoverOffset(int value);
			void saveZCoverOffset(int value);
			void saveCoverRotation(int value);
			void saveFadeOutDist(int value);
			void saveLightStrength(int value);
			void saveMaxAngle(int value);
			void loadConfig();
			void setClassicConfig();
			void setStripeConfig();
			void setOverlappedStripeConfig();
			void setModernConfig();
			void setRouletteConfig();
	public slots:
		void saveOptions();
		void restoreOptions(QSettings * settings);
signals:
		void optionsChanged();



};


#endif
