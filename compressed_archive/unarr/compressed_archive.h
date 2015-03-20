#ifndef COMPRESSED_ARCHIVE_H
#define COMPRESSED_ARCHIVE_H

#include <QObject>
#include "extract_delegate.h"
extern"C" {
#include "unarr.h"
}

class CompressedArchive : public QObject
{
	Q_OBJECT
public:
	explicit CompressedArchive(const QString & filePath, QObject *parent = 0);
	~CompressedArchive();

signals:
	
public slots:
	int getNumFiles();
	void getAllData(const QVector<quint32> & indexes, ExtractDelegate * delegate=0);
	QByteArray getRawDataAtIndex(int index);
	QList<QString> getFileNames();
	bool isValid();
	bool toolsLoaded();
private:

	bool loadFunctions();
	bool tools;
	bool valid;
	QList<QString> fileNames;
	int numFiles;
	ar_archive *ar;
	ar_stream *stream;
	QList<qint64> offsets;
};

#endif // COMPRESSED_ARCHIVE_H
