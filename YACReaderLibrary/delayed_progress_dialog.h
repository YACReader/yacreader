#ifndef DELAYED_PROGRESS_DIALOG_H
#define DELAYED_PROGRESS_DIALOG_H

#include <QElapsedTimer>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>

#include <functional>

class QProgressDialog;
class QWidget;

// a window-modal progress dialog for work that is often short: a busy cursor at once, the dialog only when the
// work lasts longer than showDelay, and then on screen for at least minimumShownTime, so it never flashes.
// It deletes itself after finish().
class DelayedProgressDialog : public QObject
{
    Q_OBJECT
public:
    static constexpr int showDelay = 500; // ms
    static constexpr int minimumShownTime = 400; // ms

    DelayedProgressDialog(const QString &label, int maximum, QWidget *parent);
    ~DelayedProgressDialog() override;

    void setValue(int value);
    bool isShown() const;
    // closes the dialog when it was on screen long enough, then calls `done`
    void finish(std::function<void()> done);

signals:
    void canceled();

private:
    void showDialog();
    void restoreCursor();

    QWidget *parent;
    QString label;
    int maximum;
    int value = 0;
    bool cursorOverridden = false;
    bool finishing = false;
    QTimer showTimer;
    QElapsedTimer shownTime;
    QPointer<QProgressDialog> dialog;
};

#endif
