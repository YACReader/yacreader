#include "select_volume.h"

#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "volumes_model.h"

SelectVolume::SelectVolume(QWidget *parent)
	:QWidget(parent),model(0)
{
	QString labelStylesheet = "QLabel {color:white; font-size:12px;font-family:Arial;}";
	QString tableStylesheet = ""
			"QTableView {alternate-background-color: #333333;background-color: #2B2B2B; outline: 0px;}"// border: 1px solid #999999; border-right:none; border-bottom:none;}"
			"QTableCornerButton::section {background-color:#F5F5F5; border:none; border-bottom:1px solid #B8BDC4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D1D1D1, stop: 1 #B8BDC4);}"
			"QTableView::item {outline: 0px; border: 0px color:#FFFFFF;}"
			"QTableView {border-top:1px solid #B8B8B8;border-bottom:none;border-left:1px solid #B8B8B8;border-right:none;}"
			"QTableView::item:selected {outline: 0px; border-bottom: 1px solid #D4D4D4;border-top: 1px solid #D4D4D4; padding-bottom:1px; background-color: #D4D4D4;  }"
			"QHeaderView::section:horizontal {background-color:#F5F5F5; border-bottom:1px solid #B8BDC4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D1D1D1, stop: 1 #B8BDC4); border-left:none; border-top:none; padding:4px; color:#313232;}"
			"QHeaderView::section:vertical {border-bottom: 1px solid #DFDFDF;border-top: 1px solid #FEFEFE;}"
			//"QTableView::item:hover {border-bottom: 1px solid #A3A3A3;border-top: 1px solid #A3A3A3; padding-bottom:1px; background-color: #A3A3A3; color: #FFFFFF; }"

			//scrollbar

			"QScrollBar:vertical { border: none; background: #404040; width: 3px; margin: 0; }"
			"QScrollBar::handle:vertical { background: #DDDDDD; width: 7px; min-height: 20px; }"
			"QScrollBar::add-line:vertical { border: none; background: #404040; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"

			"QScrollBar::sub-line:vertical {  border: none; background: #404040; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
			"QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
			"QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"

			"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none; }"
			"";

	QLabel * label = new QLabel(tr("Please, select the right series for your comic."));
	label->setStyleSheet(labelStylesheet);

	QVBoxLayout * l = new QVBoxLayout;
	QWidget * leftWidget = new QWidget;
	QVBoxLayout * left = new QVBoxLayout;
	QHBoxLayout * content = new QHBoxLayout;

	//widgets
	cover = new QLabel();
	detailLabel = new QLabel();
	detailLabel->setStyleSheet(labelStylesheet);
	tableVolumes = new QTableView();
	tableVolumes->setStyleSheet(tableStylesheet);

	left->addWidget(cover);
	left->addWidget(detailLabel);
	left->addStretch();
	leftWidget->setMaximumWidth(168);
	leftWidget->setLayout(left);

	content->addWidget(leftWidget);
	content->addWidget(tableVolumes);

	l->addSpacing(15);
	l->addWidget(label);
	l->addLayout(content);
	l->addStretch();

	l->setContentsMargins(0,0,0,0);
	setLayout(l);
	setContentsMargins(0,0,0,0);
}

void SelectVolume::load(const QString & json)
{
	if(model != 0)
		delete model;
	else
		model = new VolumesModel();

	model->load(json);
}


SelectVolume::~SelectVolume() {}
