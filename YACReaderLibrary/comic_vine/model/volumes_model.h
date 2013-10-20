#ifndef VOLUMES_MODEL_H
#define VOLUMES_MODEL_H

#include "json_model.h"

class VolumesModel : public JSONModel
{
	Q_OBJECT
public:
	explicit VolumesModel(QObject *parent = 0);
	virtual ~VolumesModel();
	//receive a valid json with a list of volumes
	void load(const QString & json);

	//QAbstractItemModel methods
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

	QString getVolumeId(const QModelIndex & index) const;
	QString getCoverURL(const QModelIndex & index) const;

signals:

public slots:

private:
	QList <QList <QString> > _data;

public:
	enum Column {
		SERIES = 0,
		YEAR,
		ISSUES,
		PUBLISHER,
		COVER_URL,
		DECK,
		ID
	};

};

#endif // VOLUMES_MODEL_H
