#ifndef YACREADERCOMICINFOHELPER_H
#define YACREADERCOMICINFOHELPER_H

#include <QObject>


class ComicModel;


class YACReaderComicInfoHelper : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderComicInfoHelper(QObject *parent = 0);

    void setModel(ComicModel *model);

    Q_INVOKABLE void rate(int index, int rating);
    Q_INVOKABLE void setRead(int index, bool read);
    Q_INVOKABLE void addToFavorites(int index);
    Q_INVOKABLE void removeFromFavorites(int index);

signals:

public slots:

protected:
    ComicModel *model;
};

#endif // YACREADERCOMICINFOHELPER_H
