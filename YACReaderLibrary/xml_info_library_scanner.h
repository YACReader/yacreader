#ifndef XMLINFOLIBRARYSCANNER_H
#define XMLINFOLIBRARYSCANNER_H

#include <QtCore>

namespace YACReader {

class XMLInfoLibraryScanner : public QThread
{
    Q_OBJECT
public:
    XMLInfoLibraryScanner();
    void scanLibrary(const QString &source, const QString &target);

protected:
    void run() override;

public slots:
    void stop();

signals:
    void comicScanned(QString, QString);

private:
    QString source;
    QString target;
    bool stopRunning;
};

}

#endif // XMLINFOLIBRARYSCANNER_H
