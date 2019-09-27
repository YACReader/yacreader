#ifndef GOTO_FLOW_TOOLBAR_H
#define GOTO_FLOW_TOOLBAR_H

#include <QWidget>
#include <QStackedWidget>

class QLineEdit;
class QIntValidator;
class QPushButton;
class QSlider;
class QLabel;

class GoToFlowToolBar : public QStackedWidget
{
    Q_OBJECT
private:
    QLineEdit *edit;
    QSlider *slider;
    QIntValidator *v;
    QPushButton *centerButton;
    QPushButton *goToButton;
    QLabel *pageHint;
    QWidget *bar;
    void paintEvent(QPaintEvent *) override;

public:
    GoToFlowToolBar(QWidget *parent = nullptr);

public slots:
    void setPage(int pageNumber);
    void setTop(int numPages);
    void goTo();
    void centerSlide();
    void updateOptions();
signals:
    void setCenter(unsigned int);
    void goTo(unsigned int);
};

#endif
