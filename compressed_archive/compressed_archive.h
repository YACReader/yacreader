#ifndef COMPRESSED_ARCHIVE_H
#define COMPRESSED_ARCHIVE_H

#include <QObject>

#ifdef Q_OS_UNIX
    #include "libp7zip/CPP/7zip/ICoder.h"
    #include "libp7zip/CPP/Common/MyCom.h"
#endif

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

class MyCodecs;

#ifdef Q_OS_UNIX
    class CompressedArchive : public QObject, public ICompressCodecsInfo, public CMyUnknownImp
#else
    class CompressedArchive : public QObject
#endif
{
	Q_OBJECT
public:
	explicit CompressedArchive(const QString & filePath, QObject *parent = 0);
	~CompressedArchive();

#ifdef Q_OS_UNIX
    MY_UNKNOWN_IMP

    STDMETHOD(GetNumberOfMethods)(UInt32 *numMethods);
    STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
    STDMETHOD(CreateDecoder)(UInt32 index, const GUID *iid, void **coder);
    STDMETHOD(CreateEncoder)(UInt32 index, const GUID *iid, void **coder);

    bool isRar;
#endif

signals:
	
public slots:
	int getNumFiles();
	QList<QByteArray> getAllData(const QVector<quint32> & indexes, ExtractDelegate * delegate = 0);
	QByteArray getRawDataAtIndex(int index);
	QList<QString> getFileNames();
	bool isValid();
	bool toolsLoaded();
private:
	SevenZipInterface * szInterface;

	QLibrary * sevenzLib;
#ifdef Q_OS_UNIX
    QLibrary * rarLib;
#endif
	bool loadFunctions();
	bool tools;
	bool valid;
	
    friend class MyCodecs;
};

#endif // COMPRESSED_ARCHIVE_H
