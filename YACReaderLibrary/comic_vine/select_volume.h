#ifndef SELECT_VOLUME_H
#define SELECT_VOLUME_H

#include <QWidget>

class QLabel;
class VolumesModel;
class QModelIndex;

class ScrapperTableView;

class SelectVolume : public QWidget
{
	Q_OBJECT
public:
	SelectVolume(QWidget * parent = 0);
	void load(const QString & json);
	virtual ~SelectVolume();
public slots:
	void loadVolumeInfo(const QModelIndex & mi);
	void setCover(const QByteArray &);
	void setDescription(const QString & jsonDetail);
	void openLink(const QString & link);
	QString getSelectedVolumeId();
private:
	QLabel * cover;
	QLabel * detailLabel;
	ScrapperTableView * tableVolumes;
	VolumesModel * model;
};

#endif // SELECT_VOLUME_H
