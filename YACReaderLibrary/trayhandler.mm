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