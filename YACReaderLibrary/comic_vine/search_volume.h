#ifndef SEARCH_VOLUME_H
#define SEARCH_VOLUME_H

#include <QWidget>

class ScrapperLineEdit;


class SearchVolume : public QWidget
{
	Q_OBJECT
public:
	SearchVolume(QWidget * parent = 0);
public slots:
	QString getVolumeInfo();
private:
	ScrapperLineEdit * volumeEdit;
};

#endif // SEARCH_VOLUME_H
