#ifndef SEARCH_VOLUME_H
#define SEARCH_VOLUME_H

#include <QtWidgets>
#include "scraper_checkbox.h"
#include "themable.h"

class ScraperLineEdit;
class ScraperCheckBox;

class SearchVolume : public QWidget, protected Themable
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
    QLabel *label;

protected:
    void applyTheme(const Theme &theme) override;
};

#endif // SEARCH_VOLUME_H
