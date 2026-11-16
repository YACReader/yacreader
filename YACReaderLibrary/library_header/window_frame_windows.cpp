#include "caption_buttons.h"
#include "library_header.h"
#include "window_frame.h"

#include <QCursor>
#include <QEvent>
#include <QMainWindow>
#include <QTimer>
#include <QtMath>
#include <qt_windows.h>

#include <dwmapi.h>
#include <windowsx.h>

namespace {

int resizeBorder(HWND hwnd)
{
    const UINT dpi = GetDpiForWindow(hwnd);
    return GetSystemMetricsForDpi(SM_CYSIZEFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
}

bool hasCaptionStyle(HWND hwnd)
{
    return (GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_CAPTION) == WS_CAPTION;
}

CaptionButtons::Button buttonForHit(WPARAM hit)
{
    switch (hit) {
    case HTMINBUTTON:
        return CaptionButtons::Minimize;
    case HTMAXBUTTON:
        return CaptionButtons::Maximize;
    case HTCLOSE:
        return CaptionButtons::Close;
    default:
        return CaptionButtons::None;
    }
}

QColor withAlpha(QColor color, int alpha)
{
    color.setAlpha(alpha);
    return color;
}

// The window keeps its native frame (WS_OVERLAPPEDWINDOW): shadow, resize
// borders, snap, animations and the system menu stay with Windows. Only the
// caption row becomes client area, where the header is drawn.
class WindowsWindowFrame : public WindowFrame
{
public:
    WindowsWindowFrame(QMainWindow *window, LibraryHeader *header)
        : WindowFrame(window, header)
    {
        // A child of the window, not of the header: the buttons stay on the
        // pages without a header (no libraries, import).
        buttons = new CaptionButtons(window);
        header->setTrailingInset(buttons->width());
        header->setAppIdentityVisible(true);
        hoverPoll = new QTimer(this);
        hoverPoll->setInterval(50);
        connect(hoverPoll, &QTimer::timeout, this, [this] { pollStickyHover(); });
        hwnd = reinterpret_cast<HWND>(window->winId());
        window->installEventFilter(this);
    }

    void setColors(const FrameColors &frameColors) override
    {
        const QColor foreground = frameColors.headerForeground;
        CaptionButtonColors colors;
        colors.foreground = foreground;
        colors.inactiveForeground = withAlpha(foreground, 0x80);
        colors.hover = withAlpha(foreground, 0x1A);
        colors.pressed = withAlpha(foreground, 0x33);
        buttons->setColors(colors);
        // Dark resize borders and system menu on dark headers.
        const BOOL dark = frameColors.headerBackground.lightnessF() < 0.5;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
    }

    bool handleNativeEvent(const QByteArray &, void *message, qintptr *result) override
    {
        const MSG *msg = static_cast<MSG *>(message);
        if (msg->hwnd != hwnd || !hasCaptionStyle(hwnd))
            return false;

        switch (msg->message) {
        case WM_NCCALCSIZE: {
            if (!msg->wParam)
                return false;
            auto *params = reinterpret_cast<NCCALCSIZE_PARAMS *>(msg->lParam);
            const LONG top = params->rgrc[0].top;
            *result = DefWindowProcW(hwnd, msg->message, msg->wParam, msg->lParam);
            // Keep native side and bottom borders; only the caption is removed.
            params->rgrc[0].top = top;
            // A maximized window hangs its frame off screen.
            if (IsZoomed(hwnd))
                params->rgrc[0].top += resizeBorder(hwnd);
            return true;
        }
        case WM_NCHITTEST:
            *result = hitTest(msg->lParam);
            return true;
        case WM_SIZE:
        case WM_DPICHANGED:
        case WM_ACTIVATE:
            QTimer::singleShot(0, this, [this] { updateMetrics(); });
            return false;
        default:
            break;
        }

        return handleCaptionButtonMessage(msg, result);
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == window) {
            switch (event->type()) {
            case QEvent::Show:
                // Sent before the native window becomes visible, once the
                // window can route WM_NCCALCSIZE to this frame.
                SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                updateMetrics();
                break;
            case QEvent::Resize:
                placeButtons();
                break;
            case QEvent::WindowStateChange:
                QTimer::singleShot(0, this, [this] { updateMetrics(); });
                break;
            case QEvent::ActivationChange:
                buttons->setWindowActive(window->isActiveWindow());
                break;
            default:
                break;
            }
        }
        return WindowFrame::eventFilter(watched, event);
    }

private:
    qreal scale() const { return window->devicePixelRatioF(); }

    void placeButtons()
    {
        buttons->setGeometry(window->width() - buttons->width(), 0, buttons->width(), header->height());
        buttons->raise();
    }

    void updateMetrics()
    {
        const bool custom = hasCaptionStyle(hwnd);
        buttons->setVisible(custom);
        buttons->setMaximized(IsZoomed(hwnd));
        header->setTrailingInset(custom ? buttons->width() : 0);
        placeButtons();
    }

    LRESULT hitTest(LPARAM lParam) const
    {
        const LRESULT native = DefWindowProcW(hwnd, WM_NCHITTEST, 0, lParam);
        if (native >= HTLEFT && native <= HTBOTTOMRIGHT)
            return native;

        POINT client { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd, &client);
        const qreal r = scale();
        const QPoint pos(qFloor(client.x / r), qFloor(client.y / r));
        const bool maximized = IsZoomed(hwnd);

        if (buttons->isVisible()) {
            const CaptionButtons::Button button = buttons->buttonAt(buttons->mapFrom(window, pos));
            if (button != CaptionButtons::None) {
                if (!maximized && client.y < qMax(1, qRound(r)))
                    return HTTOP;
                return button == CaptionButtons::Minimize ? HTMINBUTTON : button == CaptionButtons::Maximize ? HTMAXBUTTON
                                                                                                             : HTCLOSE;
            }
        }

        if (!maximized && client.y < resizeBorder(hwnd))
            return HTTOP;

        switch (header->hitTest(pos)) {
        case LibraryHeader::Hit::Caption:
            return HTCAPTION;
        case LibraryHeader::Hit::Client:
            return HTCLIENT;
        case LibraryHeader::Hit::Outside:
            // Pages without a header keep a draggable caption row.
            return pos.y() < header->height() ? HTCAPTION : HTCLIENT;
        }
        return HTCLIENT;
    }

    // Same message handling as Windows Terminal's custom caption: hover and
    // press come as non-client messages, DefWindowProc never sees a press on
    // a button (it would draw classic buttons), and actions go through
    // WM_SYSCOMMAND so Windows runs its own animations.
    bool handleCaptionButtonMessage(const MSG *msg, qintptr *result)
    {
        const CaptionButtons::Button button = buttonForHit(msg->wParam);
        switch (msg->message) {
        case WM_NCMOUSEMOVE:
            hoverPoll->stop();
            buttons->setHovered(button);
            if (button != CaptionButtons::None && !trackingNcMouse) {
                TRACKMOUSEEVENT track { sizeof(TRACKMOUSEEVENT), TME_LEAVE | TME_NONCLIENT, hwnd, HOVER_DEFAULT };
                TrackMouseEvent(&track);
                trackingNcMouse = true;
            }
            return false;
        case WM_NCMOUSELEAVE:
            trackingNcMouse = false;
            // The snap layouts flyout takes the pointer while it stays over
            // maximize; native buttons keep their hover state meanwhile.
            if (cursorOverMaximize()) {
                hoverPoll->start();
            } else {
                buttons->setHovered(CaptionButtons::None);
                buttons->setPressed(CaptionButtons::None);
            }
            return false;
        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONDBLCLK:
            if (button == CaptionButtons::None)
                return false;
            buttons->setHovered(button);
            buttons->setPressed(button);
            *result = 0;
            return true;
        case WM_NCLBUTTONUP: {
            const CaptionButtons::Button pressed = buttons->pressed();
            buttons->setPressed(CaptionButtons::None);
            if (button == CaptionButtons::None)
                return false;
            if (pressed == button)
                clickButton(button);
            *result = 0;
            return true;
        }
        default:
            return false;
        }
    }

    bool cursorOverMaximize() const
    {
        const QPoint local = buttons->mapFromGlobal(QCursor::pos());
        return buttons->buttonAt(local) == CaptionButtons::Maximize;
    }

    void pollStickyHover()
    {
        if (cursorOverMaximize())
            return;
        hoverPoll->stop();
        buttons->setHovered(CaptionButtons::None);
        buttons->setPressed(CaptionButtons::None);
    }

    void clickButton(CaptionButtons::Button button)
    {
        WPARAM command = SC_CLOSE;
        if (button == CaptionButtons::Minimize)
            command = SC_MINIMIZE;
        else if (button == CaptionButtons::Maximize)
            command = IsZoomed(hwnd) ? SC_RESTORE : SC_MAXIMIZE;
        PostMessageW(hwnd, WM_SYSCOMMAND, command, 0);
    }

    HWND hwnd = nullptr;
    CaptionButtons *buttons = nullptr;
    QTimer *hoverPoll = nullptr;
    bool trackingNcMouse = false;
};

} // namespace

WindowFrame *createPlatformWindowFrame(QMainWindow *window, LibraryHeader *header)
{
    return new WindowsWindowFrame(window, header);
}
