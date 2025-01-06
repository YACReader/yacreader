#include "library_comic_opener.h"

#include "comic_db.h"

#include <QtCore>
#include <QtWidgets>

bool YACReader::openComic(const ComicDB &comic,
                          unsigned long long libraryId,
                          const QString &path,
                          OpenComicSource source)
{
    bool yacreaderFound = false;

    QString labelParam;

    if (source.source == OpenComicSource::ReadingList) {
        labelParam = QString("--readingListId=%1").arg(source.sourceId);
    }

#ifdef Q_OS_MACOS
    QStringList possiblePaths { QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../../") };
    possiblePaths += QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

    for (auto &&ypath : possiblePaths) {
        QString yacreaderPath = QDir::cleanPath(ypath + "/YACReader.app");
        if (QFileInfo(yacreaderPath).exists()) {
            yacreaderFound = true;
            QStringList parameters { "-n", yacreaderPath, "--args", path, QString("--comicId=%1").arg(comic.id), QString("--libraryId=%1").arg(libraryId), labelParam };
            QProcess::startDetached("open", parameters);
            break;
        }
    }
#endif

#ifdef Q_OS_WIN
    QString executablePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/YACReader.exe");
    QStringList parameters { path, QString("--comicId=%1").arg(comic.id), QString("--libraryId=%1").arg(libraryId), labelParam };
    yacreaderFound = QProcess::startDetached(executablePath, parameters, QCoreApplication::applicationDirPath());
#endif

#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    QStringList parameters { path, QString("--comicId=%1").arg(comic.id), QString("--libraryId=%1").arg(libraryId), labelParam };
    yacreaderFound = QProcess::startDetached(QStringLiteral("YACReader"), parameters);
#endif

    return yacreaderFound;
}

QStringList parseCommand(const QString &input)
{
    QRegularExpression regex(R"((?:\"([^\"]*)\")|(\S+))");
    QStringList result;
    auto it = regex.globalMatch(input);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        if (match.hasMatch()) {
            result << (match.captured(1).isEmpty() ? match.captured(2) : match.captured(1));
        }
    }
    return result;
}

bool YACReader::openComicInThirdPartyApp(const QString &command, const QString &path)
{
    QStringList parsed = parseCommand(command);
    if (parsed.isEmpty()) {
        qDebug() << "Empty command";
        return false;
    }

    QString program = parsed.takeFirst();
    QStringList rawArguments = parsed;
    QStringList arguments;

    auto placeholderFound = false;
    for (auto argument : rawArguments) {
        if (argument.contains("{comic_file_path}")) {
            placeholderFound = true;
            arguments << argument.replace("{comic_file_path}", path);
        } else {
            arguments << argument;
        }
    }

    if (!placeholderFound) {
        arguments << path;
    }

    return QProcess::startDetached(program, arguments);
}
