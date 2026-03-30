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

    const auto currentVersion = QVersionNumber::fromString(QString::fromLatin1(VERSION));
    const auto trimmedSourceContent = sourceContent.trimmed();
    qsizetype suffixIndex = 0;
    const auto latestVersion = QVersionNumber::fromString(trimmedSourceContent, &suffixIndex);

    if (trimmedSourceContent.isEmpty() ||
        suffixIndex != trimmedSourceContent.size() ||
        latestVersion.segments().size() != 3) {
        return false;
    }

    if (!currentVersion.isNull() && !latestVersion.isNull() && QVersionNumber::compare(latestVersion, currentVersion) > 0) {
        emit newVersionDetected();
        return true;
    }

    return false;
}
