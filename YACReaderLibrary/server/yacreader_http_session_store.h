#ifndef YACREADERHTTPSESSIONSTORE_H
#define YACREADERHTTPSESSIONSTORE_H

#include <QObject>

class YACReaderHttpSessionStore : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderHttpSessionStore(QObject *parent = 0);

signals:

public slots:
};

#endif // YACREADERHTTPSESSIONSTORE_H