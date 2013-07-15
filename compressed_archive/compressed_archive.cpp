#include "compressed_archive.h"
#include "extract_delegate.h"

#include <QLibrary>
#include <QDebug>


#include "open_callbacks.h"
#include "extract_callbacks.h"

//DEFINE_GUID(CLSID_CFormat7z,0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
//DEFINE_GUID(IArchiveKK,0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00);        

DEFINE_GUID(CLSID_CFormat7z,      0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatRar,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x03, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatZip,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatTar,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xee, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatArj,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x04, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatBZip2,   0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x02, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatCab,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x08, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatChm,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xe9, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatCompound,0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xe5, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatCpio,    0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xed, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatDeb,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xec, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatGZip,    0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xef, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatIso,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xe7, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatLzh,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x06, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatLzma,    0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0a, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatNsis,    0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x09, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatRpm,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xeb, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatSplit,   0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xea, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatWim,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xe6, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatZ,       0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x05, 0x00, 0x00);

GUID _supportedFileFormats[] = {CLSID_CFormatRar,CLSID_CFormatZip,CLSID_CFormatTar,CLSID_CFormat7z,CLSID_CFormatArj};
std::vector<GUID> supportedFileFormats (_supportedFileFormats, _supportedFileFormats + sizeof(_supportedFileFormats) / sizeof(_supportedFileFormats[0]) );

DEFINE_GUID(IID_InArchive,        0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00);

struct SevenZipInterface {
	CreateObjectFunc createObjectFunc;
	GetMethodPropertyFunc getMethodPropertyFunc;
	GetNumberOfMethodsFunc getNumberOfMethodsFunc;
	GetNumberOfFormatsFunc getNumberOfFormatsFunc;
	GetHandlerPropertyFunc getHandlerPropertyFunc;
	GetHandlerPropertyFunc2 getHandlerPropertyFunc2;
	SetLargePageModeFunc setLargePageModeFunc;

	CMyComPtr<IInArchive> archive;

	//---
	CInFileStream *fileSpec;
	CMyComPtr<IInStream> file;
};

//SevenZipInterface * szInterface;
QLibrary * sevenzLib = 0;

CompressedArchive::CompressedArchive(const QString & filePath, QObject *parent) :
	QObject(parent)
{

	szInterface = new SevenZipInterface;
	//load functions
	loadFunctions();

	//load file
	if(szInterface->createObjectFunc != 0)
	{
		//QUuid CLSID_CFormat7z("23170f69-40c1-278a-1000-000110070000");
		//se crea el objeto Archivo: formato,tipo,objeto
		bool formatFound = false;
		CInFileStream *fileSpec = new CInFileStream;
		CMyComPtr<IInStream> file = fileSpec;

		CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback;
		CMyComPtr<IArchiveOpenCallback> openCallback = openCallbackSpec;
		openCallbackSpec->PasswordIsDefined = false;
		// openCallbackSpec->PasswordIsDefined = true;
		// openCallbackSpec->Password = L"1";

		for(unsigned int i=0;i<supportedFileFormats.size();i++)
		{
			GUID uuid = supportedFileFormats[i];
			//qDebug() << "trying : " << uuid << endl;
			if (szInterface->createObjectFunc(&supportedFileFormats[i], &IID_InArchive, (void **)&szInterface->archive) != S_OK)
				continue;

			if (!fileSpec->Open((LPCTSTR)filePath.toStdWString().data()))
				continue;
			//qDebug() << "Can not open archive file : " + filePath << endl;	

			if (szInterface->archive->Open(file, 0, openCallback) == S_OK)
			{
				formatFound = true;
				break;
			}
		}
		if(!formatFound)
			qDebug() << "Can not open archive" << endl;
	}
}

CompressedArchive::~CompressedArchive()
{
	//TODO
}

void CompressedArchive::loadFunctions()
{
	//LOAD library
	//TODO check if this works in OSX (7z.so instead of 7z.dylib)
	// fix1: try to load "7z.so"
	// fix2: rename 7z.so to 7z.dylib
	if(sevenzLib == 0)
		sevenzLib = new QLibrary("./utils/7z");
	if(!sevenzLib->load())
		qDebug() << "Loading 7z.dll : " + sevenzLib->errorString() << endl;
	else
	{
		qDebug() << "Loading functions" << endl;

		if((szInterface->createObjectFunc = (CreateObjectFunc)sevenzLib->resolve("CreateObject")) == 0)
			qDebug() << "fail loading function : CreateObject" << endl;
		if((szInterface->getMethodPropertyFunc = (GetMethodPropertyFunc)sevenzLib->resolve("GetMethodProperty")) == 0)
			qDebug() << "fail loading function : GetMethodProperty" << endl;
		if((szInterface->getNumberOfMethodsFunc = (GetNumberOfMethodsFunc)sevenzLib->resolve("GetNumberOfMethods")) == 0)
			qDebug() << "fail loading function : GetNumberOfMethods" << endl;
		if((szInterface->getNumberOfFormatsFunc = (GetNumberOfFormatsFunc)sevenzLib->resolve("GetNumberOfFormats")) == 0)
			qDebug() << "fail loading function : GetNumberOfFormats" << endl;
		if((szInterface->getHandlerPropertyFunc = (GetHandlerPropertyFunc)sevenzLib->resolve("GetHandlerProperty")) == 0)
			qDebug() << "fail loading function : GetHandlerProperty" << endl;
		if((szInterface->getHandlerPropertyFunc2 = (GetHandlerPropertyFunc2)sevenzLib->resolve("GetHandlerProperty2")) == 0)
			qDebug() << "fail loading function : GetHandlerProperty2" << endl;
		if((szInterface->setLargePageModeFunc = (SetLargePageModeFunc)sevenzLib->resolve("SetLargePageMode")) == 0)
			qDebug() << "fail loading function : SetLargePageMode" << endl;
	}
}

QList<QString> CompressedArchive::getFileNames()
{
	QList<QString> files;
	quint32 numItems = getNumFiles();
	for (quint32 i = 0; i < numItems; i++)
	{
		{
			// Get name of file
			NWindows::NCOM::CPropVariant prop;
			/*szInterface->archive->GetProperty(i, kpidIsDir, &prop);
			bool isDir;
			if (prop.vt == VT_BOOL)
				isDir = VARIANT_BOOLToBool(prop.boolVal);
			else if (prop.vt == VT_EMPTY)
				isDir = false;

			if(!isDir)
			{*/
				szInterface->archive->GetProperty(i, kpidPath, &prop);
				UString s = ConvertPropVariantToString(prop);
				const wchar_t * chars = s.operator const wchar_t *();
				files.append(QString::fromWCharArray(chars));
			//}
		}
	}
	return files;
}

int CompressedArchive::getNumFiles()
{
	quint32 numItems = 0;
    szInterface->archive->GetNumberOfItems(&numItems);
	return numItems;
}
QList<QByteArray> CompressedArchive::getAllData(const QVector<quint32> & indexes, ExtractDelegate * delegate)
{
	CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback(true,delegate);
	CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
	extractCallbackSpec->Init(szInterface->archive, L""); // second parameter is output folder path
	extractCallbackSpec->PasswordIsDefined = false;

	HRESULT result;
	if(indexes.isEmpty())
		result = szInterface->archive->Extract(NULL, -1, false, extractCallback);
	else
		result = szInterface->archive->Extract(indexes.data(), indexes.count(), false, extractCallback);
	if (result != S_OK)
	{
		qDebug() << "Extract Error" << endl;
	}

	return extractCallbackSpec->allFiles;
}

QByteArray CompressedArchive::getRawDataAtIndex(int index)
{
	if(index>=0 && index < getNumFiles())
	{
		CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback;
		CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
		extractCallbackSpec->Init(szInterface->archive, L""); // second parameter is output folder path
		extractCallbackSpec->PasswordIsDefined = false;

		UInt32 indices[1];
		indices[0] = index;
		HRESULT result = szInterface->archive->Extract(indices, 1, false, extractCallback);
		if (result != S_OK)
		{
			qDebug() << "Extract Error" << endl;
		}

		QByteArray rawData((char *)extractCallbackSpec->data,extractCallbackSpec->newFileSize);

		return rawData;
	}
	return QByteArray();
}