#ifndef YACREADER_MAIN_TOOLBAR_H
#define YACREADER_MAIN_TOOLBAR_H

#include <QWidget>

class QToolButton;
class QLabel;
class QResizeEvent;
class QPaintEvent;
class QHBoxLayout;

// TODO create methods for adding actions, separators and sctreches dynimically
class YACReaderMainToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderMainToolBar(QWidget *parent = nullptr);
    QSize sizeHint() const override;

    QToolButton *backButton;
    QToolButton *forwardButton;
    QToolButton *settingsButton;
    QToolButton *serverButton;
    QToolButton *helpButton;
    QToolButton *toggleComicsViewButton;
    QToolButton *fullscreenButton;

    void setSearchWidget(QWidget *w);
    void setCurrentFolderName(const QString &name);
signals:

public slots:

private:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

    QHBoxLayout *mainLayout;

    QLabel *currentFolder;
    QString currentFolderName;

    void addDivider();
    void addWideDivider();
};

#endif // YACREADER_MAIN_TOOLBAR_H
