#ifndef IMPORT_WIDGET_H
#define IMPORT_WIDGET_H

#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QString>
#include <QWidget>

#include "themable.h"

class ImportWidget : public QWidget, protected Themable
{
    Q_OBJECT
public:
    explicit ImportWidget(QWidget *parent = 0);

protected:
    void applyTheme(const Theme &theme) override;

signals:
    void stop();
public slots:
    void newComic(const QString &path, const QString &coverPath);
    void newCover(const QPixmap &image);
    void clear();
    void addCoverTest();
    void clearScene();
    void setImportLook();
    void setUpdateLook();
    void setUpgradeLook();
    void setXMLScanLook();
    void showCovers(bool hide);

private:
    QLabel *currentComicLabel;
    QLabel *coversLabel;
    QLabel *iconLabel;
    QLabel *text;
    QLabel *textDescription;
    QWidget *coversViewContainer;
    QGraphicsView *coversView;
    QGraphicsScene *coversScene;
    QPropertyAnimation *scrollAnimation;
    QPushButton *stopButton;

    int previousWidth;
    bool updatingCovers;
    QElapsedTimer *elapsedTimer;
    quint64 i;

    QToolButton *hideButton;
    QLabel *topDecorator;
    QLabel *bottomDecorator;

    void resizeEvent(QResizeEvent *event) override;
    void updateTextColors();
};

#endif // IMPORT_WIDGET_H
