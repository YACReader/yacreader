#include "compressed_archive.h"

#define archive_error(msg) msg << ": [" << archive_errno(a) << "]" << archive_error_string(a)

CompressedArchive::CompressedArchive(const QString &filePath, QObject *parent)
    : QObject(parent), a(nullptr), num_entries(0), valid(false), idx(0), filename(filePath)
{
    if (!open_archive()) {
        qWarning() << "error opening archive:" << filename;
        return;
    }

    archive_entry *entry;
    int result;
    while ((result = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        entries.append(archive_entry_pathname(entry));
        archive_read_data_skip(a);
        idx++;
    }

    num_entries = entries.size();

    if (result != ARCHIVE_EOF) {
        qDebug() << "finished reading archive with result of:" << result;
        qWarning() << archive_error("error reading archive");
    } else if (num_entries == 0) {
        qWarning() << "no entries read from archive.";
    } else {
        qDebug() << "# of pages in archive:" << num_entries;
        valid = true;
    }

    close_archive();
}

CompressedArchive::~CompressedArchive()
{
    close_archive();
}

bool CompressedArchive::open_archive()
{
    qDebug() << "opening archive:" << filename;
    idx = 0;

    if (a != nullptr) {
        close_archive();
    }

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if (archive_read_open_filename(a, filename.toStdString().c_str(), 10240) != ARCHIVE_OK) {
        qWarning() << archive_error("error opening archive");
        close_archive();
        return false;
    }

    return true;
}

void CompressedArchive::close_archive()
{
    qDebug() << "closing archive.";
    archive_read_free(a);
    a = nullptr;
}

bool CompressedArchive::archive_seek(quint32 index)
{
    if (idx == index) {
        return true;
    }

    // libarchive uses a streaming architecture so we cannot read files before our current position.
    // because of this, when we need to seek to an index before our current position,
    // we must reopen the archive.
    if (idx > index) {
        qDebug() << "asked for index [" << index << "] less than position [" << idx << "]."
                 << "reopening archive.";
        close_archive();

        if (!open_archive()) {
            return false;
        }
    }

    qDebug() << "current pos = [" << idx << "] seeking to [" << index << "]";

    archive_entry *entry;
    for (; idx < index; idx++) {
        if (archive_read_next_header(a, &entry) != ARCHIVE_OK) {
            qWarning() << archive_error("error reading header");
            return false;
        }

        if (archive_read_data_skip(a) != ARCHIVE_OK) {
            qWarning() << archive_error("error skipping data");
            return false;
        }
    }
    return true;
}

void CompressedArchive::getAllData(const QVector<quint32> &indexes,
                                   ExtractDelegate *delegate)
{
    qDebug() << "called getAllData: [" << indexes << "]";
    if (indexes.isEmpty())
        return;

    for (int i = 0; i < indexes.count(); i++) {
        if (delegate == nullptr || delegate->isCancelled())
            return;

        quint32 index = indexes[i];
        QByteArray bytes = getRawDataAtIndex(index);
        if (bytes.size() > 0) {
            delegate->fileExtracted(index, bytes);
        } else {
            qWarning() << "getAllData error at index: [" << index << "]";
            delegate->unknownError(index);
            return;
        }
    }
}

QByteArray CompressedArchive::read_entry()
{
    QByteArray bytes;
    archive_entry *entry;

    if (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        int64_t size = archive_entry_size(entry);
        bytes.resize(size);
        archive_read_data(a, bytes.data(), size);
    } else {
        qWarning() << archive_error("error reading entry");
    }

    idx++;
    return bytes;
}

QByteArray CompressedArchive::getRawDataAtIndex(int index)
{
    QByteArray bytes;
    if (archive_seek(index)) {
        bytes = read_entry();
    } else {
        qWarning() << "error reading data from archive. index:" << index;
    }
    return bytes;
}
