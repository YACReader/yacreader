#include "organize_files_dialog.h"

#include <QDialogButtonBox>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

OrganizeFilesDialog::OrganizeFilesDialog(QWidget *parent)
    : QDialog(parent)
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

void OrganizeFilesDialog::updatePreview()
{
    // Example metadata so the user can see the resulting layout live.
    const QString example = buildRelativePath(patternEdit->text(),
                                              QStringLiteral("Marvel"),
                                              QStringLiteral("The Amazing Spider-Man"),
                                              QStringLiteral("42"),
                                              QStringLiteral("The Sinister Six"),
                                              QStringLiteral("1"),
                                              QStringLiteral("2018"),
                                              QStringLiteral(".cbz"));
    previewLabel->setText(tr("Example: %1").arg(example));
}

static QString sanitizeSegment(QString segment)
{
    // Replace characters that are invalid in file/folder names on common
    // filesystems, then collapse whitespace and trim.
    static const QString invalid = QStringLiteral("<>:\"/\\|?*");
    for (QChar &c : segment) {
        if (invalid.contains(c) || c < QChar(0x20))
            c = QLatin1Char('_');
    }
    segment = segment.simplified();
    // Windows does not allow trailing dots or spaces in names.
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
    // {title} falls back to the series name, as requested.
    const QString effectiveTitle = title.trimmed().isEmpty() ? safeSeries : title.trimmed();

    QString result = pattern;
    result.replace(QStringLiteral("{publisher}"), safePublisher);
    result.replace(QStringLiteral("{series}"), safeSeries);
    result.replace(QStringLiteral("{number}"), number.trimmed());
    result.replace(QStringLiteral("{title}"), effectiveTitle);
    result.replace(QStringLiteral("{volume}"), volume.trimmed());
    result.replace(QStringLiteral("{year}"), year.trimmed());

    // Split into segments, sanitize each, drop empty ones.
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
