#include "series_question.h"

#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

SeriesQuestion::SeriesQuestion(QWidget *parent)
    : QWidget(parent)
{
    auto l = new QVBoxLayout;

    questionLabel = new QLabel(tr("You are trying to get information for various comics at once, are they part of the same series?"));
    yes = new QRadioButton(tr("yes"));
    no = new QRadioButton(tr("no"));

    yes->setChecked(true);

    l->addSpacing(35);
    l->addWidget(questionLabel);
    l->addWidget(yes);
    l->addWidget(no);
    l->addStretch();

    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    setContentsMargins(0, 0, 0, 0);

    initTheme(this);
}

bool SeriesQuestion::getYes()
{
    return yes->isChecked();
}

void SeriesQuestion::setYes(bool y)
{
    yes->setChecked(y);
}

void SeriesQuestion::applyTheme(const Theme &theme)
{
    auto metadataScraperDialogTheme = theme.metadataScraperDialog;

    questionLabel->setStyleSheet(metadataScraperDialogTheme.defaultLabelQSS);
    yes->setStyleSheet(metadataScraperDialogTheme.radioButtonQSS);
    no->setStyleSheet(metadataScraperDialogTheme.radioButtonQSS);
}
