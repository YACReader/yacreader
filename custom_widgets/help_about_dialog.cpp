#include "help_about_dialog.h"

#include "opengl_checker.h"
#include "global_info_provider.h"

#include <QtCore>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTextBrowser>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QScreen>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

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

    aboutText->setHtml(fileToString(path).arg(VERSION, buildNumber));
    aboutText->moveCursor(QTextCursor::Start);
}

void HelpAboutDialog::loadHelp(const QString &path)
{
    helpText->setHtml(fileToString(path));
    helpText->moveCursor(QTextCursor::Start);
}

QString HelpAboutDialog::fileToString(const QString &path)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QTextStream txtS(&f);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    txtS.setEncoding(QStringConverter::Utf8);
#else
    txtS.setCodec(QTextCodec::codecForName("UTF-8"));
#endif

    QString content = txtS.readAll();
    f.close();

    return content;
}

void HelpAboutDialog::loadSystemInfo()
{
    auto text = YACReader::getGlobalInfo();

    auto openGLChecker = OpenGLChecker();
    text.append("\nGRAPHIC INFORMATION\n");
    text.append(QString("Screen pixel ratio: %1\n").arg(devicePixelRatioF()));
    text.append(QString("OpenGL version: %1\n").arg(openGLChecker.textVersionDescription()));

    systemInfoText->setText(text);
}
