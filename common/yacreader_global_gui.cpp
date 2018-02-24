#include "yacreader_global_gui.h"

#include <QtCore>
#include <QAction>

using namespace YACReader;

void YACReader::addSperator(QWidget *w)
{
    QAction * separator = new QAction(w);
    separator->setSeparator(true);
    w->addAction(separator);
}

QAction * YACReader::createSeparator()
{
    QAction * a = new QAction(0);
    a->setSeparator(true);
    return a;
}

QIcon YACReader::noHighlightedIcon(const QString &path)
{
    QPixmap p(path);

    QIcon icon;//(path);
    icon.addFile(path,p.size(),QIcon::Normal);
    icon.addFile(path,p.size(),QIcon::Selected);
    return icon;
}

void YACReader::colorize(QImage &img, QColor &col)
{
    QRgb *data = (QRgb *)img.bits();
    QRgb *end = data + img.width()*img.height();

    int rcol = col.red(), gcol = col.green(), bcol = col.blue();
    while(data != end) {
            *data = qRgba(rcol,gcol,bcol,qAlpha(*data));
        ++data;
    }
}

QList<qulonglong> YACReader::mimeDataToComicsIds(const QMimeData *data)
{
    QList<qulonglong> comicIds;
    QByteArray rawData = data->data(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat);
    QDataStream in(&rawData,QIODevice::ReadOnly);
    in  >> comicIds; //deserialize the list of indentifiers
    return comicIds;
}
