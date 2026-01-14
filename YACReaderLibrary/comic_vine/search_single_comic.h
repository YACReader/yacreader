#ifndef SEARCH_SINGLE_COMIC_H
#define SEARCH_SINGLE_COMIC_H

#include <QtWidgets>

#include "scraper_checkbox.h"
#include "themable.h"

class ScraperLineEdit;
class QLabel;

class SearchSingleComic : public QWidget, protected Themable
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
    QLabel *label;

protected:
    void applyTheme() override;
};
#endif // SEARCH_SINGLE_COMIC_H
