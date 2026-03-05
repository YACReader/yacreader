#include "search_volume.h"

#include "scraper_lineedit.h"
#include "scraper_checkbox.h"

#include <QLabel>
#include <QVBoxLayout>

SearchVolume::SearchVolume(QWidget *parent)
    : QWidget(parent)
{
    label = new QLabel(tr("Please provide some additional information."));

    volumeEdit = new ScraperLineEdit(tr("Series:"));
    volumeEdit->setClearButtonEnabled(true);

    exactMatchCheckBox = new ScraperCheckBox(tr("Use exact match search. Disable if you want to find volumes that match some of the words in the name."), this);
    exactMatchCheckBox->setChecked(true);

    QVBoxLayout *l = new QVBoxLayout;

    l->addSpacing(35);
    l->addWidget(label);
    l->addWidget(volumeEdit);
    l->addWidget(exactMatchCheckBox);
    l->addStretch();

    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    setContentsMargins(0, 0, 0, 0);

    initTheme(this);
}

void SearchVolume::clean()
{
    volumeEdit->clear();
}

void SearchVolume::setVolumeInfo(const QString &volume)
{
    volumeEdit->setText(volume);
}

QString SearchVolume::getVolumeInfo() const
{
    return volumeEdit->text();
}

void SearchVolume::applyTheme(const Theme &theme)
{
    auto metadataScraperDialogTheme = theme.metadataScraperDialog;

    label->setStyleSheet(metadataScraperDialogTheme.defaultLabelQSS);
}
