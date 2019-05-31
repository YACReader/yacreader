#include "series_question.h"

#include <QRadioButton>
#include <QVBoxLayout>
#include <QLabel>

SeriesQuestion::SeriesQuestion(QWidget *parent)
    : QWidget(parent)
{
    auto l = new QVBoxLayout;

    QLabel *questionLabel = new QLabel(tr("You are trying to get information for various comics at once, are they part of the same series?"));
    questionLabel->setStyleSheet("QLabel {color:white; font-size:12px;font-family:Arial;}");
    yes = new QRadioButton(tr("yes"));
    no = new QRadioButton(tr("no"));

    QString rbStyle = "QRadioButton {margin-left:27px; margin-top:5px; color:white;font-size:12px;font-family:Arial;}"
                      "QRadioButton::indicator {width:11px;height:11px;}"
                      "QRadioButton::indicator::unchecked {image : url(:/images/comic_vine/radioUnchecked.png);}"
                      "QRadioButton::indicator::checked {image : url(:/images/comic_vine/radioChecked.png);}";
    yes->setStyleSheet(rbStyle);
    no->setStyleSheet(rbStyle);

    yes->setChecked(true);

    l->addSpacing(35);
    l->addWidget(questionLabel);
    l->addWidget(yes);
    l->addWidget(no);
    l->addStretch();

    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    setContentsMargins(0, 0, 0, 0);
}

bool SeriesQuestion::getYes()
{
    return yes->isChecked();
}

void SeriesQuestion::setYes(bool y)
{
    yes->setChecked(y);
}
