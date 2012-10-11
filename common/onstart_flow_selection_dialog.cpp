#include "onstart_flow_selection_dialog.h"

#include <QPushButton>
#include <QHBoxLayout>

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
	paletteHW.setBrush(acceptHW->backgroundRole(), QBrush(QImage(":/images/useNewFlowButton.png")));
	acceptHW->setPalette(paletteHW);
	QPalette paletteSW;
	paletteSW.setBrush(rejectHW->backgroundRole(), QBrush(QImage(":/images/useOldFlowButton.png")));
	rejectHW->setPalette(paletteSW);
	//QHBoxLayout * layout = new QHBoxLayout;
	//layout->addWidget(acceptHW);
	//layout->addWidget(rejectHW);

	QPalette palette;
	palette.setBrush(this->backgroundRole(), QBrush(QImage(":/images/onStartFlowSelection.png")));
	setPalette(palette);


	//setLayout(layout);

	resize(664,371);
}
