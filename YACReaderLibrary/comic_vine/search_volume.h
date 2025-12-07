#ifndef SEARCH_VOLUME_H
#define SEARCH_VOLUME_H

#include <QtWidgets>
#include "scraper_checkbox.h"

class ScraperLineEdit;
class ScraperCheckBox;

class SearchVolume : public QWidget
{
    Q_OBJECT
public:
    SearchVolume(QWidget *parent = nullptr);
    void clean();
    void setVolumeInfo(const QString &volume);
    QString getVolumeInfo() const;
    bool getExactMatch() const { return exactMatchCheckBox->isChecked(); }

private:
    ScraperLineEdit *volumeEdit;
    ScraperCheckBox *exactMatchCheckBox;
};

#endif // SEARCH_VOLUME_H
