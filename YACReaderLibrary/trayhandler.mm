
#include <QtGlobal>
#undef __OBJC_BOOL_IS_BOOL

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
