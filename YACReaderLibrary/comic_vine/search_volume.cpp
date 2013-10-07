#include "search_volume.h"

#include "scrapper_lineedit.h"

#include <QLabel>
#include <QVBoxLayout>

SearchVolume::SearchVolume(QWidget * parent)
	:QWidget(parent)
{
	QLabel * label = new QLabel(tr("No results found, please provide some aditional information."));
	label->setStyleSheet("QLabel {color:white; font-size:12px;font-family:Arial;}");

	volumeEdit = new ScrapperLineEdit(tr("Series:"));

	QVBoxLayout * l = new QVBoxLayout;

	l->addSpacing(35);
	l->addWidget(label);
	l->addWidget(volumeEdit);
	l->addStretch();

	l->setContentsMargins(0,0,0,0);
	setLayout(l);
	setContentsMargins(0,0,0,0);
}

QString SearchVolume::getVolumeInfo()
{
	return volumeEdit->text();
}
