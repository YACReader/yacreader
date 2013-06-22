#include "help_about_dialog.h"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QTextBrowser>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QDesktopWidget>

HelpAboutDialog::HelpAboutDialog(QWidget * parent)
:QDialog(parent)
{
	QVBoxLayout * layout = new QVBoxLayout();

	tabWidget = new QTabWidget();

	tabWidget->addTab(aboutText = new QTextBrowser(), tr("About"));
	aboutText->setOpenExternalLinks(true);
	//aboutText->setFont(QFont("Comic Sans MS", 10)); //purisa
	tabWidget->addTab(helpText = new QTextBrowser(), tr("Help"));
	helpText->setOpenExternalLinks(true);
	//helpText->setFont(QFont("Comic Sans MS", 10));
	//helpText->setDisabled(true);
	//tabWidget->addTab(,"About Qt");

	layout->addWidget(tabWidget);
	layout->setContentsMargins(1,3,1,1);

	setLayout(layout);
	resize(500, QApplication::desktop()->availableGeometry().height()*0.83);
}

HelpAboutDialog::HelpAboutDialog(const QString & pathAbout,const QString & pathHelp,QWidget * parent)
:QDialog(parent)
{
	loadAboutInformation(pathAbout);
	loadHelp(pathHelp);
}

void HelpAboutDialog::loadAboutInformation(const QString & path)
{
	aboutText->setHtml(fileToString(path));
	aboutText->moveCursor(QTextCursor::Start);
}

void HelpAboutDialog::loadHelp(const QString & path)
{
	helpText->setHtml(fileToString(path));
	helpText->moveCursor(QTextCursor::Start);
}

QString HelpAboutDialog::fileToString(const QString & path)
{
	QFile f(path);
	f.open(QIODevice::ReadOnly);
	QTextStream txtS(&f);

	txtS.setCodec(QTextCodec::codecForName("UTF-8"));

	QString content = txtS.readAll();
	f.close();

	return content;
}