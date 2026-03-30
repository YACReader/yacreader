#include "check_new_version.h"

#include "yacreader_global.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include <QVersionNumber>
#include <QtGlobal>

#define PREVIOUS_VERSION_TESTING "6.0.0"

HttpVersionChecker::HttpVersionChecker()
    : HttpWorker("https://raw.githubusercontent.com/YACReader/yacreader/master/VERSION", DEFAULT_USER_AGENT)
{
    connect(this, &HttpVersionChecker::dataReady, this, QOverload<const QByteArray &>::of(&HttpVersionChecker::checkNewVersion));
}

void HttpVersionChecker::checkNewVersion(const QByteArray &data)
{
    checkNewVersion(QString(data));
}

bool HttpVersionChecker::checkNewVersion(QString sourceContent)
{
#ifdef QT_DEBUG
    const auto currentVersion = QVersionNumber::fromString(PREVIOUS_VERSION_TESTING);
#else
    const auto currentVersion = QVersionNumber::fromString(QString::fromLatin1(VERSION));
#endif
    const auto latestVersion = QVersionNumber::fromString(sourceContent.trimmed());

    if (!currentVersion.isNull() && !latestVersion.isNull() && QVersionNumber::compare(latestVersion, currentVersion) > 0) {
        emit newVersionDetected();
        return true;
    }

    return false;
}
