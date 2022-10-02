
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#undef __OBJC_BOOL_IS_BOOL
#endif

#import <AppKit/AppKit.h>
#include "trayhandler.h"

void OSXShowDockIcon()
{
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
}
void OSXHideDockIcon()
{
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
}
