#include "api_key_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>

#include "yacreader_global_gui.h"

ApiKeyDialog::ApiKeyDialog(QWidget *parent)
    : QDialog(parent)
{
    auto layout = new QVBoxLayout;
    auto buttonsLayout = new QHBoxLayout;

    settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat); // TODO unificar la creación del fichero de config con el servidor
    settings->beginGroup("ComicVine");

    QLabel *info = new QLabel(tr("Before you can connect to Comic Vine, you need your own API key. Please, get one free <a href=\"http://www.comicvine.com/api/\">here</a>"));
    info->setWordWrap(true);
    info->setOpenExternalLinks(true);
    edit = new QLineEdit();
    edit->setPlaceholderText(tr("Paste here your Comic Vine API key"));
    connect(edit, &QLineEdit::textChanged, this, &ApiKeyDialog::enableAccept);

    acceptButton = new QPushButton(tr("Accept"));
    acceptButton->setDisabled(true);
    connect(acceptButton, &QAbstractButton::clicked, this, &ApiKeyDialog::saveApiKey);

    cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);

    layout->addWidget(info);
    layout->addWidget(edit);
    layout->addStretch();

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(acceptButton);
    buttonsLayout->addWidget(cancelButton);

    layout->addLayout(buttonsLayout);

    setLayout(layout);

    resize(400, 150);

    if (settings->contains(COMIC_VINE_API_KEY))
        edit->setText(settings->value(COMIC_VINE_API_KEY).toString());
}

ApiKeyDialog::~ApiKeyDialog()
{
    delete settings;
}

void ApiKeyDialog::enableAccept(const QString &text)
{
    // TODO key validation
    acceptButton->setEnabled(!text.isEmpty());
}

void ApiKeyDialog::saveApiKey()
{
    settings->setValue(COMIC_VINE_API_KEY, edit->text().trimmed());
    accept();
}
