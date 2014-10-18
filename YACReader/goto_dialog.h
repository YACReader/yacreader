#ifndef __GOTODIALOG_H
#define __GOTODIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QIntValidator>

	class GoToDialog : public QDialog
	{
	Q_OBJECT
	public:
		GoToDialog(QWidget * parent = 0);
	private:
		QLabel * numPagesLabel;
		QLabel * textLabel;
		QLineEdit * pageNumber;
		QIntValidator * v;
		QPushButton * accept;
		QPushButton * cancel;
		void setupUI();
	public slots:
		void goTo();
		void setNumPages(unsigned int numPages);
        void open();
	signals:
		void goToPage(unsigned int page);
	};

#endif

