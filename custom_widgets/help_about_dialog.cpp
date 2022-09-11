#include "help_about_dialog.h"

#include "opengl_checker.h"

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
    QString text;

    text.append("SYSTEM INFORMATION\n");
    text.append(QString("Qt version: %1\n").arg(qVersion()));
    text.append(QString("Build ABI: %1\n").arg(QSysInfo::buildAbi()));
    text.append(QString("build CPU architecture: %1\n").arg(QSysInfo::buildCpuArchitecture()));
    text.append(QString("CPU architecture: %1\n").arg(QSysInfo::currentCpuArchitecture()));
    text.append(QString("Kernel type: %1\n").arg(QSysInfo::kernelType()));
    text.append(QString("Kernel version: %1\n").arg(QSysInfo::kernelVersion()));
    text.append(QString("Product info: %1\n").arg(QSysInfo::prettyProductName()));

    //    QProcess systemInfoProcess;
    //    QString tempOutput;

    //    if (QSysInfo::kernelType() == "winnt") {
    //        QString cpuname = "wmic cpu get name";
    //        systemInfoProcess.start("cmd", QList<QString>() << "/C" << cpuname);
    //        systemInfoProcess.waitForFinished();
    //        tempOutput = QString(systemInfoProcess.readAllStandardOutput()).replace("\n", " ");
    //        text.append(QString("CPU: %1\n").arg(tempOutput));
    //    }

    //    if (QSysInfo::kernelType() == "linux") {
    //        QString cpuname = "cat /proc/cpuinfo | grep 'model name' | uniq";
    //        systemInfoProcess.start("bash", QList<QString>() << "-c" << cpuname);
    //        systemInfoProcess.waitForFinished();
    //        tempOutput = systemInfoProcess.readAllStandardOutput();
    //        text.append(QString("CPU: %1\n").arg(tempOutput));
    //    }

    auto openGLChecker = OpenGLChecker();
    text.append("\nGRAPHIC INFORMATION\n");
    text.append(QString("Screen pixel ratio: %1\n").arg(devicePixelRatioF()));
    text.append(QString("OpenGL version: %1\n").arg(openGLChecker.textVersionDescription()));

    //    if (QSysInfo::kernelType() == "winnt") {
    //        QString gpu = "wmic PATH Win32_videocontroller get VideoProcessor";
    //        systemInfoProcess.start("cmd", QList<QString>() << "/C" << gpu);
    //        systemInfoProcess.waitForFinished();
    //        tempOutput = systemInfoProcess.readAllStandardOutput();
    //        text.append(QString("GPU: %1\n").arg(tempOutput));

    //        QString gpuram = "wmic PATH Win32_VideoController get AdapterRAM";
    //        systemInfoProcess.start("cmd", QList<QString>() << "/C" << gpuram);
    //        systemInfoProcess.waitForFinished();
    //        tempOutput = systemInfoProcess.readAllStandardOutput();
    //        text.append(QString("GPU RAM: %1\n").arg(tempOutput));
    //    }

    systemInfoText->setText(text);
}
