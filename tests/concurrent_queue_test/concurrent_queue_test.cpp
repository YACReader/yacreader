#include "concurrent_queue.h"

#include <QDebug>
#include <QDebugStateSaver>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QTest>
#include <QTime>
#include <QVector>

#include <atomic>
#include <chrono>
#include <numeric>
#include <sstream>
#include <thread>
#include <vector>

namespace chrono = std::chrono;
using Clock = chrono::steady_clock;
using YACReader::ConcurrentQueue;

namespace {
double toMilliseconds(Clock::duration duration)
{
    return chrono::duration_cast<chrono::microseconds>(duration).count() / 1000.0;
}

QString currentThreadInfo()
{
    std::ostringstream os;
    os << std::this_thread::get_id();
    return QString::fromStdString(os.str());
}

QDebug log()
{
    return qInfo().noquote() << currentThreadInfo() << '|'
                             << QTime::currentTime().toString(Qt::ISODateWithMs) << '|';
}

using Total = std::atomic<int>;

struct JobData {
    int summand;
    Clock::duration sleepingTime;
};
using JobDataSet = QVector<JobData>;

int expectedTotal(const JobDataSet &jobs)
{
    return std::accumulate(jobs.cbegin(), jobs.cend(), 0,
                           [](int total, JobData job) {
                               return total + job.summand;
                           });
}

int expectedTotal(const QVector<JobDataSet> &jobs)
{
    return std::accumulate(jobs.cbegin(), jobs.cend(), 0,
                           [](int total, const JobDataSet &dataSet) {
                               return total + expectedTotal(dataSet);
                           });
}

class Id
{
public:
    explicit Id(int threadId, int jobId)
        : threadId { threadId }, jobId { jobId } { }

    QString toString() const { return QStringLiteral("[%1.%2]").arg(threadId).arg(jobId); }

private:
    const int threadId;
    const int jobId;
};

QDebug operator<<(QDebug debug, Id id)
{
    QDebugStateSaver saver(debug);
    debug.noquote() << id.toString();
    return debug;
}

class Job
{
public:
    explicit Job(Total &total, JobData data, Id id)
        : total { total }, data { data }, id { id } { }

    void operator()()
    {
        log().nospace() << id << " sleep " << toMilliseconds(data.sleepingTime) << " ms...";
        std::this_thread::sleep_for(data.sleepingTime);

        const auto updatedTotal = (total += data.summand);
        log().nospace() << id << " +" << data.summand << " => " << updatedTotal;
    }

private:
    Total &total;
    const JobData data;
    const Id id;
};

class Enqueuer
{
public:
    explicit Enqueuer(ConcurrentQueue &queue, Total &total, const JobDataSet &jobs, int threadId)
        : queue { queue }, total { total }, jobs { jobs }, threadId { threadId } { }

    void operator()()
    {
        const char *const jobStr = jobs.size() == 1 ? "job" : "jobs";
        log() << QStringLiteral("#%1 enqueuing %2 %3...").arg(threadId).arg(jobs.size()).arg(jobStr);
        for (int i = 0; i < jobs.size(); ++i)
            queue.enqueue(Job(total, jobs.at(i), Id(threadId, i + 1)));
        log() << QStringLiteral("#%1 enqueuing complete.").arg(threadId);
    }

private:
    ConcurrentQueue &queue;
    Total &total;
    const JobDataSet jobs;
    const int threadId;
};

}

Q_DECLARE_METATYPE(JobData)

class ConcurrentQueueTest : public QObject
{
    Q_OBJECT
private slots:
    void init();

    void singleUserThread_data();
    void singleUserThread();

    void multipleUserThreads_data();
    void multipleUserThreads();

private:
    static constexpr int primaryThreadId { 0 };

    QString messageFormatString(int threadCount) const
    {
        auto format = QStringLiteral("#%1 %5 %2 %3 => %4").arg(primaryThreadId);
        const char *const threadStr = threadCount == 1 ? "thread" : "threads";
        return format.arg(threadCount).arg(threadStr).arg(total.load());
    }

    void printStartedMessage(int threadCount) const
    {
        log() << messageFormatString(threadCount).arg("started");
    }
    void printWaitedMessage(int threadCount) const
    {
        log() << messageFormatString(threadCount).arg("waited for");
    }

    Total total { 0 };
};

void ConcurrentQueueTest::init()
{
    total = 0;
}

void ConcurrentQueueTest::singleUserThread_data()
{
    QTest::addColumn<int>("threadCount");
    QTest::addColumn<JobDataSet>("jobs");

    using ms = chrono::milliseconds;

    QTest::newRow("-") << 0 << JobDataSet {};
    QTest::newRow("0") << 7 << JobDataSet {};
    QTest::newRow("A") << 1 << JobDataSet { { 5, ms(0) } };
    QTest::newRow("B") << 5 << JobDataSet { { 12, ms(1) } };
    QTest::newRow("C") << 1 << JobDataSet { { 1, ms(0) }, { 5, ms(2) }, { 3, ms(1) } };
    QTest::newRow("D") << 4 << JobDataSet { { 20, ms(1) }, { 8, ms(5) }, { 5, ms(2) } };
    QTest::newRow("E") << 2 << JobDataSet { { 1, ms(2) }, { 2, ms(1) } };
    QTest::newRow("F") << 3 << JobDataSet { { 8, ms(3) }, { 5, ms(4) }, { 2, ms(1) }, { 11, ms(1) }, { 100, ms(3) } };
}

void ConcurrentQueueTest::singleUserThread()
{
    QFETCH(const int, threadCount);
    QFETCH(const JobDataSet, jobs);

    ConcurrentQueue queue(threadCount);
    printStartedMessage(threadCount);

    Enqueuer(queue, total, jobs, primaryThreadId)();

    queue.waitAll();
    printWaitedMessage(threadCount);

    QCOMPARE(total.load(), expectedTotal(jobs));
}

void ConcurrentQueueTest::multipleUserThreads_data()
{
    QTest::addColumn<int>("threadCount");
    QTest::addColumn<QVector<JobDataSet>>("jobs");

    using ms = chrono::milliseconds;

    JobDataSet jobs1 { { 1, ms(1) } };
    JobDataSet jobs2 { { 2, ms(4) } };
    QVector<JobDataSet> allJobs { jobs1, jobs2 };
    QTest::newRow("A1") << 1 << allJobs;
    QTest::newRow("A2") << 2 << allJobs;

    jobs1.push_back({ 5, ms(3) });
    jobs2.push_back({ 10, ms(1) });
    allJobs = { jobs1, jobs2 };
    QTest::newRow("B1") << 2 << allJobs;
    QTest::newRow("B2") << 3 << allJobs;
    QTest::newRow("B3") << 8 << allJobs;

    jobs1.push_back({ 20, ms(0) });
    jobs2.push_back({ 40, ms(2) });
    allJobs = { jobs1, jobs2 };
    QTest::newRow("C") << 4 << allJobs;

    JobDataSet jobs3 { { 80, ms(0) }, { 160, ms(2) }, { 320, ms(1) }, { 640, ms(0) }, { 2000, ms(3) } };
    allJobs.push_back(jobs3);
    QTest::newRow("D1") << 3 << allJobs;
    QTest::newRow("D2") << 5 << allJobs;

    JobDataSet jobs4 { { 4000, ms(1) }, { 8000, ms(3) } };
    allJobs.push_back(jobs4);
    QTest::newRow("E1") << 4 << allJobs;
    QTest::newRow("E2") << 6 << allJobs;
}

void ConcurrentQueueTest::multipleUserThreads()
{
    QFETCH(const int, threadCount);
    QFETCH(const QVector<JobDataSet>, jobs);

    ConcurrentQueue queue(threadCount);
    printStartedMessage(threadCount);

    if (!jobs.empty()) {
        std::vector<std::thread> enqueuerThreads;
        enqueuerThreads.reserve(jobs.size() - 1);
        for (int i = 1; i < jobs.size(); ++i)
            enqueuerThreads.emplace_back(Enqueuer(queue, total, jobs.at(i), i));

        Enqueuer(queue, total, jobs.constFirst(), primaryThreadId)();
        for (auto &t : enqueuerThreads)
            t.join();
    }

    queue.waitAll();
    printWaitedMessage(threadCount);

    QCOMPARE(total.load(), expectedTotal(jobs));
}

QTEST_APPLESS_MAIN(ConcurrentQueueTest)

#include "concurrent_queue_test.moc"
