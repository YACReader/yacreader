#ifndef SERIES_QUESTION_H
#define SERIES_QUESTION_H

#include <QWidget>

#include "themable.h"

class QRadioButton;
class QLabel;

class SeriesQuestion : public QWidget, protected Themable
{
    Q_OBJECT

public:
    SeriesQuestion(QWidget *parent = nullptr);
    bool getYes();
    void setYes(bool yes = true);

private:
    QLabel *questionLabel;
    QRadioButton *yes;
    QRadioButton *no;

protected:
    void applyTheme() override;
};

#endif // SERIES_QUESTION_H
