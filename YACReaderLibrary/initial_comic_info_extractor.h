#ifndef INITIALCOMICINFOEXTRACTOR_H
#define INITIALCOMICINFOEXTRACTOR_H

#include <QImage>
#include <QObject>
#include <QPixmap>
#include <QProcess>
#include <QString>

namespace YACReader {
class InitialComicInfoExtractor : public QObject
{
    Q_OBJECT

public:
    InitialComicInfoExtractor(QString fileSource, QString target = "", int coverPage = 1, bool getXMLMetadata = false);

private:
    QString _fileSource;
    QString _target;
    QString _currentName;
    int _numPages;
    QPair<int, int> _coverSize;
    QImage _cover;
    bool _coverExtracted;
    int _coverPage;
    bool _fileSupported;
    int getXMLMetadata;
    static bool crash;
    QByteArray _xmlInfoData;
    void saveCover(const QString &path, const QImage &cover);

public slots:
    void extract();
    int getNumPages() { return _numPages; }
    QPixmap getCover() { return QPixmap::fromImage(_cover); }
    QImage getCoverImage() const { return _cover; }
    bool hasValidCover() const { return _coverExtracted; }
    // False when the file is not a comic YACReader can show, e.g. an EPUB book made of
    // text. Such a file has to be ignored, not stored as a comic without pages.
    bool isFileSupported() const { return _fileSupported; }
    QPair<int, int> getOriginalCoverSize() { return _coverSize; }
    QByteArray getXMLInfoRawData();
signals:
    void openingError(QProcess::ProcessError error);
};
}

#endif // INITIALCOMICINFOEXTRACTOR_H
