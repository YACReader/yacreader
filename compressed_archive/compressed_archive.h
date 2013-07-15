#ifndef COMPRESSED_ARCHIVE_H
#define COMPRESSED_ARCHIVE_H

#include <QObject>

class ExtractDelegate;

#ifdef Q_OS_WIN
	#include "7z_includes.h"
	#define _MY_WINAPI WINAPI
#else
	#define _MY_WINAPI
#endif

typedef quint32 (_MY_WINAPI * CreateObjectFunc)(const GUID *clsID,const GUID *interfaceID,void **outObject);
typedef quint32 (_MY_WINAPI *GetMethodPropertyFunc)(quint32 index, PROPID propID, PROPVARIANT *value);
typedef quint32 (_MY_WINAPI *GetNumberOfMethodsFunc)(quint32 *numMethods);
typedef quint32 (_MY_WINAPI *GetNumberOfFormatsFunc)(quint32 *numFormats);
typedef quint32 (_MY_WINAPI *GetHandlerPropertyFunc)(PROPID propID, PROPVARIANT *value);
typedef quint32 (_MY_WINAPI *GetHandlerPropertyFunc2)(quint32 index, PROPID propID, PROPVARIANT *value);
typedef quint32 (_MY_WINAPI *SetLargePageModeFunc)();

class QLibrary;
#include <QString>
#include <QList>

struct SevenZipInterface;

class CompressedArchive : public QObject
{
    Q_OBJECT
public:
    explicit CompressedArchive(const QString & filePath, QObject *parent = 0);
	~CompressedArchive();
    
signals:
    
public slots:
	int getNumFiles();
	QList<QByteArray> getAllData(const QVector<quint32> & indexes, ExtractDelegate * delegate = 0);
	QByteArray getRawDataAtIndex(int index);
	QList<QString> getFileNames();
private:
	SevenZipInterface * szInterface;

	void loadFunctions();
    
};

#endif // COMPRESSED_ARCHIVE_H
