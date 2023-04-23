#include <QNetworkInterface>
#include "ip_config_helper.h"
#include "qnaturalsorting.h"

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
    for (auto add : QNetworkInterface::allAddresses()) {
        // Exclude loopback, local, multicast
        if (add.isGlobal()) {
            addresses.push_back(add.toString());
        }
    }

    std::sort(addresses.begin(), addresses.end(), ipComparator);
    return addresses;
}
