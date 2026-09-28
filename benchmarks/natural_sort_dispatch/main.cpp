#include <QCollator>
#include <QString>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif

static QCollator collator = [] {
    QCollator c;
    c.setNumericMode(true);
    c.setIgnorePunctuation(false);
    c.setCaseSensitivity(Qt::CaseInsensitive);
    return c;
}();

NOINLINE int primary(const QString &left, const QString &right)
{
    return collator.compare(left, right);
}

NOINLINE int secondary(const QString &left, const QString &right)
{
    return QString::compare(left, right, Qt::CaseInsensitive);
}

NOINLINE bool usePrimary()
{
    return std::getenv("BENCH_USE_SECONDARY") == nullptr;
}

NOINLINE int dispatch(const QString &left, const QString &right)
{
#if defined(DISPATCH_direct)
    return primary(left, right);
#elif defined(DISPATCH_indirect)
    using Compare = int (*)(const QString &, const QString &);
    static const Compare compare = usePrimary() ? primary : secondary;
    return compare(left, right);
#elif defined(DISPATCH_boolean)
    static const bool selected = usePrimary();
    return selected ? primary(left, right) : secondary(left, right);
#endif
}

int main(int argc, char **argv)
{
    const std::uint64_t iterations = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1000;
    std::vector<QString> names;
    names.reserve(1000);
    for (int series = 0; series < 10; ++series) {
        for (int issue = 0; issue < 100; ++issue) {
            const QString separator = issue % 3 == 0 ? QStringLiteral(".") : issue % 3 == 1 ? QStringLiteral("-")
                                                                                            : QStringLiteral(" ");
            names.push_back(QStringLiteral("Series ") + QString::number(series) + separator + QString::number(issue) + QStringLiteral(" (2024).cbz"));
        }
    }
    std::mt19937 random(12345);
    std::shuffle(names.begin(), names.end(), random);

    std::int64_t checksum = 0;
    checksum += dispatch(names[0], names[1]); // Initialize the selected path before timing.
    std::int64_t nanoseconds = 0;

    for (std::uint64_t i = 0; i < iterations; ++i) {
        auto values = names;
        const auto start = std::chrono::steady_clock::now();
        std::sort(values.begin(), values.end(), [](const QString &left, const QString &right) {
            return dispatch(left, right) < 0;
        });
        const auto end = std::chrono::steady_clock::now();
        nanoseconds += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        checksum += values.front().size() + values.back().size();
    }

    std::cout << (static_cast<double>(nanoseconds) / static_cast<double>(iterations)) << " ns/sort " << checksum << '\n';
}
