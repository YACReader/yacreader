#ifndef COMIC_PAGE_LABEL_H
#define COMIC_PAGE_LABEL_H

#include <QLabel>

class ComicPageLabel : public QLabel
{
public:
    explicit ComicPageLabel(QWidget *parent = nullptr);
    void setBookFoldShadow(bool enabled, qreal seamRatio, Qt::Orientation seamOrientation);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool foldShadowEnabled = false;
    qreal foldSeamRatio = 0.5;
    Qt::Orientation foldSeamOrientation = Qt::Vertical;
};

#endif
