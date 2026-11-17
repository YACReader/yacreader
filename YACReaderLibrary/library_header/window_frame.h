#ifndef WINDOW_FRAME_H
#define WINDOW_FRAME_H

#include <QObject>

class QMainWindow;
class LibraryHeader;
struct Theme;

// How the header joins the top-level window. The base class is the
// conventional frame: the system draws the title bar and the header sits
// below it. Platform frames put the header in the caption row.
class WindowFrame : public QObject
{
    Q_OBJECT
public:
    // Call before the window is shown.
    static WindowFrame *create(QMainWindow *window, LibraryHeader *header);

    virtual void applyTheme(const Theme &theme);
    // Forwarded from QWidget::nativeEvent() of the window.
    virtual bool handleNativeEvent(const QByteArray &eventType, void *message, qintptr *result);

protected:
    WindowFrame(QMainWindow *window, LibraryHeader *header);

    QMainWindow *window;
    LibraryHeader *header;
};

// nullptr when the platform has no frame of its own.
WindowFrame *createPlatformWindowFrame(QMainWindow *window, LibraryHeader *header);

#endif // WINDOW_FRAME_H
