#include "yacreader_global_gui.h"

#include "overloaded.h"

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

namespace {

// the kind of each row in the rows format
enum class RowKind {
    Comic,
    ReadingListEntry,
    LabelEntry,
    Placeholder,
};

}

QList<ComicRowId> YACReader::mimeDataToRows(const QMimeData *data)
{
    QList<QPair<int, qulonglong>> values;
    QByteArray rawData = data->data(YACReader::YACReaderLibraryComicRowsMimeDataFormat);
    QDataStream in(&rawData, QIODevice::ReadOnly);
    in >> values;

    QList<ComicRowId> rows;
    rows.reserve(values.size());
    for (const auto &[kind, id] : std::as_const(values)) {
        switch (static_cast<RowKind>(kind)) {
        case RowKind::Comic:
            rows.append(ComicId(id));
            break;
        case RowKind::ReadingListEntry:
            rows.append(ListItemId { ListKind::ReadingList, id });
            break;
        case RowKind::LabelEntry:
            rows.append(ListItemId { ListKind::Label, id });
            break;
        case RowKind::Placeholder:
            rows.append(PlaceholderId(id));
            break;
        }
    }
    return rows;
}

void YACReader::setRowsMimeData(QMimeData *data, const QList<ComicRowId> &rows)
{
    QList<QPair<int, qulonglong>> values;
    values.reserve(rows.size());
    for (const auto &row : rows) {
        values.append(std::visit(overloaded {
                                         [](const ComicId &comic) { return QPair<int, qulonglong>(int(RowKind::Comic), comic.value()); },
                                         [](const ListItemId &entry) { return QPair<int, qulonglong>(int(entry.listKind == ListKind::ReadingList ? RowKind::ReadingListEntry : RowKind::LabelEntry), entry.id); },
                                         [](const PlaceholderId &placeholder) { return QPair<int, qulonglong>(int(RowKind::Placeholder), placeholder.value()); },
                                 },
                                 row));
    }

    QByteArray rawData;
    QDataStream out(&rawData, QIODevice::WriteOnly);
    out << values;
    data->setData(YACReader::YACReaderLibraryComicRowsMimeDataFormat, rawData);
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
