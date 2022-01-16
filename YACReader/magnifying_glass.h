#ifndef __MAGNIFYING_GLASS
#define __MAGNIFYING_GLASS

#include <QLabel>
#include <QtGui>
#include <QMouseEvent>
#include <QWidget>

class MagnifyingGlass : public QLabel
{
    Q_OBJECT
private:
    float zoomLevel;
    void setup(const QSize &size);
    void resizeAndUpdate(int w, int h);

    // The following 4 functions increase/decrease their argument and return true,
    // unless the maximum dimension value has been reached, in which case they
    // do not modify the argument and return false.
    bool growWidth(int &w) const;
    bool shrinkWidth(int &w) const;
    bool growHeight(int &h) const;
    bool shrinkHeight(int &h) const;

public:
    MagnifyingGlass(int width, int height, QWidget *parent);
    MagnifyingGlass(const QSize &size, QWidget *parent);
    void mouseMoveEvent(QMouseEvent *event) override;
public slots:
    void updateImage(int x, int y);
    void updateImage();
    void wheelEvent(QWheelEvent *event) override;
    void zoomIn();
    void zoomOut();
    void sizeUp();
    void sizeDown();
    void heightUp();
    void heightDown();
    void widthUp();
    void widthDown();
};

#endif
