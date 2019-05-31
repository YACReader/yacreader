#ifndef IMPORT_WIDGET_H
#define IMPORT_WIDGET_H

#include <QtWidgets>

class ImportWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImportWidget(QWidget *parent = 0);

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

    void resizeEvent(QResizeEvent *event) override;
};

#endif // IMPORT_WIDGET_H
