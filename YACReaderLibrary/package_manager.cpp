#include "package_manager.h"

#include <QCoreApplication>

PackageManager::PackageManager()
    : creating(false), _7z(nullptr)
{
}

void PackageManager::createPackage(const QString &libraryPath, const QString &dest)
{
    creating = true;
    QStringList attributes;
    attributes << "a"
               << "-y"
               << "-ttar" << dest + ".clc" << libraryPath;
    start7z(attributes);
}

void PackageManager::extractPackage(const QString &packagePath, const QString &destDir)
{
    creating = false;
    QStringList attributes;
    QString output = "-o";
    output += destDir;
    attributes << "x"
               << "-y" << output << packagePath;
    start7z(attributes);
}

void PackageManager::start7z(const QStringList &arguments)
{
    if (_7z != nullptr) {
        _7z->deleteLater();
    }

    _7z = new QProcess(this);
    connect(_7z, &QProcess::errorOccurred, this, &PackageManager::handleError);
    connect(_7z, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &PackageManager::handleFinished);
#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    _7z->start("7z", arguments); // TODO: use 7z.so
#else
    _7z->start(QCoreApplication::applicationDirPath() + "/utils/7zip", arguments); // TODO: use 7z.dll
#endif
}

void PackageManager::cancel()
{
    if (_7z != nullptr) {
        _7z->disconnect();
        _7z->kill();
        if (creating) {
            // TODO remove dest+".clc"
        } else {
            // TODO fixed: is done by libraryWindow
        }
    }
}

void PackageManager::handleFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        if (creating) {
            emit exported();
        } else {
            emit imported();
        }
        return;
    }

    emit failed(_7z != nullptr ? QString::fromLocal8Bit(_7z->readAllStandardError()).trimmed() : QString());
}

void PackageManager::handleError(QProcess::ProcessError error)
{
    Q_UNUSED(error)
    emit failed(_7z != nullptr ? _7z->errorString() : QString());
}
