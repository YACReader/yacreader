#ifndef HELP_ABOUT_DIALOG_H
#define HELP_ABOUT_DIALOG_H

#include "themable.h"

#include <QDialog>

class QTabWidget;
class QTextBrowser;

class HelpAboutDialog : public QDialog, protected Themable
{
    Q_OBJECT
protected:
    void applyTheme(const Theme &theme) override;

public:
    HelpAboutDialog(QWidget *parent = 0);
    HelpAboutDialog(const QString &pathAbout, const QString &pathHelp, QWidget *parent = 0);
public slots:
    void loadAboutInformation(const QString &path);
    void loadHelp(const QString &path);

private:
    QTabWidget *tabWidget;
    QTextBrowser *aboutText;
    QTextBrowser *helpText;
    QTextBrowser *systemInfoText;
    QString fileToString(const QString &path);
    void loadSystemInfo();
    void applyHtmlTheme();

    QString aboutHtmlContent;
    QString helpHtmlContent;
};

#endif // HELP_ABOUT_DIALOG_H
