#include "delayed_progress_dialog.h"

#include <QApplication>
#include <QProgressDialog>

DelayedProgressDialog::DelayedProgressDialog(const QString &label, int maximum, QWidget *parent)
    : QObject(parent), parent(parent), label(label), maximum(maximum)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    cursorOverridden = true;

    showTimer.setSingleShot(true);
    connect(&showTimer, &QTimer::timeout, this, &DelayedProgressDialog::showDialog);
    showTimer.start(showDelay);
}

DelayedProgressDialog::~DelayedProgressDialog()
{
    restoreCursor();
    if (dialog)
        dialog->deleteLater();
}

void DelayedProgressDialog::setValue(int value)
{
    this->value = value;
    if (dialog)
        dialog->setValue(value);
}

bool DelayedProgressDialog::isShown() const
{
    return dialog != nullptr;
}

void DelayedProgressDialog::finish(std::function<void()> done)
{
    if (finishing)
        return;
    finishing = true;
    showTimer.stop();

    const auto close = [this, done = std::move(done)] {
        restoreCursor();
        if (dialog)
            dialog->hide();
        if (done)
            done();
        deleteLater();
    };
    if (dialog && shownTime.elapsed() < minimumShownTime)
        QTimer::singleShot(minimumShownTime - shownTime.elapsed(), this, close);
    else
        close();
}

void DelayedProgressDialog::showDialog()
{
    restoreCursor();
    dialog = new QProgressDialog(label, tr("Cancel"), 0, maximum, parent);
    dialog->setWindowModality(Qt::WindowModal);
    dialog->setMinimumWidth(350);
    dialog->setMinimumDuration(0);
    dialog->setAutoClose(false);
    dialog->setAutoReset(false);
    dialog->setValue(value);
    connect(dialog, &QProgressDialog::canceled, this, &DelayedProgressDialog::canceled);
    dialog->show();
    shownTime.start();
}

void DelayedProgressDialog::restoreCursor()
{
    if (!cursorOverridden)
        return;
    cursorOverridden = false;
    QApplication::restoreOverrideCursor();
}
