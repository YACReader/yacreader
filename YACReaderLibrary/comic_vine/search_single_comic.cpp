#include "search_single_comic.h"

#include "scraper_lineedit.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

SearchSingleComic::SearchSingleComic(QWidget *parent)
    : QWidget(parent)
{

    // QLabel * label = new QLabel(tr("Please provide some additional information. At least one field is needed."));
    label = new QLabel(tr("Please provide some additional information for this comic."));

    // titleEdit = new ScraperLineEdit(tr("Title:"));
    // numberEdit = new ScraperLineEdit(tr("Number:"));
    volumeEdit = new ScraperLineEdit(tr("Series:"));
    volumeEdit->setClearButtonEnabled(true);

    exactMatchCheckBox = new ScraperCheckBox(tr("Use exact match search. Disable if you want to find volumes that match some of the words in the name."));
    exactMatchCheckBox->setChecked(true);

    // numberEdit->setMaximumWidth(126);

    auto l = new QVBoxLayout;
    // QHBoxLayout * hl = new QHBoxLayout;
    // hl->addWidget(titleEdit);
    // hl->addWidget(numberEdit);

    l->addSpacing(35);
    l->addWidget(label);
    // l->addLayout(hl);
    l->addWidget(volumeEdit);
    l->addWidget(exactMatchCheckBox);
    l->addStretch();

    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    setContentsMargins(0, 0, 0, 0);

    initTheme(this);
}

QString SearchSingleComic::getVolumeInfo() const
{
    return volumeEdit->text();
}

void SearchSingleComic::setVolumeInfo(const QString &volume)
{
    volumeEdit->setText(volume);
}

QString SearchSingleComic::getComicInfo()
{
    // return titleEdit->text();
    return "";
}

int SearchSingleComic::getComicNumber()
{
    // QString numberText = numberEdit->text();
    // if(numberText.isEmpty())
    //	return -1;
    // return numberText.toInt();
    return 0;
}

void SearchSingleComic::clean()
{
    volumeEdit->clear();
}

void SearchSingleComic::applyTheme(const Theme &theme)
{
    auto metadataScraperDialogTheme = theme.metadataScraperDialog;

    label->setStyleSheet(metadataScraperDialogTheme.defaultLabelQSS);
}
