#include "import_widget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QScrollBar>
#include <QGraphicsItemAnimation>
#include <QTimeLine>
// TODO: is QGLWidget needed here???
// #include <QGLWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QToolButton>
#include <QResizeEvent>

#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class YACReaderActivityIndicatorWidget : public QWidget
{
public:
    YACReaderActivityIndicatorWidget(QWidget *parent = 0);
public slots:

private:
    QLabel *normal;
    QLabel *glow;
};

YACReaderActivityIndicatorWidget::YACReaderActivityIndicatorWidget(QWidget *parent)
    : QWidget(parent)
{
    QPixmap line(":/images/noLibrariesLine.png");
    QPixmap glowLine(":/images/glowLine.png");
    normal = new QLabel(this);
    glow = new QLabel(this);

    normal->setPixmap(line);
    glow->setPixmap(glowLine);

    auto layout = new QHBoxLayout();

    layout->addWidget(normal, 0, Qt::AlignVCenter);

    setLayout(layout);

    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(0);

    // setFixedHeight(3);
    // resize(579,3);
    glow->setGeometry(4, 4, glowLine.width(), glowLine.height());
    // normal->setGeometry(0,1,579,1);

    auto effect = new QGraphicsOpacityEffect();
    // effect->setOpacity(1.0);

    auto *animation = new QPropertyAnimation(effect, "opacity", this);

    animation->setDuration(1000);
    animation->setStartValue(1);
    animation->setEndValue(0);
    // animation->setEasingCurve(QEasingCurve::InQuint);

    auto *animation2 = new QPropertyAnimation(effect, "opacity", this);

    animation2->setDuration(1000);
    animation2->setStartValue(0);
    animation2->setEndValue(1);
    // animation2->setEasingCurve(QEasingCurve::InQuint);

    glow->setGraphicsEffect(effect);

    connect(animation, &QPropertyAnimation::finished, animation2, [=] { animation2->start(); });
    connect(animation2, &QPropertyAnimation::finished, animation, [=] { animation->start(); });

    animation->start();
}

ImportWidget::ImportWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAutoFillBackground(true);

    iconLabel = new QLabel();

    auto activityIndicator = new YACReaderActivityIndicatorWidget();

    text = new QLabel();
    textDescription = new QLabel();
    textDescription->setWordWrap(true);
    textDescription->setMaximumWidth(330);
    currentComicLabel = new QLabel("...");

    coversViewContainer = new QWidget(this);
    auto coversViewLayout = new QVBoxLayout;
    coversViewContainer->setLayout(coversViewLayout);
    coversViewContainer->setMaximumHeight(316);
    coversViewContainer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);

    coversView = new QGraphicsView();
    coversView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    coversView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    coversView->setMaximumHeight(300);

    coversScene = new QGraphicsScene();
    coversView->setAlignment(Qt::AlignLeft);
    coversView->setScene(coversScene);
    coversView->setFixedHeight(300);

    coversView->setInteractive(false);

    scrollAnimation = new QPropertyAnimation(coversView->horizontalScrollBar(), "value");

    topDecorator = new QLabel();
    bottomDecorator = new QLabel();
    topDecorator->setScaledContents(true);
    bottomDecorator->setScaledContents(true);

    coversViewLayout->addWidget(topDecorator, 0);
    coversViewLayout->addWidget(coversView, 1);
    coversViewLayout->addWidget(bottomDecorator, 0);
    coversViewLayout->setContentsMargins(0, 0, 0, 0);
    coversViewLayout->setSpacing(0);

    QPushButton *stop = new QPushButton(tr("stop"));
    stop->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    stopButton = stop;

    auto layout = new QVBoxLayout(this);
    auto buttonLayout = new QHBoxLayout();
    auto topLayout = new QHBoxLayout();
    auto textLayout = new QVBoxLayout();

    QWidget *topWidget = new QWidget();
    topWidget->setFixedWidth(650);
    textLayout->addStretch();
    textLayout->addWidget(text);
    textLayout->addSpacing(12);
    textLayout->addWidget(textDescription);
    textLayout->addStretch();

    topLayout->addStretch();
    topLayout->addWidget(iconLabel, 0, Qt::AlignVCenter);
    topLayout->addSpacing(30);
    topLayout->addLayout(textLayout, 1);
    topLayout->addStretch();
    topLayout->setContentsMargins(0, 0, 0, 0);

    topWidget->setLayout(topLayout);

    layout->setAlignment(Qt::AlignHCenter);

    buttonLayout->addSpacing(250);
    buttonLayout->addWidget(stop);
    buttonLayout->addSpacing(250);

    layout->addSpacing(50);
    layout->addWidget(topWidget, 0, Qt::AlignHCenter);
    layout->addSpacing(20);
    layout->addWidget(activityIndicator, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addLayout(buttonLayout, 0);
    layout->addSpacing(10);
    layout->addStretch();
    coversLabel = new QLabel(tr("Some of the comics being added..."));

    hideButton = new QToolButton(this);
    hideButton->setFixedSize(25, 18);
    hideButton->setStyleSheet("QToolButton { border: none; padding: 0px; }"
                              "QToolButton:pressed { border: none; padding: 0px; }"
                              "QToolButton:checked { border: none; padding: 0px; }");
    hideButton->setCheckable(true);

    connect(hideButton, &QAbstractButton::toggled, this, &ImportWidget::showCovers);

    layout->addWidget(coversLabel, 0, Qt::AlignHCenter);
    layout->addWidget(coversViewContainer);
    // layout->addStretch();
    layout->addWidget(currentComicLabel, 0, Qt::AlignHCenter);
    layout->setContentsMargins(0, layout->contentsMargins().top(), 0, layout->contentsMargins().bottom());

    connect(stop, &QAbstractButton::clicked, this, &ImportWidget::stop);
    // connect(stop,SIGNAL(clicked()),this,SLOT(addCoverTest()));

    previousWidth = 0;
    updatingCovers = false;
    elapsedTimer = new QElapsedTimer();
    elapsedTimer->start();

    initTheme(this);
}

void ImportWidget::newComic(const QString &path, const QString &coverPath)
{
    if (!this->isVisible())
        return;

    currentComicLabel->setText(path);

    if (((elapsedTimer->elapsed() >= 1100) || ((previousWidth < coversView->width()) && (elapsedTimer->elapsed() >= 500))) && scrollAnimation->state() != QAbstractAnimation::Running) // todo elapsed time
    {
        updatingCovers = true;
        elapsedTimer->start();

        QPixmap p(coverPath);
        p = p.scaledToHeight(300, Qt::SmoothTransformation);

        auto item = new QGraphicsPixmapItem(p);
        item->setPos(previousWidth, 0);
        coversScene->addItem(item);

        previousWidth += 10 + p.width();

        foreach (QGraphicsItem *itemToRemove, coversScene->items()) {
            auto last = dynamic_cast<QGraphicsPixmapItem *>(itemToRemove);

            if ((last->pos().x() + last->pixmap().width()) < coversView->horizontalScrollBar()->value()) // TODO check this
            {
                coversScene->removeItem(last);
                delete last;
            }
        }

        QScrollBar *scrollBar = coversView->horizontalScrollBar();

        float speedFactor = 2.5;
        int origin = scrollBar->value();
        int dest = origin + 10 + p.width();

        scrollAnimation->setDuration((dest - origin) * speedFactor);
        scrollAnimation->setStartValue(origin);
        scrollAnimation->setEndValue(dest);
        QEasingCurve easing(QEasingCurve::OutQuad);
        scrollAnimation->setEasingCurve(easing);
        scrollAnimation->start();
    }
}

void ImportWidget::newCover(const QPixmap &image)
{
    Q_UNUSED(image)
}
static int j = 0;
void ImportWidget::addCoverTest()
{
    QPixmap p(QString("c:/temp/%1.jpg").arg(i));
    p = p.scaledToHeight(300, Qt::SmoothTransformation);
    auto item = new QGraphicsPixmapItem(p);
    item->setPos(previousWidth, 0);
    item->setZValue(i / 10000.0);
    previousWidth += 10 + p.width();
    coversScene->addItem(item);
    if (previousWidth >= coversView->width()) {
        QGraphicsItem *last = coversScene->items().last();
        int width = p.width();
        if (j >= 1) {
            coversScene->removeItem(last);
            delete last;
        } else
            j++;

        foreach (QGraphicsItem *itemToMove, coversScene->items()) {

            auto timer = new QTimeLine(/*350*/ 1000);
            timer->setFrameRange(0, 60);

            auto animation = new QGraphicsItemAnimation;
            animation->setItem(itemToMove);
            animation->setTimeLine(timer);

            QPointF point = itemToMove->scenePos();
            float step = (width + 10) / 60.0;
            for (int i = 0; i < 60; ++i)
                animation->setPosAt(i / 60.0, QPointF(point.x() - ((i + 1) * step), point.y()));

            timer->start();
        }
        previousWidth -= 10 + width;
    }

    i++;
}

void ImportWidget::clear()
{
    previousWidth = 0;

    // nos aseguramos de que las animaciones han finalizado antes de borrar
    QList<QGraphicsItem *> all = coversScene->items();
    for (int i = 0; i < all.size(); i++) {
        QGraphicsItem *gi = all[i];
        if (gi->parentItem() == NULL)
            delete gi;
    }
    coversScene->clear();

    delete coversScene;
    coversScene = new QGraphicsScene;

    coversView->setScene(coversScene);

    updatingCovers = false;

    currentComicLabel->setText("...");

    this->i = 0;
}

void ImportWidget::setImportLook()
{
    iconLabel->setPixmap(theme.importWidget.importingIcon);
    text->setText(tr("Importing comics"));
    textDescription->setText(tr("<p>YACReaderLibrary is now creating a new library.</p><p>Create a library could take several minutes. You can stop the process and update the library later for completing the task.</p>"));

    stopButton->setVisible(true);
    coversLabel->setVisible(true);
    coversViewContainer->setVisible(true);
    hideButton->setVisible(true);
}

void ImportWidget::setUpdateLook()
{
    iconLabel->setPixmap(theme.importWidget.updatingIcon);
    text->setText(tr("Updating the library"));
    textDescription->setText(tr("<p>The current library is being updated. For faster updates, please, update your libraries frequently.</p><p>You can stop the process and continue updating this library later.</p>"));

    stopButton->setVisible(true);
    coversLabel->setVisible(true);
    coversViewContainer->setVisible(true);
    hideButton->setVisible(true);
}

void ImportWidget::setUpgradeLook()
{
    iconLabel->setPixmap(theme.importWidget.updatingIcon);
    text->setText(tr("Upgrading the library"));
    textDescription->setText(tr("<p>The current library is being upgraded, please wait.</p>"));

    stopButton->setVisible(false);
    coversLabel->setVisible(false);
    coversViewContainer->setVisible(false);
    hideButton->setVisible(false);
}

void ImportWidget::setXMLScanLook()
{
    iconLabel->setPixmap(theme.importWidget.updatingIcon);
    text->setText(tr("Scanning the library"));
    textDescription->setText(tr("<p>Current library is being scanned for legacy XML metadata information.</p><p>This is only needed once, and only if the library was crated with YACReaderLibrary 9.8.2 or earlier.</p>"));

    stopButton->setVisible(true);
    coversLabel->setVisible(false);
    coversViewContainer->setVisible(false);
    hideButton->setVisible(false);
}

void ImportWidget::clearScene()
{
}

void ImportWidget::showCovers(bool hide)
{
    coversLabel->setHidden(hide);
    coversViewContainer->setHidden(hide);
}

void ImportWidget::resizeEvent(QResizeEvent *event)
{
    hideButton->move(event->size().width() - hideButton->width() - (currentComicLabel->height() / 2), event->size().height() - hideButton->height() - (currentComicLabel->height() / 2));

    QWidget::resizeEvent(event);
}

void ImportWidget::applyTheme(const Theme &theme)
{
    const auto &importTheme = theme.importWidget;

    // Covers toggle button
    hideButton->setIcon(importTheme.coversToggleIcon);
    hideButton->setIconSize(hideButton->size());

    // Background
    QPalette p(palette());
    p.setColor(QPalette::Window, importTheme.backgroundColor);
    setPalette(p);

    // Covers view background
    coversView->setStyleSheet(QString("QGraphicsView {background-color: %1; border:none;}")
                                      .arg(importTheme.coversViewBackgroundColor.name()));

    // Covers decorations
    topDecorator->setPixmap(importTheme.topCoversDecoration);
    topDecorator->setFixedHeight(importTheme.topCoversDecoration.height());
    bottomDecorator->setPixmap(importTheme.bottomCoversDecoration);
    bottomDecorator->setFixedHeight(importTheme.bottomCoversDecoration.height());

    // Apply text colors
    updateTextColors();
}

void ImportWidget::updateTextColors()
{
    const auto &importTheme = theme.importWidget;

    // Title text
    text->setStyleSheet(importTheme.titleLabelQSS);

    // Description text
    QPalette descPalette = textDescription->palette();
    descPalette.setColor(QPalette::WindowText, importTheme.descriptionTextColor);
    textDescription->setPalette(descPalette);

    // Current comic label
    QPalette comicPalette = currentComicLabel->palette();
    comicPalette.setColor(QPalette::WindowText, importTheme.currentComicTextColor);
    currentComicLabel->setPalette(comicPalette);

    // Covers label
    QPalette coversLabelPalette = coversLabel->palette();
    coversLabelPalette.setColor(QPalette::WindowText, importTheme.coversLabelColor);
    coversLabel->setPalette(coversLabelPalette);
}
