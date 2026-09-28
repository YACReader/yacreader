#include "qnaturalsorting.h"

#include <QCollator>

static QCollator collatorCI = [] {
    QCollator c;
    c.setNumericMode(true);
    c.setIgnorePunctuation(false);
    c.setCaseSensitivity(Qt::CaseInsensitive);
    return c;
}();

static QCollator collatorCS = [] {
    QCollator c;
    c.setNumericMode(true);
    c.setIgnorePunctuation(false);
    c.setCaseSensitivity(Qt::CaseSensitive);
    return c;
}();

bool naturalCollationIsSupported()
{
    static const bool supported = [] {
        // QCollator takes setNumericMode() and setCaseSensitivity() silently and then
        // ignores both when Qt has no ICU backend. Ask it two questions it can only
        // answer correctly with numeric mode and case folding in place.
        const bool numericMode = collatorCI.compare(QStringLiteral("a2"), QStringLiteral("a10")) < 0;
        const bool caseFolding = collatorCI.compare(QStringLiteral("a"), QStringLiteral("A")) == 0;

        if (!numericMode || !caseFolding) {
            qWarning("QCollator does not support natural sorting in this Qt build, most likely because Qt was built without ICU. Falling back to the built in comparison.");
        }

        return numericMode && caseFolding;
    }();

    return supported;
}

static char32_t codePointAt(QStringView text, qsizetype index, qsizetype &width)
{
    const QChar first = text.at(index);
    if (first.isHighSurrogate() && index + 1 < text.size() && text.at(index + 1).isLowSurrogate()) {
        width = 2;
        return QChar::surrogateToUcs4(first, text.at(index + 1));
    }

    width = 1;
    return first.unicode();
}

static int digitAt(QStringView text, qsizetype index, qsizetype &width)
{
    const char32_t codePoint = codePointAt(text, index, width);
    return QChar::isDigit(codePoint) ? QChar::digitValue(codePoint) : -1;
}

static qsizetype chunkEnd(QStringView text, qsizetype start, bool digits)
{
    qsizetype end = start;
    while (end < text.size()) {
        qsizetype width;
        if ((digitAt(text, end, width) >= 0) != digits) {
            break;
        }
        end += width;
    }
    return end;
}

static int compareDigitChunks(QStringView left, QStringView right)
{
    // Compare digit values without converting a potentially very long run to an integer.
    qsizetype leftStart = 0;
    qsizetype rightStart = 0;
    qsizetype width;
    while (leftStart < left.size() && digitAt(left, leftStart, width) == 0) {
        leftStart += width;
    }
    while (rightStart < right.size() && digitAt(right, rightStart, width) == 0) {
        rightStart += width;
    }

    qsizetype leftDigits = 0;
    qsizetype rightDigits = 0;
    for (qsizetype i = leftStart; i < left.size();) {
        digitAt(left, i, width);
        i += width;
        ++leftDigits;
    }
    for (qsizetype i = rightStart; i < right.size();) {
        digitAt(right, i, width);
        i += width;
        ++rightDigits;
    }
    if (leftDigits != rightDigits) {
        return leftDigits < rightDigits ? -1 : 1;
    }

    while (leftStart < left.size()) {
        qsizetype leftWidth;
        qsizetype rightWidth;
        const int leftDigit = digitAt(left, leftStart, leftWidth);
        const int rightDigit = digitAt(right, rightStart, rightWidth);
        if (leftDigit != rightDigit) {
            return leftDigit < rightDigit ? -1 : 1;
        }
        leftStart += leftWidth;
        rightStart += rightWidth;
    }
    return 0;
}

static int compareTextChunks(QStringView left, QStringView right, Qt::CaseSensitivity caseSensitivity)
{
    // The collator is still the best the platform can do for letters, even when it
    // dropped numeric mode. Do the case folding here, because it dropped that too.
    const int difference = (caseSensitivity == Qt::CaseInsensitive)
            ? collatorCS.compare(left.toString().toCaseFolded(), right.toString().toCaseFolded())
            : collatorCS.compare(left, right);

    return difference < 0 ? -1 : (difference > 0 ? 1 : 0);
}

int naturalCompareFallback(const QString &s1, const QString &s2, Qt::CaseSensitivity caseSensitivity)
{
    const QStringView left(s1);
    const QStringView right(s2);

    qsizetype i = 0;
    qsizetype j = 0;

    // Walk both strings one chunk at a time, a chunk being a run of digits or a run
    // of anything else.
    while (i < left.size() && j < right.size()) {
        qsizetype width;
        const bool leftIsNumber = digitAt(left, i, width) >= 0;
        const bool rightIsNumber = digitAt(right, j, width) >= 0;

        if (leftIsNumber != rightIsNumber) {
            return leftIsNumber ? -1 : 1;
        }

        const qsizetype leftEnd = chunkEnd(left, i, leftIsNumber);
        const qsizetype rightEnd = chunkEnd(right, j, rightIsNumber);

        const QStringView leftChunk = left.sliced(i, leftEnd - i);
        const QStringView rightChunk = right.sliced(j, rightEnd - j);

        const int difference = leftIsNumber
                ? compareDigitChunks(leftChunk, rightChunk)
                : compareTextChunks(leftChunk, rightChunk, caseSensitivity);

        if (difference != 0) {
            return difference;
        }

        i = leftEnd;
        j = rightEnd;
    }

    if (i < left.size()) {
        return 1;
    }
    if (j < right.size()) {
        return -1;
    }

    // Every chunk matched but the strings can still differ, as in "007" and "7" or
    // in "ABC" and "abc". Keep a fixed order between them, so that sorting the same
    // list twice gives the same result.
    const int raw = QString::compare(s1, s2, Qt::CaseSensitive);
    return raw < 0 ? -1 : (raw > 0 ? 1 : 0);
}

static int naturalCompareCollator(const QString &s1, const QString &s2, Qt::CaseSensitivity caseSensitivity)
{
    QCollator &c = (caseSensitivity == Qt::CaseSensitive) ? collatorCS : collatorCI;
    return c.compare(s1, s2);
}

int naturalCompare(const QString &s1, const QString &s2, Qt::CaseSensitivity caseSensitivity)
{
    using Compare = int (*)(const QString &, const QString &, Qt::CaseSensitivity);
    static const Compare compare = naturalCollationIsSupported() ? naturalCompareCollator : naturalCompareFallback;
    return compare(s1, s2, caseSensitivity);
}
bool naturalSortLessThanCS(const QString &left, const QString &right)
{
    return (naturalCompare(left, right, Qt::CaseSensitive) < 0);
}

bool naturalSortLessThanCI(const QString &left, const QString &right)
{
    return (naturalCompare(left, right, Qt::CaseInsensitive) < 0);
}

bool comicNumberLessThan(const QVariant &leftNumber, const QString &leftName,
                         const QVariant &rightNumber, const QString &rightName)
{
    if (leftNumber.isNull() && rightNumber.isNull())
        return naturalSortLessThanCI(leftName, rightName);

    if (!leftNumber.isNull() && !rightNumber.isNull())
        return naturalSortLessThanCI(leftNumber.toString(), rightNumber.toString());

    return rightNumber.isNull();
}

bool naturalSortLessThanCIFileInfo(const QFileInfo &left, const QFileInfo &right)
{
    return naturalSortLessThanCI(left.fileName(), right.fileName());
}

bool naturalSortLessThanCILibraryItem(LibraryItem *left, LibraryItem *right)
{
    return naturalSortLessThanCI(left->name, right->name);
}
