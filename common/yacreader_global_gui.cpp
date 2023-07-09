#include "yacreader_global_gui.h"

#include <QtCore>
#include <QAction>

using namespace YACReader;

void YACReader::addSperator(QWidget *w)
{
    QAction *separator = new QAction(w);
    separator->setSeparator(true);
    w->addAction(separator);
}

QAction *YACReader::createSeparator()
{
    QAction *a = new QAction(0);
    a->setSeparator(true);
    return a;
}

QIcon YACReader::noHighlightedIcon(const QString &path)
{
    QPixmap sp(path);
    QPixmap p = hdpiPixmap(path, sp.size());

    QIcon icon;
    icon.addFile(path, p.size(), QIcon::Normal);
    icon.addFile(path, p.size(), QIcon::Selected);
    return icon;
}

void YACReader::colorize(QImage &img, QColor &col)
{
    QRgb *data = (QRgb *)img.bits();
    QRgb *end = data + img.width() * img.height();

    int rcol = col.red(), gcol = col.green(), bcol = col.blue();
    while (data != end) {
        *data = qRgba(rcol, gcol, bcol, qAlpha(*data));
        ++data;
    }
}

QList<qulonglong> YACReader::mimeDataToComicsIds(const QMimeData *data)
{
    QList<qulonglong> comicIds;
    QByteArray rawData = data->data(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat);
    QDataStream in(&rawData, QIODevice::ReadOnly);
    in >> comicIds; // deserialize the list of indentifiers
    return comicIds;
}

// TODO some SVG assets are missing in macos (WIP)
// we need two sets of icons, one for the toolbar and one for the context menu because of this bug (QTBUG-96553): https://bugreports.qt.io/browse/QTBUG-96553

QString YACReader::addExtensionToIconPath(const QString &path)
{
#ifdef YACREADER_LIBRARY
#ifdef Q_OS_MACOS // TODO_Y_MAC_UI
    return path + ".png";
#else
    return path + ".svg";
#endif
#else
    return path + ".svg";
#endif
}

QString YACReader::addExtensionToIconPathInToolbar(const QString &path)
{
    return path + "_18x18.svg";
}

QAction *YACReader::actionWithCustomIcon(const QIcon &icon, QAction *action)
{
    auto a = new QAction(icon, action->text());

    a->setEnabled(action->isEnabled());
    a->setCheckable(action->isCheckable());

    a->setChecked(action->isChecked());

    QObject::connect(a, &QAction::triggered, action, &QAction::triggered);

    QObject::connect(action, &QAction::changed, action, [=]() {
        a->setEnabled(action->isEnabled());
        a->setCheckable(action->isCheckable());

        a->setChecked(action->isChecked());
    });
    QObject::connect(a, &QAction::toggled, action, &QAction::setChecked);
    QObject::connect(action, &QAction::toggled, a, &QAction::setChecked);

    return a;
}

QPixmap YACReader::hdpiPixmap(const QString &file, QSize size)
{
    return QIcon(file).pixmap(size);
}
