#include "no_libraries_widget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

NoLibrariesWidget::NoLibrariesWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAutoFillBackground(true);

    iconLabel = new QLabel();

    QPixmap line(":/images/noLibrariesLine.png");
    QLabel *lineLabel = new QLabel();
    lineLabel->setPixmap(line);

    text = new QLabel(tr("You don't have any libraries yet"));
    text->setStyleSheet("QLabel {font-size:25px;font-weight:bold;}");
    textDescription = new QLabel(tr("<p>You can create a library in any folder, YACReaderLibrary will import all comics and folders from this folder. If you have created any library in the past you can open them.</p><p>Don't forget that you can use YACReader as a stand alone application for reading the comics on your computer.</p>"));
    textDescription->setWordWrap(true);
    textDescription->setMaximumWidth(330);

    QPushButton *createButton = new QPushButton(tr("create your first library"));
    createButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QPushButton *addButton = new QPushButton(tr("add an existing one"));
    addButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

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

    buttonLayout->addSpacing(125);
    buttonLayout->addWidget(createButton);
    layout->addSpacing(25);
    buttonLayout->addWidget(addButton);
    buttonLayout->addSpacing(125);

    layout->addStretch();
    layout->addWidget(topWidget);
    layout->addSpacing(20);
    layout->addWidget(lineLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addLayout(buttonLayout, 0);
    layout->addSpacing(150);
    layout->addStretch();

    connect(createButton, &QAbstractButton::clicked, this, &NoLibrariesWidget::createNewLibrary);
    connect(addButton, &QAbstractButton::clicked, this, &NoLibrariesWidget::addExistingLibrary);

    initTheme(this);
}

void NoLibrariesWidget::applyTheme(const Theme &theme)
{
    auto emptyTheme = theme.emptyContainer;

    QPalette p(palette());
    p.setColor(QPalette::Window, emptyTheme.backgroundColor);
    setPalette(p);

    QPalette textPalette = text->palette();
    textPalette.setColor(QPalette::WindowText, emptyTheme.textColor);
    text->setPalette(textPalette);

    QPalette descPalette = textDescription->palette();
    descPalette.setColor(QPalette::WindowText, emptyTheme.descriptionTextColor);
    textDescription->setPalette(descPalette);

    iconLabel->setPixmap(emptyTheme.noLibrariesIcon);
}
