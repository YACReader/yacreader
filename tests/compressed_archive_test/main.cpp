
#include <QtCore/QCoreApplication>
#include <QDir>
#include <QElapsedTimer>

#include "compressed_archive.h"

#include <iostream>

using namespace std;

// This program uses PROTOS Genome Test Suite c10-archive [0] for testing the CompressedArchive wrapper files support
// It tests the following formats: RAR, ZIP, TAR
// Arter downloading c10-archive-r1.iso, open it and full extract RAR_TAR.BZ2, ZIP_TAR.BZ2, TAR_TAR.BZ2 files into a folder
// This program takes the path to that folder as an argument
//
//  [0] https://www.ee.oulu.fi/research/ouspg/PROTOS_Test-Suite_c10-archive#Download
//
int main(int argc, char *argv[])
{
    if (argc < 2) {
        cout << "Usage: compressed_archive_test PATH" << endl;
        return 0;
    }

    // QCoreApplication app(argc, argv);

    QString s(argv[1]);

    QStringList supportedFormats;
    supportedFormats << "rar"
                     << "zip"
                     << "tar";

    QElapsedTimer timer;
    timer.start();

    quint32 totalFiles = 0;
    foreach (QString format, supportedFormats) {
        QDir rootDir(s);
        if (!rootDir.cd(format)) {
            cout << "Folder for format '" << format.toStdString() << "' not found" << endl;
            continue;
        }
        rootDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

        QFileInfoList files = rootDir.entryInfoList();
        quint32 totalFormat = 0;
        quint32 errors = 0;
        quint64 init = timer.elapsed();

        foreach (QFileInfo fileInfo, files) {
            totalFiles++;
            totalFormat++;
            CompressedArchive archive(fileInfo.filePath());
            if (!archive.isValid())
                errors++;
            else {
                int i = archive.getNumFiles();
                cerr << i;
                QList<QString> filenames = archive.getFileNames();
                if (!filenames.isEmpty()) {
                    cerr << archive.getFileNames().at(0).toStdString();
                }
            }
        }
        quint64 end = timer.elapsed();

        cout << "Format '" << format.toStdString() << "'" << endl;
        cout << "Total files : " << totalFormat << endl;
        cout << "Errors : " << errors << endl;
        cout << "Elapsed time : " << (end - init) / 1000 << "s" << endl;
        cout << endl;
    }

    cout << endl;
    cout << "Total time : " << timer.elapsed() / 1000 << "s" << endl;
    cout << endl;

    // return app.exec();
}
