#include "yacreader_sidebar.h"

#include <QPainter>
#include <QLayout>

#include "yacreader_folders_view.h"
#include "yacreader_reading_lists_view.h"
#include "yacreader_library_list_widget.h"
#include "yacreader_search_line_edit.h"
#include "yacreader_titled_toolbar.h"

#include "yacreader_global.h"
#include "yacreader_global_gui.h"

YACReaderSideBar::YACReaderSideBar(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat); // TODO unificar la creación del fichero de config con el servidor
    settings->beginGroup("libraryConfig");

    // widgets
    foldersView = new YACReaderFoldersView;
    readingListsView = new YACReaderReadingListsView;
    selectedLibrary = new YACReaderLibraryListWidget;

    // Titles will be set from theme in applyTheme
    librariesTitle = new YACReaderTitledToolBar("");
    foldersTitle = new YACReaderTitledToolBar("");
    readingListsTitle = new YACReaderTitledToolBar("");

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);

    selectedLibrary->setContextMenuPolicy(Qt::ActionsContextMenu);
    selectedLibrary->setAttribute(Qt::WA_MacShowFocusRect, false);
    selectedLibrary->setFocusPolicy(Qt::NoFocus);

    // layout
    QVBoxLayout *l = new QVBoxLayout;

    l->setContentsMargins(0, 0, 0, 0);

    // LIBRARIES-------------------------------------------------------
    l->addSpacing(5);

    l->addWidget(librariesTitle);

    l->addSpacing(4);
    auto sep1 = new YACReaderSideBarSeparator(this);
    separators.append(sep1);
    l->addWidget(sep1);
    l->addSpacing(3);

    l->addWidget(selectedLibrary);
    l->addSpacing(11);

    // END LIBRARIES---------------------------------------------------

    // FOLDERS---------------------------------------------------------
    QWidget *foldersContainer = new QWidget(this);
    QVBoxLayout *foldersLayout = new QVBoxLayout;
    foldersLayout->setContentsMargins(0, 0, 0, 0);
    foldersLayout->setSpacing(0);

    auto sep2 = new YACReaderSideBarSeparator(this);
    separators.append(sep2);
    foldersLayout->addWidget(sep2);
    foldersLayout->addSpacing(4);

    foldersLayout->addWidget(foldersTitle);

    foldersLayout->addSpacing(4);
    auto sep3 = new YACReaderSideBarSeparator(this);
    separators.append(sep3);
    foldersLayout->addWidget(sep3);
    foldersLayout->addSpacing(4);

    foldersLayout->addWidget(foldersView);
    foldersLayout->addSpacing(6);

    foldersContainer->setLayout(foldersLayout);
    splitter->addWidget(foldersContainer);
    // END FOLDERS------------------------------------------------------

    // READING LISTS----------------------------------------------------
    splitter->addWidget(readingListsView);

    QVBoxLayout *readingListsHeaderLayout = new QVBoxLayout;
    readingListsHeaderLayout->setContentsMargins(0, 0, 0, 0);
    readingListsHeaderLayout->setSpacing(0);

    auto sep4 = new YACReaderSideBarSeparator(this);
    separators.append(sep4);
    readingListsHeaderLayout->addWidget(sep4);
    readingListsHeaderLayout->addSpacing(4);

    readingListsHeaderLayout->addWidget(readingListsTitle);

    readingListsHeaderLayout->addSpacing(4);
    auto sep5 = new YACReaderSideBarSeparator(this);
    separators.append(sep5);
    readingListsHeaderLayout->addWidget(sep5);
    readingListsHeaderLayout->addSpacing(4);

    readingListsHeaderLayout->addStretch();
    QSplitterHandle *handle = splitter->handle(1);
    handle->setLayout(readingListsHeaderLayout);
    // END READING LISTS------------------------------------------------

    l->addWidget(splitter);
    l->setSpacing(0);

    setLayout(l);

    if (settings->contains(SIDEBAR_SPLITTER_STATUS))
        splitter->restoreState(settings->value(SIDEBAR_SPLITTER_STATUS).toByteArray());

    initTheme(this);
}

void YACReaderSideBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), theme.sidebar.backgroundColor);
}

void YACReaderSideBar::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);

    settings->setValue(SIDEBAR_SPLITTER_STATUS, splitter->saveState());
}

QSize YACReaderSideBar::sizeHint() const
{
    return QSize(275, 200);
}

void YACReaderSideBar::applyTheme(const Theme &theme)
{
    splitter->setStyleSheet(theme.sidebar.splitterQSS);

    // Titles are per-instance, toolbars handle their own colors via Themable
    auto applyCase = [&](const QString &s) { return theme.sidebar.uppercaseLabels ? s.toUpper() : s; };
    librariesTitle->setTitle(applyCase(QObject::tr("Libraries")));
    foldersTitle->setTitle(applyCase(QObject::tr("Folders")));
    readingListsTitle->setTitle(applyCase(QObject::tr("Reading Lists")));

    for (auto separator : separators) {
        separator->setColor(theme.sidebar.sectionSeparatorColor);
    }

    update(); // Trigger repaint for background color
}

YACReaderSideBarSeparator::YACReaderSideBarSeparator(QWidget *parent)
    : QWidget(parent), separatorColor(QColor("#575757"))
{
    setFixedHeight(1);
}

void YACReaderSideBarSeparator::setColor(const QColor &color)
{
    separatorColor = color;
    update();
}

void YACReaderSideBarSeparator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    painter.fillRect(5, 0, width() - 10, height(), separatorColor);
}
