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
    void keyPressEvent(QKeyEvent *event) override;

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
