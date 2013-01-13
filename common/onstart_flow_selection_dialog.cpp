#include "onstart_flow_selection_dialog.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <qlocale.h>

OnStartFlowSelectionDialog::OnStartFlowSelectionDialog(QWidget * parent)
	:QDialog(parent)
{
	setModal(true);
	QPushButton * acceptHW = new QPushButton(this);
	connect(acceptHW,SIGNAL(clicked()),this,SLOT(accept()));
	QPushButton * rejectHW = new QPushButton(this); //and use SW flow
	connect(rejectHW,SIGNAL(clicked()),this,SLOT(reject()));

	acceptHW->setGeometry(90,165,110,118);
	acceptHW->setFlat(true);
	acceptHW->setAutoFillBackground(true);
	rejectHW->setGeometry(464,165,110,118);
	rejectHW->setFlat(true);
	rejectHW->setAutoFillBackground(true);

	QPalette paletteHW;
	QLocale locale = this->locale();
	QLocale::Language language = locale.language();

	/*if(language == QLocale::Spanish)
		paletteHW.setBrush(acceptHW->backgroundRole(), QBrush(QImage(":/images/useNewFlowButton_es.png")));
	else
		paletteHW.setBrush(acceptHW->backgroundRole(), QBrush(QImage(":/images/useNewFlowButton.png")));*/


	paletteHW.setBrush(acceptHW->backgroundRole(), QBrush(QImage(":/images/nonexxx.png")));
	acceptHW->setPalette(paletteHW);
	QPalette paletteSW;
	paletteSW.setBrush(rejectHW->backgroundRole(), QBrush(QImage(":/images/nonexxx.png")));
	rejectHW->setPalette(paletteSW);
	//QHBoxLayout * layout = new QHBoxLayout;
	//layout->addWidget(acceptHW);
	//layout->addWidget(rejectHW);

	QPalette palette;
	if(language == QLocale::Spanish)
		palette.setBrush(this->backgroundRole(), QBrush(QImage(":/images/onStartFlowSelection_es.png")));
	else
		palette.setBrush(this->backgroundRole(), QBrush(QImage(":/images/onStartFlowSelection.png")));

	setPalette(palette);


	//setLayout(layout);

	resize(664,371);
}
