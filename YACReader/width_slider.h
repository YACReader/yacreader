#ifndef WIDTH_SLIDER_H
#define WIDTH_SLIDER_H

#include <QWidgetAction>

class QLabel;
class QSlider;

class YACReaderSlider : public QWidget
{
    Q_OBJECT
private:
    QLabel * percentageLabel;
    QSlider * slider;

public:
    YACReaderSlider (QWidget * parent = 0);
    void show();

protected:
    virtual void focusOutEvent(QFocusEvent * event);

public slots:
    void updateText(int value);
    void updateZoomRatio(int value);


signals:
    void zoomRatioChanged(int value);
};

class YACReaderSliderAction : public QWidgetAction 
{
	Q_OBJECT
private:
    YACReaderSlider * widget;

public:
	
	YACReaderSliderAction (QWidget * parent = 0);

public slots:
	void updateText(int value);
    void updateZoomRatio(int value);


signals:
    void zoomRatioChanged(int value);
};

#endif
