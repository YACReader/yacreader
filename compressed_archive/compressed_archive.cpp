#include <QtCore>

#include "compressed_archive.h"
#include "extract_delegate.h"

#include <QLibrary>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

#include "open_callbacks.h"
#include "extract_callbacks.h"

#include "7z_includes.h"
#include "lib7zip/CPP/Common/MyInitGuid.h"

#define _MY_WINAPI WINAPI

typedef quint32(_MY_WINAPI *CreateObjectFunc)(const GUID *clsID, const GUID *interfaceID, void **outObject);
typedef quint32(_MY_WINAPI *GetMethodPropertyFunc)(quint32 index, PROPID propID, PROPVARIANT *value);
typedef quint32(_MY_WINAPI *GetNumberOfMethodsFunc)(quint32 *numMethods);
typedef quint32(_MY_WINAPI *GetNumberOfFormatsFunc)(quint32 *numFormats);
typedef quint32(_MY_WINAPI *GetHandlerPropertyFunc)(PROPID propID, PROPVARIANT *value);
typedef quint32(_MY_WINAPI *GetHandlerPropertyFunc2)(quint32 index, PROPID propID, PROPVARIANT *value);
typedef quint32(_MY_WINAPI *SetLargePageModeFunc)();

// DEFINE_GUID(CLSID_CFormat7z,0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
// DEFINE_GUID(IArchiveKK,0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00);

DEFINE_GUID(CLSID_CFormat7z, 0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatRar, 0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x03, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatRar5, 0X23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xCC, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatZip, 0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatTar, 0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0xee, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatArj, 0x23170f69, 0x40c1, 0x278a, 0x10, 0x00, 0x00, 0x01, 0x10, 0x04, 0x00, 0x00);

// unused Formats
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

GUID _supportedFileFormats[] = { CLSID_CFormatRar, CLSID_CFormatZip, CLSID_CFormatTar, CLSID_CFormat7z, CLSID_CFormatArj, CLSID_CFormatRar5 };
std::vector<GUID> supportedFileFormats(_supportedFileFormats, _supportedFileFormats + sizeof(_supportedFileFormats) / sizeof(_supportedFileFormats[0]));

DEFINE_GUID(IID_InArchive, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00);
DEFINE_GUID(IID_ISetCompressCodecsInfoX, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x04, 0x00, 0x61, 0x00, 0x00);

struct SevenZipInterface {
    CreateObjectFunc createObjectFunc;
    GetMethodPropertyFunc getMethodPropertyFunc;
    GetNumberOfMethodsFunc getNumberOfMethodsFunc;
    GetNumberOfFormatsFunc getNumberOfFormatsFunc;
    GetHandlerPropertyFunc getHandlerPropertyFunc;
    GetHandlerPropertyFunc2 getHandlerPropertyFunc2;
    SetLargePageModeFunc setLargePageModeFunc;
    CMyComPtr<IInArchive> archive;
};

// SevenZipInterface * szInterface;

const unsigned char rar[7] = { static_cast<unsigned char>(0x52), static_cast<unsigned char>(0x61), static_cast<unsigned char>(0x72), static_cast<unsigned char>(0x21), static_cast<unsigned char>(0x1A), static_cast<unsigned char>(0x07), static_cast<unsigned char>(0x00) };
const unsigned char rar5[8] = { static_cast<unsigned char>(0x52), static_cast<unsigned char>(0x61), static_cast<unsigned char>(0x72), static_cast<unsigned char>(0x21), static_cast<unsigned char>(0x1A), static_cast<unsigned char>(0x07), static_cast<unsigned char>(0x01), static_cast<unsigned char>(0x00) };
const unsigned char zip[2] = { static_cast<unsigned char>(0x50), static_cast<unsigned char>(0x4B) };
const unsigned char sevenz[6] = { static_cast<unsigned char>(0x37), static_cast<unsigned char>(0x7A), static_cast<unsigned char>(0xBC), static_cast<unsigned char>(0xAF), static_cast<unsigned char>(0x27), static_cast<unsigned char>(0x1C) };
const unsigned char tar[6] = "ustar";
const unsigned char arj[2] = { static_cast<unsigned char>(0x60), static_cast<unsigned char>(0xEA) };

CompressedArchive::CompressedArchive(const QString &filePath, QObject *parent)
    : QObject(parent), sevenzLib(0), valid(false), tools(false)
{
    szInterface = new SevenZipInterface;
    // load functions
    if (!loadFunctions())
        return;

    tools = true;
    // load file
    if (szInterface->createObjectFunc != 0) {
        // QUuid CLSID_CFormat7z("23170f69-40c1-278a-1000-000110070000");
        // se crea el objeto Archivo: formato,tipo,objeto
        bool formatFound = false;
        CInFileStream *fileSpec = new CInFileStream;
        CMyComPtr<IInStream> file = fileSpec;

        YCArchiveOpenCallback *openCallbackSpec = new YCArchiveOpenCallback;
        CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
        openCallbackSpec->PasswordIsDefined = false;

        // get file type from suffix
        int i = -1;
        QFile filex(filePath);

        if (!filex.open(QIODevice::ReadOnly))
            return;
        QByteArray magicNumber = filex.read(8); // read first 8 bytes
        if (memcmp(magicNumber, rar, 6) == 0)
            if (memcmp(magicNumber, rar5, 7) == 0)
                i = 5;
            else
                i = 0;
        else if (memcmp(magicNumber, zip, 2) == 0)
            i = 1;
        else if (memcmp(magicNumber, sevenz, 6) == 0)
            i = 3;
        else if (memcmp(magicNumber, arj, 2) == 0)
            i = 4;
        else {
            filex.seek(257);
            magicNumber = filex.read(8);
            if (memcmp(magicNumber, tar, 5) == 0)
                i = 2;
        }
        if (i == -1) // fallback code
        {
            QFileInfo fileinfo(filePath);
            if (fileinfo.suffix() == "zip" || fileinfo.suffix() == "cbz") {
                i = 1;
            } else {
                return;
            }
        }

#ifdef USE_UNICODE_FSTRING
        if (!fileSpec->Open((LPCTSTR)filePath.toStdWString().c_str()))
#else
        if (!fileSpec->Open((LPCTSTR)filePath.toStdString().c_str()))
#endif
        {
            qDebug() << "unable to load" + filePath;
            return;
        }

        // GUID uuid = supportedFileFormats[i];
        // qDebug() << "trying : " << uuid << endl;
        if (szInterface->createObjectFunc(&supportedFileFormats[i], &IID_InArchive, (void **)&szInterface->archive) == S_OK) {
            // qDebug() << "Can not open archive file : " + filePath << Qt::endl;

            if (szInterface->archive->Open(file, 0, openCallback) == S_OK) {
                valid = formatFound = true;
                qDebug() << "Opened archive file : " + filePath << Qt::endl;
                setupFilesNames();
                return;
            }
        }
        if (!formatFound) {
            qDebug() << "Can not open archive" << Qt::endl;
        }
    }
}

CompressedArchive::~CompressedArchive()
{
    // always close the archive!
    if (szInterface->archive) {
        szInterface->archive->Close();
    }

    delete szInterface;
    delete sevenzLib;
}

bool CompressedArchive::loadFunctions()
{
    // LOAD library
    if (sevenzLib == 0) {
#if defined Q_OS_UNIX && !defined Q_OS_MACOS
        QFileInfo sevenzlibrary(QString(LIBDIR) + "/yacreader/7z.so");
        if (sevenzlibrary.exists()) {
            sevenzLib = new QLibrary(sevenzlibrary.absoluteFilePath());
        } else {
            sevenzLib = new QLibrary(QString(LIBDIR) + "/7zip/7z.so");
        }
#else
        sevenzLib = new QLibrary(QCoreApplication::applicationDirPath() + "/utils/7z");
#endif
    }
    if (!sevenzLib->load()) {
        qDebug() << "Error Loading 7z.dll : " + sevenzLib->errorString() << Qt::endl;
        QCoreApplication::exit(700); // TODO yacreader_global can't be used here, it is GUI dependant, YACReader::SevenZNotFound
        return false;
    } else {
        qDebug() << "Loading functions" << Qt::endl;

        if ((szInterface->createObjectFunc = (CreateObjectFunc)sevenzLib->resolve("CreateObject")) == 0)
            qDebug() << "fail loading function : CreateObject" << Qt::endl;
        if ((szInterface->getMethodPropertyFunc = (GetMethodPropertyFunc)sevenzLib->resolve("GetMethodProperty")) == 0)
            qDebug() << "fail loading function : GetMethodProperty" << Qt::endl;
        if ((szInterface->getNumberOfMethodsFunc = (GetNumberOfMethodsFunc)sevenzLib->resolve("GetNumberOfMethods")) == 0)
            qDebug() << "fail loading function : GetNumberOfMethods" << Qt::endl;
        if ((szInterface->getNumberOfFormatsFunc = (GetNumberOfFormatsFunc)sevenzLib->resolve("GetNumberOfFormats")) == 0)
            qDebug() << "fail loading function : GetNumberOfFormats" << Qt::endl;
        if ((szInterface->getHandlerPropertyFunc = (GetHandlerPropertyFunc)sevenzLib->resolve("GetHandlerProperty")) == 0)
            qDebug() << "fail loading function : GetHandlerProperty" << Qt::endl;
        if ((szInterface->getHandlerPropertyFunc2 = (GetHandlerPropertyFunc2)sevenzLib->resolve("GetHandlerProperty2")) == 0)
            qDebug() << "fail loading function : GetHandlerProperty2" << Qt::endl;
        if ((szInterface->setLargePageModeFunc = (SetLargePageModeFunc)sevenzLib->resolve("SetLargePageMode")) == 0)
            qDebug() << "fail loading function : SetLargePageMode" << Qt::endl;
    }

    return true;
}

void CompressedArchive::setupFilesNames()
{
    quint32 numItems = getNumEntries();
    quint32 p = 0;
    for (quint32 i = 0; i < numItems; i++) {

        // Get name of file
        NWindows::NCOM::CPropVariant prop;
        szInterface->archive->GetProperty(i, kpidIsDir, &prop);
        bool isDir;
        if (prop.vt == VT_BOOL)
            isDir = VARIANT_BOOLToBool(prop.boolVal);
        else if (prop.vt == VT_EMPTY)
            isDir = false;
        else
            continue;

        if (!isDir) {
            szInterface->archive->GetProperty(i, kpidPath, &prop);
            UString s = prop.bstrVal;
            const wchar_t *chars = s.operator const wchar_t *();
            files.append(QString::fromWCharArray(chars));
            offsets.append(i);
            indexesToPages.insert(i, p);
            p++;
        }
    }
}

QVector<quint32> CompressedArchive::translateIndexes(const QVector<quint32> &indexes)
{
    QVector<quint32> translatedIndexes;

    foreach (quint32 i, indexes) {
        if (i < (quint32)offsets.length())
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

QList<QByteArray> CompressedArchive::getAllData(const QVector<quint32> &indexes, ExtractDelegate *delegate)
{
    YCArchiveExtractCallback *extractCallbackSpec = new YCArchiveExtractCallback(indexesToPages, true, delegate);
    CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    extractCallbackSpec->Init(szInterface->archive, L""); // second parameter is output folder path
    extractCallbackSpec->PasswordIsDefined = false;

    QVector<quint32> currentIndexes = translateIndexes(indexes);

    HRESULT result;
    if (indexes.isEmpty())
        result = szInterface->archive->Extract(NULL, -1, false, extractCallback);
    else
        result = szInterface->archive->Extract(currentIndexes.data(), currentIndexes.count(), false, extractCallback);
    if (result != S_OK) {
        qDebug() << "Extract Error" << Qt::endl;
    }

    return extractCallbackSpec->allFiles;
}

QByteArray CompressedArchive::getRawDataAtIndex(int index)
{
    if (index >= 0 && index < getNumFiles()) {
        YCArchiveExtractCallback *extractCallbackSpec = new YCArchiveExtractCallback(indexesToPages);
        CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
        extractCallbackSpec->Init(szInterface->archive, L""); // second parameter is output folder path
        extractCallbackSpec->PasswordIsDefined = false;

        UInt32 indices[1];

        if (index < offsets.length())
            indices[0] = offsets.at(index);
        else
            indices[0] = index;

        HRESULT result = szInterface->archive->Extract(indices, 1, false, extractCallback);
        if (result != S_OK) {
            qDebug() << "Extract Error" << Qt::endl;
        }

        return QByteArray((char *)extractCallbackSpec->data, extractCallbackSpec->newFileSize);
    }
    return QByteArray();
}
