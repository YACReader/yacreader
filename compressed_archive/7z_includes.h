#ifndef _7Z_INCLUDES_H
#define _7Z_INCLUDES_H

//WIN includes
#ifdef Q_OS_WIN
#include "lib7zip/CPP/Common/StringConvert.h"
#include "lib7zip/CPP/Common/MyInitGuid.h"
#include "lib7zip/CPP/Common/MyCom.h"
#include "lib7zip/CPP/7zip/Common/FileStreams.h"
#include "lib7zip/CPP/7zip/Archive/IArchive.h"

#include "lib7zip/CPP/7zip/IStream.h"

#include "lib7zip/CPP/7zip/IPassword.h"
#include "lib7zip/CPP/7zip/MyVersion.h"

#include "lib7zip/C/Types.h"

#include "lib7zip/CPP/Windows/PropVariant.h"
#include "lib7zip/CPP/Windows/PropVariantConversions.h"

#include "lib7zip/CPP/7zip/Common/StreamObjects.h"
#include "lib7zip/CPP/7zip/Common/StreamUtils.h"

extern "C"
{
#include "lib7zip/C/Alloc.h"
}
#else
//POSIX includes
#include "libp7zip/CPP/myWindows/myPrivate.h"
#include "libp7zip/CPP/myWindows/config.h"

#include "libp7zip/CPP/Common/MyGuidDef.h"
#include "libp7zip/CPP/Common/MyWindows.h"

#include "libp7zip/CPP/Common/StringConvert.h"
#include "libp7zip/CPP/Common/MyInitGuid.h"
#include "libp7zip/CPP/Common/MyCom.h"
#include "libp7zip/CPP/7zip/Common/FileStreams.h"
#include "libp7zip/CPP/7zip/Archive/IArchive.h"

#include "libp7zip/CPP/7zip/IStream.h"

#include "libp7zip/CPP/7zip/IPassword.h"
#include "libp7zip/CPP/7zip/MyVersion.h"

#include "libp7zip/C/Types.h"

#include "libp7zip/CPP/Windows/Defs.h"
#include "libp7zip/CPP/Windows/PropVariant.h"
#include "libp7zip/CPP/Windows/PropVariantConversions.h"

#include "libp7zip/CPP/7zip/Common/StreamObjects.h"
#include "libp7zip/CPP/7zip/Common/StreamUtils.h"

extern "C"
{
#include "libp7zip/C/Alloc.h"
}
#endif

#endif // _7Z_INCLUDES_H
