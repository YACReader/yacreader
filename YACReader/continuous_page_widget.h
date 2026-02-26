#ifndef CONTINUOUS_PAGE_WIDGET_H
#define CONTINUOUS_PAGE_WIDGET_H

#include <QWidget>
#include <QSize>
#include <QVector>
#include <QHash>
#include <QList>

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
    struct ScaledPageCacheEntry {
        qint64 sourceCacheKey = 0;
        QSize sourceSize;
        QSize targetSize;
        QImage scaledImage;
    };

    struct ScaledPageCache {
        int effectiveWidth = -1;
        QHash<int, ScaledPageCacheEntry> pages;

        void invalidateAll()
        {
            effectiveWidth = -1;
            pages.clear();
        }

        void invalidateForWidth(int width)
        {
            if (effectiveWidth != width) {
                effectiveWidth = width;
                pages.clear();
            }
        }

        void invalidatePage(int pageIndex)
        {
            pages.remove(pageIndex);
        }

        void keepOnlyRange(int minPageIndex, int maxPageIndex)
        {
            if (pages.isEmpty()) {
                return;
            }

            QList<int> keysToRemove;
            keysToRemove.reserve(pages.size());
            for (auto it = pages.constBegin(); it != pages.constEnd(); ++it) {
                if (it.key() < minPageIndex || it.key() > maxPageIndex) {
                    keysToRemove.append(it.key());
                }
            }

            for (int key : keysToRemove) {
                pages.remove(key);
            }
        }
    };

    const QImage *scaledImageForPaint(int pageIndex, const QImage *source, const QSize &targetSize, int effectiveWidth);

    Render *render = nullptr;
    ContinuousViewModel *continuousViewModel = nullptr;
    ScaledPageCache scaledPageCache;
};

#endif // CONTINUOUS_PAGE_WIDGET_H
