#ifndef WIDTH_SLIDER_H
#define WIDTH_SLIDER_H

#include <QWidgetAction>

class QLabel;
class QSlider;

class YACReaderSliderAction : public QWidgetAction 
{
	Q_OBJECT
private:
	QLabel * percentageLabel;
	QSlider * slider;

public:
	
    YACReaderSliderAction (QWidget * parent = 0);

public slots:
	void updateText(int value);
	void updateFitToWidthRatio(float v);


signals:
	void fitToWidthRatioChanged(float value);
};

#endif
