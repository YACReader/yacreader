#ifndef VIEW_SELECTOR_H
#define VIEW_SELECTOR_H

#include <QWidget>

class QAction;
class QToolButton;
struct HeaderColors;

// Grid / Flow / Info segments for three checkable actions. The actions hold
// the state; the content views manager checks the current view.
class ViewSelector : public QWidget
{
    Q_OBJECT
public:
    ViewSelector(QAction *grid, QAction *flow, QAction *info, QWidget *parent = nullptr);
    void applyTheme(const HeaderColors &colors);
    QList<QToolButton *> segments() const { return buttons; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<QToolButton *> buttons;
    QColor background;
};

#endif // VIEW_SELECTOR_H
