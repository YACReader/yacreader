#ifndef SCRAPPER_LINEEDIT_H
#define SCRAPPER_LINEEDIT_H

#include <QLineEdit>

class QLabel;

class ScrapperLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	ScrapperLineEdit(const QString & title, QWidget * widget = 0);
protected:
	void resizeEvent(QResizeEvent *);
private:
	QLabel * titleLabel;
};

#endif // SCRAPPER_LINEEDIT_H
