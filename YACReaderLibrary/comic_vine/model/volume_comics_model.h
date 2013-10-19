#ifndef VOLUME_COMICS_MODEL_H
#define VOLUME_COMICS_MODEL_H

#include "json_model.h"

class VolumeComicsModel : public JSONModel
{
	Q_OBJECT
public:
	explicit VolumeComicsModel(QObject *parent = 0);
	void load(const QString & json);
signals:

public slots:

};

#endif // VOLUME_COMICS_MODEL_H
