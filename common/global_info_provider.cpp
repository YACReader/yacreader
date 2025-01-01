#include "global_info_provider.h"

#include <QtCore>
#include <QImageReader>
#include <QPaintDevice>

#ifdef YACREADER_LIBRARY
#include <QSqlDatabase>
#endif

QString YACReader::getGlobalInfo()
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

    text.append("\nAPP INFORMATION\n");
    QString supportedImageFormats = QImageReader::supportedImageFormats().join(", ");
    text.append(QString("Image formats supported: %1\n").arg(supportedImageFormats));
    // append if sqlite driver is available
#ifdef YACREADER_LIBRARY
    text.append(QString("SQLite driver available: %1\n").arg(QSqlDatabase::isDriverAvailable("QSQLITE") ? "yes" : "no"));
#endif

#ifdef use_unarr
    text.append("Compression backend: unarr (no RAR5 support)\n");
#elif defined use_libarchive
    text.append("Compression backend: libarchive\n");
#else
    text.append("Compression backend: 7zip\n");
#endif

// print pdf backend used, poppler, pdfkit, pdfium
#ifdef NO_PDF
    text.append("PDF support: None\n");
#elif defined USE_PDFKIT
    text.append("PDF support: PDFKit\n");
#elif defined USE_PDFIUM
    text.append("PDF support: PDFium\n");
#else
    text.append("PDF support: Poppler\n");
#endif

    return text;
}
