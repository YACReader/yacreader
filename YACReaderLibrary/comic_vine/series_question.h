#ifndef SERIES_QUESTION_H
#define SERIES_QUESTION_H

#include <QWidget>

class QRadioButton;

class SeriesQuestion : public QWidget
{
	Q_OBJECT

public:
	SeriesQuestion(QWidget * parent = 0);
	bool getYes();
	void setYes(bool yes = true);

private:
	QRadioButton * yes;
	QRadioButton * no;
};


#endif // SERIES_QUESTION_H
