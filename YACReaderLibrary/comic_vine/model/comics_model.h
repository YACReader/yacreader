#ifndef COMICS_MODEL_H
#define COMICS_MODEL_H

#include "json_model.h"

class ComicsModel : public JSONModel
{
	Q_OBJECT
public:
	explicit ComicsModel(QObject *parent = 0);

signals:

public slots:

};

#endif // COMICS_MODEL_H
