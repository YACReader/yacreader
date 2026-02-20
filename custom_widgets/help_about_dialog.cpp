#include "help_about_dialog.h"

#include "global_info_provider.h"

#include <QtCore>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTextBrowser>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QScreen>

#include "yacreader_global.h"

HelpAboutDialog::HelpAboutDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();

    tabWidget = new QTabWidget();

    tabWidget->addTab(aboutText = new QTextBrowser(), tr("About"));
    aboutText->setOpenExternalLinks(true);
    // aboutText->setFont(QFont("Comic Sans MS", 10)); //purisa
    tabWidget->addTab(helpText = new QTextBrowser(), tr("Help"));
    helpText->setOpenExternalLinks(true);

    tabWidget->addTab(systemInfoText = new QTextBrowser(), tr("System info"));
    // helpText->setFont(QFont("Comic Sans MS", 10));
    // helpText->setDisabled(true);
    // tabWidget->addTab(,"About Qt");

    layout->addWidget(tabWidget);
    layout->setContentsMargins(1, 3, 1, 1);

    setLayout(layout);

    QScreen *screen = parent != nullptr ? parent->window()->screen() : nullptr;
    if (screen == nullptr) {
        screen = QApplication::screens().constFirst();
    }

    int heightDesktopResolution = screen != nullptr ? screen->size().height() : 600;

    resize(500, heightDesktopResolution * 0.83);

    loadSystemInfo();

    initTheme(this);
}

HelpAboutDialog::HelpAboutDialog(const QString &pathAbout, const QString &pathHelp, QWidget *parent)
    : QDialog(parent)
{
    loadAboutInformation(pathAbout);
    loadHelp(pathHelp);
}

void HelpAboutDialog::loadAboutInformation(const QString &path)
{
    QString buildNumber = "0";

#ifdef BUILD_NUMBER
    buildNumber = BUILD_NUMBER;
#endif

    aboutHtmlContent = fileToString(path).arg(VERSION, buildNumber);
    applyHtmlTheme();
}

void HelpAboutDialog::loadHelp(const QString &path)
{
    helpHtmlContent = fileToString(path);
    applyHtmlTheme();
}

QString HelpAboutDialog::fileToString(const QString &path)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QTextStream txtS(&f);

    txtS.setEncoding(QStringConverter::Utf8);

    QString content = txtS.readAll();
    f.close();

    return content;
}

void HelpAboutDialog::loadSystemInfo()
{
    auto text = YACReader::getGlobalInfo();

    text.append("\nGRAPHIC INFORMATION\n");
    text.append(QString("Screen pixel ratio: %1\n").arg(devicePixelRatioF()));

    systemInfoText->setText(text);
}

void HelpAboutDialog::applyTheme(const Theme &theme)
{
    Q_UNUSED(theme)
    applyHtmlTheme();
}

void HelpAboutDialog::applyHtmlTheme()
{
    auto helpTheme = theme.helpAboutDialog;

    // Original colors in the HTML CSS
    const QString originalHeadingColor = "#302f2d";
    const QString originalLinkColor = "#C19441";

    if (!aboutHtmlContent.isEmpty()) {
        QString themedAbout = aboutHtmlContent;
        themedAbout.replace(originalHeadingColor, helpTheme.headingColor.name(), Qt::CaseInsensitive);
        themedAbout.replace(originalLinkColor, helpTheme.linkColor.name(), Qt::CaseInsensitive);
        aboutText->setHtml(themedAbout);
        aboutText->moveCursor(QTextCursor::Start);
    }

    if (!helpHtmlContent.isEmpty()) {
        QString themedHelp = helpHtmlContent;
        themedHelp.replace(originalHeadingColor, helpTheme.headingColor.name(), Qt::CaseInsensitive);
        themedHelp.replace(originalLinkColor, helpTheme.linkColor.name(), Qt::CaseInsensitive);
        helpText->setHtml(themedHelp);
        helpText->moveCursor(QTextCursor::Start);
    }
}
