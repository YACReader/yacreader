#include "width_slider.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include "configuration.h"

YACReaderSliderAction::YACReaderSliderAction (QWidget * parent) 
	:QWidgetAction (0) {
    
        QWidget* pWidget = new QWidget (NULL);
        QHBoxLayout* pLayout = new QHBoxLayout();

		pLayout->addStretch();

        percentageLabel = new QLabel ("100%");
		percentageLabel->setStyleSheet("QLabel { color : white; }");
		percentageLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        pLayout->addWidget (percentageLabel);
        slider = new QSlider(NULL);
		slider->setOrientation(Qt::Horizontal);
        pLayout->addWidget (slider);

		QString sliderCSS = 
			
			"QSlider::sub-page:horizontal {background-image: url(:/images/sliderSubPage.png); border: 0px; margin-left: 18px;}"
			"QSlider::add-page:horizontal {background-image: url(:/images/sliderAddPage.png); border: 0px; margin-right: 25px;}"
			"QSlider::handle:horizontal {image: url(:/images/sliderHandle.png); width: 31px;height:45px; }" 
			"QSlider::groove:horizontal {border-image:url(:/images/sliderGround.png); border-left:-2px; border-right:0;}" 
			;
		slider->setStyleSheet(sliderCSS); 
		slider->setFixedSize(218,45);
        
		QLabel*	imgLabel = new QLabel(pWidget);  
		QPixmap p(":/images/sliderBackground.png"); 
		imgLabel->resize(p.size());
		imgLabel->setPixmap(p);

		pLayout->setMargin(0);
		pLayout->setSpacing(0);

		pLayout->setStretchFactor(percentageLabel,1);
		pLayout->setStretchFactor(slider,0);
		

		pWidget->setLayout (pLayout);
		pWidget->setAutoFillBackground(false);

		pWidget->setMinimumSize(276,45);

        setDefaultWidget(pWidget);
		
		slider->setMinimum(50);
		slider->setMaximum(100);
		slider->setPageStep(5);
		
		int value = Configuration::getConfiguration().getFitToWidthRatio()*100;
		slider->setValue(value);
		percentageLabel->setText(QString("%1 %").arg(value));
		connect(slider,SIGNAL(valueChanged(int)),this,SLOT(updateText(int)));

		
}

void YACReaderSliderAction::updateText(int value)
{
	percentageLabel->setText(QString("%1 %").arg(value));
	Configuration::getConfiguration().setFitToWidthRatio(value/100.0);
	emit(fitToWidthRatioChanged(value / 100.0f));
}

void YACReaderSliderAction::updateFitToWidthRatio(float v)
{
	int value = v*100;
	slider->setValue(value);
	percentageLabel->setText(QString("%1 %").arg(value));
}