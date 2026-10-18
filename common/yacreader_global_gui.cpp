#include "yacreader_global_gui.h"

#include <QAction>
#include <QFileDialog>
#include <QImageReader>
#include <QtCore>

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

void YACReader::colorize(QImage &img, const QColor &col)
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

QList<ListItemId> YACReader::mimeDataToListEntries(const QMimeData *data)
{
    QList<QPair<int, qulonglong>> entries;
    QByteArray rawData = data->data(YACReader::YACReaderLibraryListEntriesMimeDataFormat);
    QDataStream in(&rawData, QIODevice::ReadOnly);
    in >> entries;

    QList<ListItemId> entryIds;
    entryIds.reserve(entries.size());
    for (const auto &[listKind, id] : std::as_const(entries))
        entryIds.append({ static_cast<ListKind>(listKind), id });
    return entryIds;
}

QList<ComicRowId> YACReader::mimeDataToRows(const QMimeData *data)
{
    QList<ComicRowId> rows;
    if (const auto entryIds = mimeDataToListEntries(data); !entryIds.isEmpty()) {
        for (const auto &entryId : entryIds)
            rows.append(entryId);
        return rows;
    }
    for (const auto id : mimeDataToComicsIds(data))
        rows.append(ComicId(id));
    return rows;
}

void YACReader::setListEntriesMimeData(QMimeData *data, const QList<ListItemId> &entryIds)
{
    QList<QPair<int, qulonglong>> entries;
    entries.reserve(entryIds.size());
    for (const auto &entryId : entryIds)
        entries.append({ static_cast<int>(entryId.listKind), entryId.id });

    QByteArray rawData;
    QDataStream out(&rawData, QIODevice::WriteOnly);
    out << entries;
    data->setData(YACReader::YACReaderLibraryListEntriesMimeDataFormat, rawData);
}

QAction *YACReader::wrappedToolbarAction(QAction *action)
{
    auto a = new QAction(action->text());

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

    // asign a to action somehow so we can retrieve it later
    action->setProperty("wrappedToolbarAction", QVariant::fromValue<QObject *>(a));

    return a;
}

QPixmap YACReader::hdpiPixmap(const QString &file, QSize size)
{
    return QIcon(file).pixmap(size);
}

QString YACReader::imageFileLoader(QWidget *parent)
{
    QString supportedImageFormatsString;
    const auto supportedImageFormats = QImageReader::supportedImageFormats();
    for (const QByteArray &format : supportedImageFormats) {
        supportedImageFormatsString += QString("*.%1 ").arg(QString(format));
    }

    return QFileDialog::getOpenFileName(parent, QObject::tr("Select custom cover"), QDir::homePath(), QObject::tr("Images (%1)").arg(supportedImageFormatsString));
}

QString YACReader::imagePathFromMimeData(const QMimeData *mimeData)
{
    QString filePath;

    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();

        if (!urlList.isEmpty()) {
            QUrl url = urlList.first();
            if (url.isLocalFile()) {
                filePath = url.toLocalFile();

                QFileInfo fileInfo(filePath);
                QString extension = fileInfo.suffix().toLower();
                const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
                bool isSupported = false;

                for (const QByteArray &format : supportedFormats) {
                    if (extension == QString(format).toLower()) {
                        isSupported = true;
                        break;
                    }
                }

                if (!isSupported) {
                    filePath.clear();
                }
            }
        }
    }

    return filePath;
}
