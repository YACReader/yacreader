#ifndef JSON_MODEL_H
#define JSON_MODEL_H

#include <QAbstractItemModel>

class JSONModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit JSONModel(QObject *parent = nullptr);
    virtual void load(const QString &json) = 0;

signals:

public slots:
};

#endif // JSON_MODEL_H
