#include "delayed_progress_dialog.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QProgressDialog>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>

class DelayedProgressDialogTest : public QObject
{
    Q_OBJECT
private slots:
    void shortWorkNeverShowsTheDialog();
    void longWorkShowsTheDialogForTheMinimumTime();
    void longShownWorkClosesAtOnce();
    void cancelIsReported();
};

void DelayedProgressDialogTest::shortWorkNeverShowsTheDialog()
{
    QWidget window;
    QPointer progress = new DelayedProgressDialog("Working", 10, &window);
    QVERIFY(QApplication::overrideCursor() != nullptr);

    QTest::qWait(100);
    bool done = false;
    progress->finish([&done] { done = true; });

    QVERIFY(done);
    QVERIFY(QApplication::overrideCursor() == nullptr);
    QVERIFY(window.findChildren<QProgressDialog *>().isEmpty());
    QTRY_VERIFY(progress.isNull());
}

void DelayedProgressDialogTest::longWorkShowsTheDialogForTheMinimumTime()
{
    QWidget window;
    auto *progress = new DelayedProgressDialog("Working", 10, &window);
    QTest::qWait(DelayedProgressDialog::showDelay / 2);
    QVERIFY(!progress->isShown());

    QTRY_VERIFY(progress->isShown());
    QVERIFY(QApplication::overrideCursor() == nullptr);
    progress->setValue(4);
    QCOMPARE(window.findChild<QProgressDialog *>()->value(), 4);

    // the work ends just after the dialog opened: it stays the minimum time
    bool done = false;
    QElapsedTimer finished;
    finished.start();
    progress->finish([&done] { done = true; });
    QVERIFY(!done);
    QTRY_VERIFY(done);
    QVERIFY(finished.elapsed() >= DelayedProgressDialog::minimumShownTime - 100);
}

void DelayedProgressDialogTest::longShownWorkClosesAtOnce()
{
    QWidget window;
    auto *progress = new DelayedProgressDialog("Working", 10, &window);
    QTRY_VERIFY(progress->isShown());
    QTest::qWait(DelayedProgressDialog::minimumShownTime + 50);

    bool done = false;
    progress->finish([&done] { done = true; });
    QVERIFY(done);
}

void DelayedProgressDialogTest::cancelIsReported()
{
    QWidget window;
    auto *progress = new DelayedProgressDialog("Working", 10, &window);
    QSignalSpy canceled(progress, &DelayedProgressDialog::canceled);
    QTRY_VERIFY(progress->isShown());

    window.findChild<QProgressDialog *>()->findChild<QPushButton *>()->click();
    QCOMPARE(canceled.count(), 1);
    progress->finish({ });
}

QTEST_MAIN(DelayedProgressDialogTest)

#include "main.moc"
