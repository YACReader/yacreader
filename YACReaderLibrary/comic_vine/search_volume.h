#ifndef SEARCH_VOLUME_H
#define SEARCH_VOLUME_H

#include <QWidget>

class ScraperLineEdit;

class SearchVolume : public QWidget
{
    Q_OBJECT
public:
    SearchVolume(QWidget *parent = nullptr);
    void clean();
    void setVolumeInfo(const QString &volume);
public slots:
    QString getVolumeInfo() const;

private:
    ScraperLineEdit *volumeEdit;
};

#endif // SEARCH_VOLUME_H
