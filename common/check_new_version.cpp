#include "check_new_version.h"

#include <QUrl>
#include <QtGlobal>
#include <QStringList>

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>

#define PREVIOUS_VERSION_TESTING "6.0.0"

HttpVersionChecker::HttpVersionChecker()
    : HttpWorker("https://raw.githubusercontent.com/YACReader/yacreader/master/common/yacreader_global.h")
{
    connect(this, &HttpVersionChecker::dataReady, this, QOverload<const QByteArray &>::of(&HttpVersionChecker::checkNewVersion));
}

void HttpVersionChecker::checkNewVersion(const QByteArray &data)
{
    checkNewVersion(QString(data));
}

bool HttpVersionChecker::checkNewVersion(QString sourceContent)
{
    QRegExp rx("#define VERSION \"([0-9]+).([0-9]+).([0-9]+)\"");

    int index = 0;
    bool newVersion = false;
    bool sameVersion = true;
    // bool currentVersionIsNewer = false;
#ifdef QT_DEBUG
    QString version(PREVIOUS_VERSION_TESTING);
#else
    QString version(VERSION);
#endif
    QStringList sl = version.split(".");
    if ((index = rx.indexIn(sourceContent)) != -1) {
        int length = qMin(sl.size(), (rx.cap(4) != "") ? 4 : 3);
        for (int i = 0; i < length; i++) {
            if (rx.cap(i + 1).toInt() < sl.at(i).toInt()) {
                return false;
            }
            if (rx.cap(i + 1).toInt() > sl.at(i).toInt()) {
                newVersion = true;
                break;
            } else
                sameVersion = sameVersion && rx.cap(i + 1).toInt() == sl.at(i).toInt();
        }
        if (!newVersion && sameVersion) {
            if ((sl.size() == 3) && (rx.cap(4) != ""))
                newVersion = true;
        }
    }

    if (newVersion == true) {
        emit newVersionDetected();
        return true;
    } else {
        return false;
    }
}
