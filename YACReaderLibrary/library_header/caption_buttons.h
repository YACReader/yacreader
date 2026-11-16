#ifndef CAPTION_BUTTONS_H
#define CAPTION_BUTTONS_H

#include <QWidget>

class QVariantAnimation;

struct CaptionButtonColors {
    QColor foreground;
    QColor inactiveForeground;
    QColor hover;
    QColor pressed;
    QColor closeHover { 0xC4, 0x2B, 0x1C };
    QColor closePressed { 0xC4, 0x2B, 0x1C, 0x98 };
    QColor closeForeground { Qt::white };
};

// Paint-only minimize / maximize / close buttons for Windows. Input arrives as
// non-client messages (HTMINBUTTON, HTMAXBUTTON, HTCLOSE) through the window
// frame, so Windows still shows snap layouts and its own tooltips.
class CaptionButtons : public QWidget
{
public:
    enum Button { None = -1,
                  Minimize = 0,
                  Maximize = 1,
                  Close = 2 };

    static constexpr int ButtonWidth = 46;
    static constexpr int HoverInMs = 0;
    static constexpr int HoverOutMs = 150;

    explicit CaptionButtons(QWidget *parent = nullptr);
    void setColors(const CaptionButtonColors &colors);

    Button buttonAt(const QPoint &pos) const;
    void setHovered(Button button);
    void setPressed(Button button);
    Button pressed() const { return pressedButton; }
    void setWindowActive(bool active);
    void setMaximized(bool maximized);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QRect buttonRect(Button button) const;
    QString glyph(Button button) const;

    CaptionButtonColors colors;
    QFont glyphFont;
    Button hoveredButton = None;
    Button pressedButton = None;
    bool active = true;
    bool maximized = false;
    qreal hoverLevel[3] = { 0, 0, 0 };
    QVariantAnimation *animations[3];
};

#endif // CAPTION_BUTTONS_H
