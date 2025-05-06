#ifndef SEARCH_SINGLE_COMIC_H
#define SEARCH_SINGLE_COMIC_H

#include <QtWidgets>

#include "scraper_checkbox.h"

class ScraperLineEdit;

class SearchSingleComic : public QWidget
{
    Q_OBJECT
public:
    SearchSingleComic(QWidget *parent = nullptr);
    QString getVolumeInfo() const;
    bool getExactMatch() const { return exactMatchCheckBox->isChecked(); }
    void setVolumeInfo(const QString &volume);
    QString getComicInfo();
    int getComicNumber();
    void clean();

private:
    // ScraperLineEdit *titleEdit;
    // ScraperLineEdit *numberEdit;
    ScraperLineEdit *volumeEdit;
    ScraperCheckBox *exactMatchCheckBox;
};
#endif // SEARCH_SINGLE_COMIC_H
