#ifndef _7Z_INCLUDES_H
#define _7Z_INCLUDES_H

//WIN includes
#ifdef Q_OS_WIN
extern "C"
{
#include "lib7zip/C/Alloc.h"
#include "lib7zip/C/7zTypes.h"
}

#include "lib7zip/CPP/Common/Common.h"
#include "lib7zip/CPP/Common/MyWindows.h"


#include "lib7zip/CPP/Common/StringConvert.h"
#include "lib7zip/CPP/Common/MyInitGuid.h"
#include "lib7zip/CPP/Common/MyCom.h"
#include "lib7zip/CPP/7zip/Common/FileStreams.h"
#include "lib7zip/CPP/7zip/Archive/IArchive.h"

#include "lib7zip/CPP/7zip/IStream.h"

#include "lib7zip/CPP/7zip/IPassword.h"
#include "lib7zip/CPP/7zip/MyVersion.h"

#include "lib7zip/CPP/Windows/PropVariant.h"
#include "lib7zip/CPP/Windows/PropVariantConv.h"

#include "lib7zip/CPP/7zip/Common/StreamObjects.h"
#include "lib7zip/CPP/7zip/Common/StreamUtils.h"

#else
//POSIX includes

extern "C"
{
#include "libp7zip/C/Alloc.h"
#include "libp7zip/C/7zTypes.h"
}

#include "libp7zip/CPP/Windows/Defs.h"


#include "libp7zip/CPP/Common/Common.h"
#include "libp7zip/CPP/Common/MyCom.h"

//macos
//typedef int BOOL; ---> typedef signed char BOOL;
#include "libp7zip/CPP/include_windows/windows.h"
#include "libp7zip/CPP/include_windows/basetyps.h"
#include "libp7zip/CPP/include_windows/tchar.h"

#include "libp7zip/CPP/Common/MyWindows.h"

#include "libp7zip/CPP/Common/StringConvert.h"

#include "libp7zip/CPP/7zip/Archive/IArchive.h"

#include "libp7zip/CPP/7zip/IStream.h"
#include "libp7zip/CPP/7zip/IPassword.h"
#include "libp7zip/CPP/7zip/IProgress.h"

#include "libp7zip/CPP/7zip/Common/FileStreams.h"

#include "libp7zip/CPP/7zip/MyVersion.h"

#include "libp7zip/CPP/Windows/PropVariant.h"
#include "libp7zip/CPP/Windows/PropVariantConv.h"

#include "libp7zip/CPP/7zip/Common/StreamObjects.h"
#include "libp7zip/CPP/7zip/Common/StreamUtils.h"

#include "libp7zip/CPP/7zip/ICoder.h"


#endif

#endif // _7Z_INCLUDES_H
