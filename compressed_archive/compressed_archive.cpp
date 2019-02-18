#include <QtCore>
#ifndef Q_OS_WIN
#include "libp7zip/CPP/Common/MyInitGuid.h"
#endif

#include "compressed_archive.h"
#include "extract_delegate.h"

#include <QLibrary>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

#include "open_callbacks.h"
#include "extract_callbacks.h"

#include "7z_includes.h"

#ifdef Q_OS_WIN
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

//DEFINE_GUID(CLSID_CFormat7z,0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
//DEFINE_GUID(IArchiveKK,0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00);        

DEFINE_GUID(CLSID_CFormat7z,      0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatRar,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x03, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatRar5,    0X23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xCC, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatZip,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatTar,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xee, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatArj,     0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x04, 0x00, 0x00);

//unused Formats
/*DEFINE_GUID(CLSID_CFormatBZip2,   0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x02, 0x00, 0x00);
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
DEFINE_GUID(CLSID_CFormatZ,       0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x05, 0x00, 0x00);*/

#ifdef Q_OS_WIN
GUID _supportedFileFormats[] = {CLSID_CFormatRar,CLSID_CFormatZip,CLSID_CFormatTar,CLSID_CFormat7z,CLSID_CFormatArj,CLSID_CFormatRar5};
#else
GUID _supportedFileFormats[] = {CLSID_CFormatZip,CLSID_CFormatTar,CLSID_CFormat7z,CLSID_CFormatArj};
#endif
std::vector<GUID> supportedFileFormats (_supportedFileFormats, _supportedFileFormats + sizeof(_supportedFileFormats) / sizeof(_supportedFileFormats[0]) );

DEFINE_GUID(IID_InArchive,                0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00);
DEFINE_GUID(IID_ISetCompressCodecsInfoX,   0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x04, 0x00, 0x61, 0x00, 0x00);

/*#ifdef Q_OS_UNIX
DEFINE_GUID(IID_IOutStream,   0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00);
DEFINE_GUID(IID_IInStream,   0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00);
DEFINE_GUID(IID_IStreamGetSize,   0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00);
DEFINE_GUID(IID_ISequentialInStream,   0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00);
#endif*/

struct SevenZipInterface {
	CreateObjectFunc createObjectFunc;
	GetMethodPropertyFunc getMethodPropertyFunc;
	GetNumberOfMethodsFunc getNumberOfMethodsFunc;
	GetNumberOfFormatsFunc getNumberOfFormatsFunc;
	GetHandlerPropertyFunc getHandlerPropertyFunc;
	GetHandlerPropertyFunc2 getHandlerPropertyFunc2;
	SetLargePageModeFunc setLargePageModeFunc;

#ifdef Q_OS_UNIX
    CreateObjectFunc createObjectFuncRar;
    GetMethodPropertyFunc getMethodPropertyFuncRar;
    GetNumberOfMethodsFunc getNumberOfMethodsFuncRar;
#endif

	CMyComPtr<IInArchive> archive;
};

//SevenZipInterface * szInterface;

const unsigned char rar[7]={static_cast<unsigned char>(0x52), static_cast<unsigned char>(0x61), static_cast<unsigned char>(0x72), static_cast<unsigned char>(0x21), static_cast<unsigned char>(0x1A), static_cast<unsigned char>(0x07), static_cast<unsigned char>(0x00)};
const unsigned char rar5[8]={static_cast<unsigned char>(0x52), static_cast<unsigned char>(0x61), static_cast<unsigned char>(0x72), static_cast<unsigned char>(0x21), static_cast<unsigned char>(0x1A), static_cast<unsigned char>(0x07), static_cast<unsigned char>(0x01), static_cast<unsigned char>(0x00)};
const unsigned char zip[2]={static_cast<unsigned char>(0x50), static_cast<unsigned char>(0x4B)};
const unsigned char sevenz[6]={static_cast<unsigned char>(0x37), static_cast<unsigned char>(0x7A), static_cast<unsigned char>(0xBC), static_cast<unsigned char>(0xAF), static_cast<unsigned char>(0x27), static_cast<unsigned char>(0x1C)};
const unsigned char tar[6]="ustar";
const unsigned char arj[2]={static_cast<unsigned char>(0x60), static_cast<unsigned char>(0xEA)};

CompressedArchive::CompressedArchive(const QString & filePath, QObject *parent) :
    QObject(parent),sevenzLib(0),valid(false),tools(false)
#ifdef Q_OS_UNIX
  ,isRar(false)
#endif
{
	szInterface = new SevenZipInterface;
	//load functions
	if(!loadFunctions())
		return;

	tools = true;
	//load file
	if(szInterface->createObjectFunc != 0)
	{
		//QUuid CLSID_CFormat7z("23170f69-40c1-278a-1000-000110070000");
		//se crea el objeto Archivo: formato,tipo,objeto
		bool formatFound = false;
		CInFileStream *fileSpec = new CInFileStream;
		CMyComPtr<IInStream> file = fileSpec;

        YCArchiveOpenCallback *openCallbackSpec = new YCArchiveOpenCallback;
		CMyComPtr<IArchiveOpenCallback> openCallback = openCallbackSpec;
		openCallbackSpec->PasswordIsDefined = false;
		// openCallbackSpec->PasswordIsDefined = true;
		// openCallbackSpec->Password = L"1";
		
		//get file type from suffix
		int i=-1;
		QFile filex(filePath);
		
		if (!filex.open(QIODevice::ReadOnly))
			return;
		QByteArray magicNumber=filex.read(8); //read first 8 bytes
		
		//if (memcmp(magicNumber,rar5,8)==0)
			//return; //rar5 is not supported
		//qDebug() << memcmp(magicNumber,rar,7);
		//TODO: this suffix matching is rather primitive - better approach?
#ifdef Q_OS_UNIX
		if (memcmp(magicNumber,rar,6) != 0)
		{
			//match suffix to GUID list
			if (memcmp(magicNumber,zip,2)==0)
				i=0;
			else if (memcmp(magicNumber,sevenz,6)==0)
				i=2;
			else if (memcmp(magicNumber,arj,2)==0)
				i=3;
			else 
				{
					filex.seek(257);
					magicNumber=filex.read(8);
					if (memcmp(magicNumber,tar,5)==0)
						i=1;
				}
			if (i==-1) //fallback code
			{
				QFileInfo fileinfo(filePath);
				if (fileinfo.suffix() == "zip" || fileinfo.suffix() == "cbz")
				{
					i=0;
				}
				else
				{
					return;
				}
			}
#else
		if (memcmp(magicNumber,rar,6) == 0)
			if (memcmp(magicNumber,rar5,7) == 0)
                i=5;
			else
				i=0;
		else if (memcmp(magicNumber,zip,2)==0)
			i=1;
		else if (memcmp(magicNumber,sevenz,6)==0)
			i=3;
		else if (memcmp(magicNumber,arj,2)==0)
			i=4;
		else {
				filex.seek(257);
				magicNumber=filex.read(8);
				if (memcmp(magicNumber,tar,5)==0)
				i=2;
			}
		if (i==-1) //fallback code
			{
				QFileInfo fileinfo(filePath);
				if (fileinfo.suffix() == "zip" || fileinfo.suffix() == "cbz")
				{
					i=1;
				}
				else
				{
					return;
				}
			}
#endif

#ifdef UNICODE
		if (!fileSpec->Open((LPCTSTR)filePath.toStdWString().c_str()))
#else
		if (!fileSpec->Open((LPCTSTR)filePath.toStdString().c_str()))
#endif
		{
			qDebug() << "unable to load" + filePath;
			return;
		}
		
			//GUID uuid = supportedFileFormats[i];
			//qDebug() << "trying : " << uuid << endl;
		if (szInterface->createObjectFunc(&supportedFileFormats[i], &IID_InArchive, (void **)&szInterface->archive) == S_OK)
		{
		//qDebug() << "Can not open archive file : " + filePath << endl;	

			if (szInterface->archive->Open(file, 0, openCallback) == S_OK)
				{
					valid = formatFound = true;
					qDebug() << "Opened archive file : " + filePath << endl;
                    setupFilesNames();
					return;
				}
		}
			
			
	
#ifdef Q_OS_WIN
		if(!formatFound)
		{
			qDebug() << "Can not open archive" << endl;
		}
	}
}
#else
	}
	else
	{
        //RAR in macos and unix
        GUID clsidRar;
        if (memcmp(magicNumber,rar5,7) == 0) {
            clsidRar = CLSID_CFormatRar5;
        } else {
            clsidRar = CLSID_CFormatRar;
        }

	    isRar=true; //tell the destructor we *tried* to open a rar file!
            if (szInterface->createObjectFunc(&clsidRar, &IID_InArchive, (void **)&szInterface->archive) != S_OK)
            {
                qDebug() << "Error creating rar archive :" + filePath;
                return;
            }
       	    
	    CMyComPtr<ISetCompressCodecsInfo> codecsInfo;

            if (szInterface->archive->QueryInterface(IID_ISetCompressCodecsInfoX,(void **)&codecsInfo) != S_OK)
            {
                qDebug() << "Error getting rar codec :" + filePath;
                return;
            }
            if (codecsInfo->SetCompressCodecsInfo(this)  != S_OK)
            {
                qDebug() << "Error setting rar codec";
		return;
            }

#ifdef UNICODE
            if (!fileSpec->Open((LPCTSTR)filePath.toStdWString().c_str()))
#else
            if (!fileSpec->Open((LPCTSTR)filePath.toStdString().c_str()))
#endif
            {
                qDebug() << "Error opening rar file :" + filePath;
                return;
            }
            //qDebug() << "Can not open archive file : " + filePath << endl;

            if (szInterface->archive->Open(file, 0, openCallback) == S_OK)
            {
                valid = formatFound = true;
                setupFilesNames();
                //isRar = true;
            }
        }
	}
}
#endif


CompressedArchive::~CompressedArchive()
{
    //always close the archive!
    if (szInterface->archive)
    {
        szInterface->archive->Close();
    }

#ifdef Q_OS_UNIX
    if(isRar) //TODO: Memory leak!!!! If AddRef is not used, a crash occurs in "delete szInterface"
    {
        szInterface->archive->AddRef();
    }
#endif
    delete szInterface;

#ifdef Q_OS_UNIX
    delete rarLib;
#endif
    delete sevenzLib;
}

bool CompressedArchive::loadFunctions()
{
    //LOAD library
    //TODO check if this works in OSX (7z.so instead of 7z.dylib)
    // fix1: try to load "7z.so"
    // fix2: rename 7z.so to 7z.dylib
    if(sevenzLib == 0)
    {
#if defined Q_OS_UNIX
    #if defined Q_OS_MAC
        rarLib = new QLibrary(QCoreApplication::applicationDirPath()+"/utils/Codecs/Rar");
    #else
        //check if a yacreader specific version of p7zip exists on the system
        QFileInfo rarCodec(QString(LIBDIR)+"/yacreader/Codecs/Rar.so");
        if (rarCodec.exists())
        {
            rarLib = new QLibrary(rarCodec.absoluteFilePath());
        }
        else
        {
            rarLib = new QLibrary(QString(LIBDIR)+"/p7zip/Codecs/Rar.so");
        }
    #endif
        if(!rarLib->load())
        {
            qDebug() << "Error Loading Rar.so : " + rarLib->errorString() << endl;
            QCoreApplication::exit(700); //TODO yacreader_global can't be used here, it is GUI dependant, YACReader::SevenZNotFound
            return false;
        }
#endif
#if defined Q_OS_UNIX && !defined Q_OS_MAC
    QFileInfo sevenzlibrary(QString(LIBDIR)+"/yacreader/7z.so");
    if (sevenzlibrary.exists())
    {
        sevenzLib = new QLibrary(sevenzlibrary.absoluteFilePath());
    }
    else
    {
        sevenzLib = new QLibrary(QString(LIBDIR)+"/p7zip/7z.so");
    }
#else
    sevenzLib = new QLibrary(QCoreApplication::applicationDirPath()+"/utils/7z");
#endif
    }
    if(!sevenzLib->load())
    {
        qDebug() << "Error Loading 7z.dll : " + sevenzLib->errorString() << endl;
        QCoreApplication::exit(700); //TODO yacreader_global can't be used here, it is GUI dependant, YACReader::SevenZNotFound
        return false;
    }
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

#ifdef Q_OS_UNIX
        if((szInterface->createObjectFuncRar = (CreateObjectFunc)rarLib->resolve("CreateObject")) == 0)
            qDebug() << "fail loading function (rar) : CreateObject" << endl;
        if((szInterface->getMethodPropertyFuncRar = (GetMethodPropertyFunc)rarLib->resolve("GetMethodProperty")) == 0)
            qDebug() << "fail loading function (rar) : GetMethodProperty" << endl;
        if((szInterface->getNumberOfMethodsFuncRar = (GetNumberOfMethodsFunc)rarLib->resolve("GetNumberOfMethods")) == 0)
            qDebug() << "fail loading function (rar) : GetNumberOfMethods" << endl;
#endif
    }

    return true;
}

void CompressedArchive::setupFilesNames()
{
    quint32 numItems = getNumEntries();
    quint32 p = 0;
    for (quint32 i = 0; i < numItems; i++)
    {

        // Get name of file
        NWindows::NCOM::CPropVariant prop;
        szInterface->archive->GetProperty(i, kpidIsDir, &prop);
        bool isDir;
        if (prop.vt == VT_BOOL)
            isDir = VARIANT_BOOLToBool(prop.boolVal);
        else if (prop.vt == VT_EMPTY)
            isDir = false;

        if(!isDir)
        {
            szInterface->archive->GetProperty(i, kpidPath, &prop);
            UString s = prop.bstrVal;
            const wchar_t * chars = s.operator const wchar_t *();
            files.append(QString::fromWCharArray(chars));
            offsets.append(i);
            indexesToPages.insert(i,p);
            p++;
        }

    }
}

QVector<quint32> CompressedArchive::translateIndexes(const QVector<quint32> & indexes)
{
    QVector<quint32> translatedIndexes;

    foreach(quint32 i, indexes)
    {
        if(i < (quint32)offsets.length())
            translatedIndexes.append(offsets.at(i));
    }

    return translatedIndexes;
}

QList<QString> CompressedArchive::getFileNames()
{
    return files;
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
    return files.length();
}

int CompressedArchive::getNumEntries()
{
    quint32 numItems = 0;
    szInterface->archive->GetNumberOfItems(&numItems);
    return numItems;
}

QList<QByteArray> CompressedArchive::getAllData(const QVector<quint32> & indexes, ExtractDelegate * delegate)
{
    YCArchiveExtractCallback *extractCallbackSpec = new YCArchiveExtractCallback(indexesToPages, true, delegate);
    CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    extractCallbackSpec->Init(szInterface->archive, L""); // second parameter is output folder path
    extractCallbackSpec->PasswordIsDefined = false;

    QVector<quint32> currentIndexes = translateIndexes(indexes);

    HRESULT result;
    if(indexes.isEmpty())
        result = szInterface->archive->Extract(NULL, -1, false, extractCallback);
    else
        result = szInterface->archive->Extract(currentIndexes.data(), currentIndexes.count(), false, extractCallback);
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
        YCArchiveExtractCallback *extractCallbackSpec = new YCArchiveExtractCallback(indexesToPages);
        CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
        extractCallbackSpec->Init(szInterface->archive, L""); // second parameter is output folder path
        extractCallbackSpec->PasswordIsDefined = false;

        UInt32 indices[1];

        if(index < offsets.length())
            indices[0] = offsets.at(index);
        else
            indices[0] = index;

        HRESULT result = szInterface->archive->Extract(indices, 1, false, extractCallback);
        if (result != S_OK)
        {
            qDebug() << "Extract Error" << endl;
        }

        return QByteArray((char *)extractCallbackSpec->data,extractCallbackSpec->newFileSize);
    }
    return QByteArray();
}

#ifdef Q_OS_UNIX

STDMETHODIMP CompressedArchive::GetNumMethods(UInt32 *numMethods)
{
    return szInterface->getNumberOfMethodsFuncRar(numMethods);
}

STDMETHODIMP CompressedArchive::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
    return  szInterface->getMethodPropertyFuncRar(index,propID,value);
}

int i = 0;
STDMETHODIMP CompressedArchive::CreateDecoder(UInt32 index, const GUID *interfaceID, void **coder)
{
    NCOM::CPropVariant propVariant;
    szInterface->getMethodPropertyFuncRar(index,NMethodPropID::kDecoder,&propVariant);
    return szInterface->createObjectFuncRar((const GUID *)propVariant.bstrVal,interfaceID,coder);
}

STDMETHODIMP CompressedArchive::CreateEncoder(UInt32 index, const GUID *interfaceID, void **coder)
{
    return S_OK;//szInterface->createObjectFuncRar(&CLSID_CFormatRar,interfaceID,coder);
}

#endif
