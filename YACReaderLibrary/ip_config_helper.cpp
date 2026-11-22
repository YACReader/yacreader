#include "ip_config_helper.h"

#include "qnaturalsorting.h"

#include <QNetworkInterface>
#include <QRegularExpression>
#include <QSet>

// 192.168 (most comon local subnet for ips are always put first)
// IPs are sorted using natoral sorting

QList<QString> getIpAddresses()
{
    auto ipComparator = [](const QString &ip1, const QString &ip2) {
        if (ip1.startsWith("192.168") && ip2.startsWith("192.168"))
            return naturalSortLessThanCI(ip1, ip2);

        if (ip1.startsWith("192.168"))
            return true;

        if (ip2.startsWith("192.168"))
            return false;

        return naturalSortLessThanCI(ip1, ip2);
    };

    QList<QString> addresses;
    const auto allAddresses = QNetworkInterface::allAddresses();
    for (const auto &add : allAddresses) {
        // Exclude loopback, local, multicast
        if (add.isGlobal()) {
            addresses.push_back(add.toString());
        }
    }

    std::sort(addresses.begin(), addresses.end(), ipComparator);
    return addresses;
}

bool isValidServerHost(const QString &host)
{
    if (host.isEmpty() || host.size() > 253)
        return false;

    // Dotted-quad IPv4
    static const QRegularExpression ipv4(QStringLiteral("^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$"));
    const auto ipv4Match = ipv4.match(host);
    if (ipv4Match.hasMatch()) {
        for (int i = 1; i <= 4; ++i) {
            if (ipv4Match.captured(i).toInt() > 255)
                return false;
        }
        return true;
    }

    // Hostname: labels of letters, digits and hyphens, without a leading/trailing hyphen
    static const QRegularExpression label(QStringLiteral("^[A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?$"));
    const auto labels = host.split(QLatin1Char('.'));
    for (const auto &part : labels) {
        if (!label.match(part).hasMatch())
            return false;
    }
    // A name made only of digits and dots is a malformed IPv4, not a hostname
    static const QRegularExpression digitsAndDots(QStringLiteral("^[0-9.]+$"));
    return !digitsAndDots.match(host).hasMatch();
}

QString resolveServerAddress(const QString &pinned, const QList<QString> &available, const QString &selected)
{
    if (!pinned.isEmpty())
        return pinned;
    if (!selected.isEmpty() && available.contains(selected))
        return selected;
    return available.isEmpty() ? QString() : available.first();
}

QStringList localHostNameCandidates(const QString &hostName, const QString &domainName)
{
    const QString fullName = hostName.trimmed();
    QString shortName = fullName;
    const int firstDot = shortName.indexOf(QLatin1Char('.'));
    if (firstDot >= 0)
        shortName.truncate(firstDot);

    // "localhost" only reaches this very machine, so it is useless to other devices
    if (shortName.isEmpty() || shortName.compare(QStringLiteral("localhost"), Qt::CaseInsensitive) == 0)
        return { };

    QStringList candidates { shortName + QStringLiteral(".local"), shortName };

    QString domain = domainName.trimmed();
    while (domain.startsWith(QLatin1Char('.')))
        domain.remove(0, 1);
    while (domain.endsWith(QLatin1Char('.')))
        domain.chop(1);
    if (!domain.isEmpty() && domain.compare(QStringLiteral("local"), Qt::CaseInsensitive) != 0)
        candidates.append(shortName + QLatin1Char('.') + domain);
    // Some systems already report the full name ("mac.lan")
    if (firstDot >= 0)
        candidates.append(fullName);

    QStringList result;
    QSet<QString> seen;
    for (const auto &candidate : std::as_const(candidates)) {
        if (isValidServerHost(candidate) && !seen.contains(candidate.toLower())) {
            seen.insert(candidate.toLower());
            result.append(candidate);
        }
    }
    return result;
}

bool resolvesToLocalAddress(const QList<QHostAddress> &resolved, const QList<QHostAddress> &localAddresses)
{
    for (const auto &address : resolved) {
        if (address.isNull() || address.isLoopback())
            continue;
        for (const auto &local : localAddresses) {
            if (address.isEqual(local, QHostAddress::TolerantConversion))
                return true;
        }
    }
    return false;
}
