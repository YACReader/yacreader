#ifndef INITIALCOMICINFOEXTRACTOR_H
#define INITIALCOMICINFOEXTRACTOR_H

#include <QtGui>

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
    int _coverPage;
    int getXMLMetadata;
    static bool crash;
    QByteArray _xmlInfoData;
    void saveCover(const QString &path, const QImage &cover);

public slots:
    void extract();
    int getNumPages() { return _numPages; }
    QPixmap getCover() { return QPixmap::fromImage(_cover); }
    QPair<int, int> getOriginalCoverSize() { return _coverSize; }
    QByteArray getXMLInfoRawData();
signals:
    void openingError(QProcess::ProcessError error);
};
}

#endif // INITIALCOMICINFOEXTRACTOR_H
