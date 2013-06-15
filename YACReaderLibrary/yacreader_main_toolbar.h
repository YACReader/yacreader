#ifndef YACREADER_MAIN_TOOLBAR_H
#define YACREADER_MAIN_TOOLBAR_H

#include <QWidget>

class QToolButton;
class QLabel;
class QResizeEvent;
class QPaintEvent;
class QHBoxLayout;

class YACReaderMainToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderMainToolBar(QWidget *parent = 0);
    QSize sizeHint() const;
signals:
    
public slots:

private:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

    QToolButton * backButton;
    QToolButton * forwardButton;
    QToolButton * settingsButton;
    QToolButton * serverButton;
    QToolButton * helpButton;
    QToolButton * fullscreenButton;

	QHBoxLayout * mainLayout;

    QLabel * currentFolder;

    void addDivider();
    void addWideDivider();

    
};

#endif // YACREADER_MAIN_TOOLBAR_H
