#ifndef CONTINUOUS_PAGE_WIDGET_H
#define CONTINUOUS_PAGE_WIDGET_H

#include <QWidget>
#include <QSize>
#include <QVector>

#include "themable.h"

class Render;
class ContinuousViewModel;

class ContinuousPageWidget : public QWidget, protected Themable
{
    Q_OBJECT
public:
    explicit ContinuousPageWidget(QWidget *parent = nullptr);

    void setRender(Render *r);
    void setViewModel(ContinuousViewModel *viewModel);
    void reset();

    bool hasHeightForWidth() const override;
    int heightForWidth(int w) const override;
    QSize sizeHint() const override;

public slots:
    void onPageAvailable(int absolutePageIndex);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void applyTheme(const Theme &theme) override;

private:
    Render *render = nullptr;
    ContinuousViewModel *continuousViewModel = nullptr;
};

#endif // CONTINUOUS_PAGE_WIDGET_H
