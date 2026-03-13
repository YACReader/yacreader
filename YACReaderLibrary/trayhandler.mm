
#include <QtGlobal>
#undef __OBJC_BOOL_IS_BOOL

#include "trayhandler.h"

#import <AppKit/AppKit.h>

void OSXShowDockIcon()
{
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
}
void OSXHideDockIcon()
{
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
}
