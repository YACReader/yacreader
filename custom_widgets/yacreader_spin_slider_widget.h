#ifndef YACREADER_SPIN_SLIDER_WIDGET_H
#define YACREADER_SPIN_SLIDER_WIDGET_H

#include <QWidget>

class QLabel;
class QSpinBox;
class QSlider;

class YACReaderSpinSliderWidget : public QWidget
{
	Q_OBJECT
private:
	QLabel * label;
	QSpinBox * spinBox;
	QSlider * slider;
	bool tracking;
public:
	YACReaderSpinSliderWidget(QWidget * parent = 0,bool strechableSlider = false);
public slots:
	void setRange(int lowValue, int topValue, int step=1);
	void setValue(int value);
	void setText(const QString & text);
	int getValue();
	QSize minimumSizeHint() const;
	void setTracking(bool b);
	void valueWillChange(int);
	void valueWillChangeFromSpinBox(int);
	void sliderRelease();
signals:
	void valueChanged(int);

};

#endif // YACREADER_SPIN_SLIDER_WIDGET_H