#include "organize_files_dialog.h"

#include "yacreader_global.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QVBoxLayout>

OrganizeFilesDialog::OrganizeFilesDialog(const QString &libraryRoot,
                                         const QString &selectedFolderPath,
                                         QSettings *settings,
                                         QWidget *parent)
    : QDialog(parent), libraryRoot(libraryRoot), selectedFolderPath(selectedFolderPath), settings(settings)
{
    setupUI();
}

QString OrganizeFilesDialog::defaultPattern()
{
    return QStringLiteral("{publisher}/{series}/#{number} {title}");
}

void OrganizeFilesDialog::setupUI()
{
    auto description = new QLabel(tr("Files will be moved into subfolders following the format below. "
                                     "Each part separated by \"/\" becomes a folder, except the last one which becomes the file name."));
    description->setWordWrap(true);

    auto tokensLabel = new QLabel(tr("Available tokens: %1")
                                          .arg(QStringLiteral("{publisher} {series} {number} {title} {volume} {year}")));
    tokensLabel->setWordWrap(true);

    auto hintLabel = new QLabel(tr("{title} falls back to the series name when the comic has no title."));
    hintLabel->setWordWrap(true);

    patternEdit = new QLineEdit(defaultPattern());
    connect(patternEdit, &QLineEdit::textChanged, this, &OrganizeFilesDialog::updatePreview);

    relativeToRootCheck = new QCheckBox(tr("Place folders relative to the library root"));
    relativeToRootCheck->setToolTip(tr("When enabled, the format is applied from the library root instead of the "
                                       "selected folder, so it is not nested inside the folder being organized."));
    const bool relativeToRoot = settings ? settings->value(ORGANIZE_FILES_RELATIVE_TO_ROOT, true).toBool() : true;
    relativeToRootCheck->setChecked(relativeToRoot);
    connect(relativeToRootCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (settings)
            settings->setValue(ORGANIZE_FILES_RELATIVE_TO_ROOT, checked);
        updatePreview();
    });

    previewLabel = new QLabel;
    previewLabel->setWordWrap(true);
    previewLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(description);
    mainLayout->addWidget(new QLabel(tr("Format:")));
    mainLayout->addWidget(patternEdit);
    mainLayout->addWidget(relativeToRootCheck);
    mainLayout->addWidget(tokensLabel);
    mainLayout->addWidget(hintLabel);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(previewLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setModal(true);
    setWindowTitle(tr("Organize files"));
    resize(480, sizeHint().height());

    updatePreview();
}

QString OrganizeFilesDialog::formatPattern() const
{
    return patternEdit->text();
}

bool OrganizeFilesDialog::relativeToRoot() const
{
    return relativeToRootCheck->isChecked();
}

void OrganizeFilesDialog::updatePreview()
{
    const QString relative = buildRelativePath(patternEdit->text(),
                                               QStringLiteral("Marvel"),
                                               QStringLiteral("The Amazing Spider-Man"),
                                               QStringLiteral("42"),
                                               QStringLiteral("The Sinister Six"),
                                               QStringLiteral("1"),
                                               QStringLiteral("2018"),
                                               QStringLiteral(".cbz"));

    const QString base = relativeToRootCheck->isChecked() ? libraryRoot : selectedFolderPath;
    const QString example = base.isEmpty() ? relative : QDir::cleanPath(base + QLatin1Char('/') + relative);
    previewLabel->setText(tr("Example: %1").arg(example));
}

static QString sanitizeSegment(QString segment)
{
    static const QString invalid = QStringLiteral("<>:\"/\\|?*");
    for (QChar &c : segment) {
        if (invalid.contains(c) || c < QChar(0x20))
            c = QLatin1Char('_');
    }
    segment = segment.simplified();
    while (segment.endsWith(QLatin1Char('.')) || segment.endsWith(QLatin1Char(' ')))
        segment.chop(1);
    return segment;
}

QString OrganizeFilesDialog::buildRelativePath(const QString &pattern,
                                               const QString &publisher,
                                               const QString &series,
                                               const QString &number,
                                               const QString &title,
                                               const QString &volume,
                                               const QString &year,
                                               const QString &extension)
{
    const QString safeSeries = series.trimmed().isEmpty() ? tr("Unknown Series") : series.trimmed();
    const QString safePublisher = publisher.trimmed().isEmpty() ? tr("Unknown Publisher") : publisher.trimmed();
    const QString effectiveTitle = title.trimmed().isEmpty() ? safeSeries : title.trimmed();

    QString result = pattern;
    result.replace(QStringLiteral("{publisher}"), safePublisher);
    result.replace(QStringLiteral("{series}"), safeSeries);
    result.replace(QStringLiteral("{number}"), number.trimmed());
    result.replace(QStringLiteral("{title}"), effectiveTitle);
    result.replace(QStringLiteral("{volume}"), volume.trimmed());
    result.replace(QStringLiteral("{year}"), year.trimmed());

    const QStringList rawSegments = result.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    QStringList segments;
    for (const QString &raw : rawSegments) {
        const QString clean = sanitizeSegment(raw);
        if (!clean.isEmpty())
            segments << clean;
    }

    if (segments.isEmpty())
        segments << sanitizeSegment(effectiveTitle);

    QString relativePath = segments.join(QLatin1Char('/'));
    if (!extension.isEmpty())
        relativePath += extension;
    return relativePath;
}
