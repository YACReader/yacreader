#include "concurrent_queue.h"

#include <QDebug>
#include <QDebugStateSaver>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QTest>
#include <QTime>
#include <QVector>

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <numeric>
#include <random>
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

//! This test prints thousands of lines of detailed output. The output allows to analyze
//! how ConcurrentQueue is being tested, how it works and why the test fails or crashes
//! (normally it passes). The default maximum number of warnings in Qt Test is 2000,
//! which is too low for this test. Therefore, the following warning is printed before
//! the log output is suppressed: "Maximum amount of warnings exceeded. Use -maxwarnings
//! to override.". Passing `-maxwarnings 100000` command line option to the test lets it
//! print everything. Passing -silent command line option to the test suppresses all its
//! output except for RandomEngineProvider's root seeds, which are necessary to reproduce
//! interesting test results.
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

int expectedTotal(JobDataSet::const_iterator first, JobDataSet::const_iterator last)
{
    return std::accumulate(first, last, 0,
                           [](int total, JobData job) {
                               return total + job.summand;
                           });
}

int expectedTotal(const JobDataSet &jobs)
{
    return expectedTotal(jobs.cbegin(), jobs.cend());
}

int expectedTotal(const JobDataSet &jobs, std::size_t canceledCount)
{
    const auto count = jobs.size() - static_cast<int>(canceledCount);
    if (count < 0)
        qFatal("Canceled more than the total number of jobs somehow!");
    return expectedTotal(jobs.cbegin(), jobs.cbegin() + count);
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

class QueueControlMessagePrinter
{
public:
    explicit QueueControlMessagePrinter(const Total &total, int threadId, int threadCount)
        : total { total }, threadId { threadId }, threadCount { threadCount } { }

    void printStartedMessage() const
    {
        log() << threadMessageFormatString().arg("started");
    }
    void printCanceledMessage(std::size_t canceledCount) const
    {
        const char *const jobStr = canceledCount == 1 ? "job" : "jobs";
        const auto format = messageFormatString().arg("%1 %2 %3");
        log() << format.arg("canceled").arg(canceledCount).arg(jobStr);
    }
    void printBeginWaitingMessage() const
    {
        log() << threadMessageFormatString().arg("begin waiting for");
    }
    void printEndWaitingMessage() const
    {
        log() << threadMessageFormatString().arg("end waiting for");
    }

private:
    QString messageFormatString() const
    {
        return QStringLiteral("#%1 %3 => %2").arg(threadId).arg(total.load());
    }

    QString threadMessageFormatString() const
    {
        const char *const threadStr = threadCount == 1 ? "thread" : "threads";
        const auto format = messageFormatString().arg("%3 %1 %2");
        return format.arg(threadCount).arg(threadStr);
    }

    const Total &total;
    const int threadId;
    const int threadCount;
};

std::size_t cancelAndPrint(ConcurrentQueue &queue, const QueueControlMessagePrinter &printer)
{
    const auto canceledCount = queue.cancelPending();
    printer.printCanceledMessage(canceledCount);
    return canceledCount;
}

void waitAndPrint(ConcurrentQueue &queue, const QueueControlMessagePrinter &printer)
{
    printer.printBeginWaitingMessage();
    queue.waitAll();
    printer.printEndWaitingMessage();
}

template<typename T, std::size_t size>
QDebug operator<<(QDebug debug, const std::array<T, size> &array)
{
    QDebugStateSaver saver(debug);
    debug.nospace();

    debug << '(';
    if (size != 0) {
        debug << array.front();
        for (std::size_t i = 1; i != size; ++i)
            debug << ", " << array[i];
    }
    debug << ')';

    return debug;
}

using RandomEngine = std::mt19937_64;

class RandomEngineProvider
{
public:
    RandomEngineProvider()
    {
        std::random_device rd;
        const auto randomValues = generate<rootSeedCount>(rd);
        // Qt Test does not suppress output from the constructor of a test class
        // even when -silent command line option is passed. This is fortunate
        // because the root seeds can be used to reproduce a test failure.
        log() << "RandomEngineProvider's root seeds:" << randomValues;
        std::seed_seq seedSeq(randomValues.begin(), randomValues.end());
        rootEngine.reset(new std::mt19937(seedSeq));
    }

    void resetEngines(std::size_t engineCount)
    {
        engines.clear();
        engines.reserve(engineCount);
        for (; engineCount != 0; --engineCount) {
            const auto randomValues = generate<seedCount>(*rootEngine);
            std::seed_seq seedSeq(randomValues.begin(), randomValues.end());
            engines.emplace_back(seedSeq);
        }
    }

    RandomEngine &engine(std::size_t index)
    {
        return engines.at(index);
    }

private:
    // In this test we don't really care about uniformly choosing an initial state
    // from the entire state-space of the engine. It is possible to generate more
    // random numbers at the cost of performance and system entropy pool exhaustion.
    static constexpr std::size_t rootSeedCount { 8 };
    static constexpr std::size_t seedCount { 32 };

    template<std::size_t size, typename Generator>
    static std::array<std::uint32_t, size> generate(Generator &generator)
    {
        std::array<std::uint32_t, size> result;
        for (auto &value : result)
            value = generator();
        return result;
    }

    std::unique_ptr<std::mt19937> rootEngine;
    std::vector<RandomEngine> engines;
};

//! Calls random member functions of ConcurrentQueue for a limited time.
//! Ensures that total equals 0 when all jobs are complete/canceled by:
//! * setting each job's summand to 1;
//! * subtracting a job set's size from total before enqueuing jobs in the set;
//! * adding canceled job count to total after cancelation.
class RandomCaller
{
public:
    explicit RandomCaller(ConcurrentQueue &queue, Total &total, int threadId,
                          int queueThreadCount, RandomEngine &engine,
                          bool boostEnqueueOperationWeight)
        : queue(queue),
          total(total),
          threadId { threadId },
          boostEnqueueOperationWeight { boostEnqueueOperationWeight },
          printer(total, threadId, queueThreadCount),
          engine(engine)
    {
    }

    void operator()()
    {
        constexpr auto testDuration = chrono::milliseconds(10);
        const auto testStartTime = Clock::now();

        auto operation = operationDistribution();
        do {
            switch (operation(engine)) {
            case 0:
                enqueue();
                break;
            case 1:
                cancel();
                break;
            case 2:
                waitAndPrint(queue, printer);
                break;
            default:
                qFatal("Unsupported operation.");
            }
        } while (Clock::now() - testStartTime < testDuration);
    }

private:
    int randomInt(int a, int b)
    {
        return uniformInt(engine, decltype(uniformInt)::param_type(a, b));
    }

    using OperationDistribution = std::discrete_distribution<int>;

    void printProbabilities(const OperationDistribution &distribution) const
    {
        auto p = distribution.probabilities();
        constexpr std::size_t expectedProbabilityCount { 3 };
        if (p.size() != expectedProbabilityCount)
            qFatal("Wrong number of operation probabilities: %zu != %zu", p.size(), expectedProbabilityCount);

        for (auto &x : p)
            x *= 100; // Convert to percentages.
        log() << QStringLiteral("#%1 operation probabilities: e=%2%, c=%3%, w=%4%.").arg(threadId).arg(p[0]).arg(p[1]).arg(p[2]);
    }

    OperationDistribution operationDistribution()
    {
        auto distribution = boostEnqueueOperationWeight ? boostedEnqueueOperationDistribution()
                                                        : almostUniformOperationDistribution();
        printProbabilities(distribution);
        return distribution;
    }

    OperationDistribution almostUniformOperationDistribution()
    {
        constexpr int sumOfProbabilities { 100 };
        const auto enqueueProbability = randomInt(0, sumOfProbabilities);
        const auto cancelProbability = randomInt(0, sumOfProbabilities - enqueueProbability);
        const auto waitProbability = sumOfProbabilities - enqueueProbability - cancelProbability;
        if (enqueueProbability + cancelProbability + waitProbability != sumOfProbabilities)
            qFatal("The sum of probabilities is not 100%%.");

        const auto real = [](int x) { return static_cast<double>(x); };
        return { real(enqueueProbability), real(cancelProbability), real(waitProbability) };
    }

    OperationDistribution boostedEnqueueOperationDistribution()
    {
        // Make enqueue the most frequent operation to stress-test executing
        // jobs rather than canceling them almost immediately.
        const auto enqueueWeight = std::lognormal_distribution<double>(2, 0.5)(engine);
        const auto cancelWeight = 1.0;
        // Waiting is uninteresting as it doesn't even modify the queue => make it rare.
        const auto waitWeight = std::uniform_real_distribution<double>(0, 0.2)(engine);

        return { enqueueWeight, cancelWeight, waitWeight };
    }

    JobDataSet createJobs()
    {
        constexpr int minJobCount { 1 }, maxJobCount { 5 };
        JobDataSet jobs(randomInt(minJobCount, maxJobCount));
        for (auto &job : jobs) {
            constexpr int minSleepingTime { 0 }, maxSleepingTime { 5 };
            const auto sleepingTime = randomInt(minSleepingTime, maxSleepingTime);
            job = { 1, sleepingTime * chrono::microseconds(1) };
        }
        return jobs;
    }

    void enqueue()
    {
        const auto jobs = createJobs();
        total -= jobs.size();
        Enqueuer(queue, total, jobs, threadId)();
    }

    void cancel()
    {
        const auto canceledCount = cancelAndPrint(queue, printer);
        total += canceledCount;
    }

    ConcurrentQueue &queue;
    Total &total;
    const int threadId;
    const bool boostEnqueueOperationWeight;
    const QueueControlMessagePrinter printer;
    RandomEngine &engine;
    std::uniform_int_distribution<int> uniformInt;
};

}

Q_DECLARE_METATYPE(Clock::duration)
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

    void cancelPending1UserThread_data();
    void cancelPending1UserThread();

    void waitAllFromMultipleThreads_data();
    void waitAllFromMultipleThreads();

    void randomCalls_data();
    void randomCalls();

private:
    static constexpr int primaryThreadId { 0 };

    QueueControlMessagePrinter makeMessagePrinter(int threadCount) const
    {
        return QueueControlMessagePrinter(total, primaryThreadId, threadCount);
    }

    Total total { 0 };
    RandomEngineProvider randomEngineProvider;
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

    const auto printer = makeMessagePrinter(threadCount);

    ConcurrentQueue queue(threadCount);
    printer.printStartedMessage();

    Enqueuer(queue, total, jobs, primaryThreadId)();

    waitAndPrint(queue, printer);

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

    const auto printer = makeMessagePrinter(threadCount);

    ConcurrentQueue queue(threadCount);
    printer.printStartedMessage();

    if (!jobs.empty()) {
        std::vector<std::thread> enqueuerThreads;
        enqueuerThreads.reserve(jobs.size() - 1);
        for (int i = 1; i < jobs.size(); ++i)
            enqueuerThreads.emplace_back(Enqueuer(queue, total, jobs.at(i), i));

        Enqueuer(queue, total, jobs.constFirst(), primaryThreadId)();
        for (auto &t : enqueuerThreads)
            t.join();
    }

    waitAndPrint(queue, printer);

    QCOMPARE(total.load(), expectedTotal(jobs));
}

void ConcurrentQueueTest::cancelPending1UserThread_data()
{
    QTest::addColumn<int>("threadCount");
    QTest::addColumn<JobDataSet>("jobs");
    QTest::addColumn<Clock::duration>("cancelDelay");

    const auto ms = [](int count) -> Clock::duration { return chrono::milliseconds(count); };
    const auto us = [](int count) -> Clock::duration { return chrono::microseconds(count); };

    QTest::newRow("-") << 0 << JobDataSet {} << ms(0);
    QTest::newRow("01") << 2 << JobDataSet {} << ms(0);
    QTest::newRow("02") << 3 << JobDataSet {} << ms(1);
    QTest::newRow("A") << 1 << JobDataSet { { 5, ms(3) } } << ms(1);
    QTest::newRow("B") << 5 << JobDataSet { { 12, ms(1) } } << ms(1);

    JobDataSet dataSet { { 1, ms(3) }, { 5, ms(2) }, { 3, ms(1) } };
    QTest::newRow("C1") << 1 << dataSet << ms(1);
    QTest::newRow("C2") << 1 << dataSet << ms(4);
    QTest::newRow("C3") << 2 << dataSet << ms(1);
    QTest::newRow("C4") << 3 << dataSet << ms(1);
    QTest::newRow("C5") << 1 << dataSet << ms(7);

    dataSet.push_back({ 10, ms(5) });
    dataSet.push_back({ 20, ms(8) });
    dataSet.push_back({ 40, ms(20) });
    dataSet.push_back({ 80, ms(2) });
    QTest::newRow("D1") << 1 << dataSet << ms(1);
    QTest::newRow("D2") << 1 << dataSet << ms(15);
    QTest::newRow("D3") << 1 << dataSet << ms(50);
    QTest::newRow("D4") << 2 << dataSet << ms(4);
    QTest::newRow("D5") << 3 << dataSet << ms(4);
    QTest::newRow("D6") << 4 << dataSet << ms(4);
    QTest::newRow("D7") << 2 << dataSet << us(300);
    QTest::newRow("D8") << 3 << dataSet << us(500);
    QTest::newRow("D9") << 4 << dataSet << us(700);

    QTest::newRow("E") << 4 << JobDataSet { { 20, ms(1) }, { 8, ms(5) }, { 5, ms(2) } } << ms(1);
}

void ConcurrentQueueTest::cancelPending1UserThread()
{
    QFETCH(const int, threadCount);
    QFETCH(const JobDataSet, jobs);
    QFETCH(const Clock::duration, cancelDelay);

    const auto printer = makeMessagePrinter(threadCount);

    ConcurrentQueue queue(threadCount);
    printer.printStartedMessage();

    Enqueuer(queue, total, jobs, primaryThreadId)();

    std::this_thread::sleep_for(cancelDelay);
    const auto canceledCount = cancelAndPrint(queue, printer);
    QVERIFY(canceledCount <= static_cast<std::size_t>(jobs.size()));

    waitAndPrint(queue, printer);

    QCOMPARE(total.load(), expectedTotal(jobs, canceledCount));
}

void ConcurrentQueueTest::waitAllFromMultipleThreads_data()
{
    QTest::addColumn<int>("waitingThreadCount");
    for (int i : { 1, 2, 4, 7, 19 })
        QTest::addRow("%d", i) << i;
}

void ConcurrentQueueTest::waitAllFromMultipleThreads()
{
    QFETCH(const int, waitingThreadCount);
    QVERIFY(waitingThreadCount > 0);

    constexpr auto queueThreadCount = 2;
    const auto printer = makeMessagePrinter(queueThreadCount);

    ConcurrentQueue queue(queueThreadCount);
    printer.printStartedMessage();

    using ms = chrono::milliseconds;
    const JobDataSet jobs { { 5, ms(1) }, { 7, ms(2) } };
    Enqueuer(queue, total, jobs, primaryThreadId)();

    std::vector<std::thread> waitingThreads;
    waitingThreads.reserve(waitingThreadCount - 1);
    for (int id = 1; id < waitingThreadCount; ++id) {
        waitingThreads.emplace_back([=, &queue] {
            waitAndPrint(queue, QueueControlMessagePrinter(total, id, queueThreadCount));
        });
    }

    waitAndPrint(queue, printer);

    for (auto &t : waitingThreads)
        t.join();

    QCOMPARE(total.load(), expectedTotal(jobs));
}

void ConcurrentQueueTest::randomCalls_data()
{
    QTest::addColumn<int>("queueThreadCount");
    QTest::addColumn<int>("userThreadCount");
    QTest::addColumn<bool>("boostEnqueueOperationWeight");

    const auto suffix = [](bool boost) { return boost ? " +enqueue" : ""; };

    for (bool boost : { false, true })
        for (int q : { 1, 2, 4, 9, 12, 20 })
            for (int u : { 1, 2, 4, 7, 11, 18 })
                QTest::addRow("queue{%d}; %d user thread(s)%s", q, u, suffix(boost)) << q << u << boost;
}

void ConcurrentQueueTest::randomCalls()
{
    QFETCH(const int, queueThreadCount);
    QFETCH(const int, userThreadCount);
    QVERIFY(userThreadCount > 0);
    QFETCH(const bool, boostEnqueueOperationWeight);

    const auto printer = makeMessagePrinter(queueThreadCount);

    ConcurrentQueue queue(queueThreadCount);
    printer.printStartedMessage();

    randomEngineProvider.resetEngines(userThreadCount);

    std::vector<std::thread> userThreads;
    userThreads.reserve(userThreadCount - 1);
    for (int id = 1; id < userThreadCount; ++id) {
        userThreads.emplace_back(RandomCaller(queue, total, id, queueThreadCount,
                                              randomEngineProvider.engine(id),
                                              boostEnqueueOperationWeight));
    }
    RandomCaller(queue, total, primaryThreadId, queueThreadCount,
                 randomEngineProvider.engine(primaryThreadId),
                 boostEnqueueOperationWeight)();

    for (auto &t : userThreads)
        t.join();

    waitAndPrint(queue, printer);

    QCOMPARE(total.load(), 0);
}

QTEST_APPLESS_MAIN(ConcurrentQueueTest)

#include "concurrent_queue_test.moc"
