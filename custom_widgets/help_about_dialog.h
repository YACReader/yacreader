#ifndef HELP_ABOUT_DIALOG_H
#define HELP_ABOUT_DIALOG_H

#include "themable.h"

#include <QDialog>

#include <functional>

class QTabWidget;
class QTextBrowser;
class QPushButton;
class QShowEvent;

class HelpAboutDialog : public QDialog, protected Themable
{
    Q_OBJECT
protected:
    void applyTheme(const Theme &theme) override;
    void showEvent(QShowEvent *event) override;

public:
    HelpAboutDialog(QWidget *parent = 0);
    HelpAboutDialog(const QString &pathAbout, const QString &pathHelp, QWidget *parent = 0);
    void setAdditionalSystemInfoProvider(std::function<QString()> provider);
public slots:
    void loadAboutInformation(const QString &path);
    void loadHelp(const QString &path);

private:
    QTabWidget *tabWidget;
    QTextBrowser *aboutText;
    QTextBrowser *helpText;
    QTextBrowser *systemInfoText;
    QPushButton *changelogButton = nullptr;
    QString fileToString(const QString &path);
    void loadSystemInfo();
    void applyHtmlTheme();

    QString aboutHtmlContent;
    QString helpHtmlContent;
    std::function<QString()> additionalSystemInfoProvider;
};

#endif // HELP_ABOUT_DIALOG_H
