#include "compressed_archive.h"

#include <QFileInfo>
#include <QDebug>

#include "extract_delegate.h"
#include <unarr.h>

CompressedArchive::CompressedArchive(const QString & filePath, QObject *parent) :
    QObject(parent),tools(true),valid(false),numFiles(0),ar(NULL),stream(NULL)
{
	//open file
  #ifdef Q_OS_WIN
  stream = ar_open_file_w((wchar_t *)filePath.utf16());
  #else
	stream = ar_open_file(filePath.toLocal8Bit().constData());
  #endif
	if (!stream)
	{
		return;
	}

	//open archive
	ar = ar_open_rar_archive(stream);
	//TODO: build unarr with 7z support and test this!
	//if (!ar) ar = ar_open_7z_archive(stream);
	if (!ar) ar = ar_open_tar_archive(stream);
	//zip detection is costly, so it comes last...
	if (!ar) ar = ar_open_zip_archive(stream, false);
	if (!ar)
	{
		return;
	}

	//initial parse
	while (ar_parse_entry(ar))
	{
		//make sure we really got a file header
		if (ar_entry_get_size(ar) > 0)
		{
			fileNames.append(ar_entry_get_name(ar));
			offsets.append(ar_entry_get_offset(ar));
			numFiles++;
		}
	}
	if (!ar_at_eof(ar))
	{
		//fail if the initial parse didn't reach EOF
		//this might be a bit too drastic
		qDebug() << "Error while parsing archive";
		return;
	}
	if (numFiles > 0)
	{
		valid = true;
	}
}

CompressedArchive::~CompressedArchive()
{
	ar_close_archive(ar);
	ar_close(stream);
}

QList<QString> CompressedArchive::getFileNames()
{
	return fileNames;
}

bool CompressedArchive::isValid()
{
	return valid;
}

bool CompressedArchive::toolsLoaded()
{
	//for backwards compatibilty
	return tools;
}

int CompressedArchive::getNumFiles()
{
	return numFiles;
}

void CompressedArchive::getAllData(const QVector<quint32> & indexes, ExtractDelegate * delegate)
{
	if (indexes.isEmpty())
		return;

	QByteArray buffer;

	int i=0;
	while (i < indexes.count())
	{
        if(delegate->isCancelled())
        {
            return;
        }

		//use the offset list so we generated so we're not getting any non-page files
		ar_parse_entry_at(ar, offsets.at(indexes.at(i))); //set ar_entry to start of indexes
		buffer.resize(ar_entry_get_size(ar));
		if (ar_entry_uncompress(ar, buffer.data(), buffer.size())) //did we extract it?
		{
			delegate->fileExtracted(indexes.at(i), buffer); //return extracted file
		}
		else
		{
			delegate->crcError(indexes.at(i)); 	//we could not extract it...
		}
		i++;
	}
}

QByteArray CompressedArchive::getRawDataAtIndex(int index)
{
	QByteArray buffer;
	if(index >= 0 && index < getNumFiles())
	{
		ar_parse_entry_at(ar, offsets.at(index));
		buffer.resize(ar_entry_get_size(ar));
		if(ar_entry_uncompress(ar, buffer.data(), buffer.size()))
		{
			return buffer;
		}
		else
		{
			return QByteArray();
		}
	}
    return buffer;
}
