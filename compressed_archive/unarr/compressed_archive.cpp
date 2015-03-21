#include "compressed_archive.h"

#include <QFileInfo>
#include <QDebug>
#include <QApplication>

#include "yacreader_global.h"
#include "extract_delegate.h"

extern"C" {
#include "unarr.h"
}

CompressedArchive::CompressedArchive(const QString & filePath, QObject *parent) :
    QObject(parent),valid(false),tools(true),numFiles(0),ar(NULL),stream(NULL)
{
	stream = ar_open_file(filePath.toStdString().c_str());
	//try to open archive
	ar = ar_open_rar_archive(stream);
	if (!ar) ar = ar_open_zip_archive(stream, false);
	//if (!ar) ar = ar_open_7z_archive(stream);
	if (!ar) ar = ar_open_tar_archive(stream);
	if (!ar)
	{
		return;
	}
	//initial parse
	while (ar_parse_entry(ar)) 
	{
		numFiles++;
		fileNames.append(ar_entry_get_name(ar));
		offsets.append(ar_entry_get_offset(ar));
	}
	if (numFiles > 0)
	{
		valid = true;
		tools = true;
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
		if (i==0)
		{
			ar_parse_entry_at(ar, offsets.at(indexes.at(0))); //set ar_entry to start of indexes
		}
		else
		{
			ar_parse_entry(ar);
		}
		buffer.resize(ar_entry_get_size(ar));
		ar_entry_uncompress(ar, buffer.data(), buffer.size());
		delegate->fileExtracted(indexes.at(i), buffer); //return extracted files :)
		i++;
	}
}

QByteArray CompressedArchive::getRawDataAtIndex(int index)
{
	QByteArray buffer;
	if(index >= 0 && index < getNumFiles())
	{
		ar_parse_entry_at(ar, offsets.at(index));
		while (ar_entry_get_size(ar)==0)
		{
			ar_parse_entry(ar);
		}
		buffer.resize(ar_entry_get_size(ar));
		ar_entry_uncompress(ar, buffer.data(), buffer.size());
		//return buffer;
	}
    return buffer;
}
