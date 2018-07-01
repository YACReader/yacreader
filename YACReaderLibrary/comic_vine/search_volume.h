#ifndef SEARCH_VOLUME_H
#define SEARCH_VOLUME_H

#include <QWidget>

class ScraperLineEdit;


class SearchVolume : public QWidget
{
	Q_OBJECT
public:
	SearchVolume(QWidget * parent = 0);
	void clean();
public slots:
	QString getVolumeInfo();
private:
	ScraperLineEdit * volumeEdit;
};

#endif // SEARCH_VOLUME_H
