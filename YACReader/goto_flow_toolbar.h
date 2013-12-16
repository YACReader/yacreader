#ifndef GOTO_FLOW_TOOLBAR_H
#define GOTO_FLOW_TOOLBAR_H

#include <QWidget>

class QLineEdit;
class QIntValidator;
class QPushButton;

class GoToFlowToolBar : public QWidget
{
	Q_OBJECT
	private:
		QLineEdit * edit;
		QIntValidator * v;
		QPushButton * centerButton;
		QPushButton * goToButton;
		QWidget * bar;
		void resizeEvent(QResizeEvent * event);

	public:
		GoToFlowToolBar(QWidget * parent = 0);
	public slots:
		void setPage(int pageNumber);
		void setTop(int numPages);
		void goTo();
		void centerSlide();
	signals:
		void setCenter(unsigned int);
		void goTo(unsigned int);
};

#endif