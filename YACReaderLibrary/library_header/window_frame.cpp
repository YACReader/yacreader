#include "window_frame.h"

#include "library_header.h"

#include <QMainWindow>

WindowFrame::WindowFrame(QMainWindow *window, LibraryHeader *header)
    : QObject(window), window(window), header(header)
{
    // The system title bar shows the app icon and name; platform frames that
    // replace it show them again in the header.
    header->setAppIdentityVisible(false);
}

WindowFrame *WindowFrame::create(QMainWindow *window, LibraryHeader *header)
{
    if (WindowFrame *frame = createPlatformWindowFrame(window, header))
        return frame;
    return new WindowFrame(window, header);
}

void WindowFrame::setColors(const FrameColors &)
{
}

bool WindowFrame::handleNativeEvent(const QByteArray &, void *, qintptr *)
{
    return false;
}

#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
WindowFrame *createPlatformWindowFrame(QMainWindow *, LibraryHeader *)
{
    return nullptr;
}
#endif
