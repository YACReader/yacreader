#include "ip_config_helper.h"

#include <QHostAddress>
#include <QObject>
#include <QStringList>
#include <QTest>

class IpConfigHelperTest : public QObject
{
    Q_OBJECT

private slots:
    void acceptsIpv4Addresses();
    void rejectsMalformedIpv4Addresses();
    void acceptsHostnames();
    void rejectsInvalidHostnames();
    void candidatesCoverMdnsDnsAndShortName();
    void candidatesHandleFullyQualifiedNames();
    void candidatesRejectUnusableNames();
    void resolvedNameMustBelongToThisComputer();
    void pinnedAddressAlwaysWins();
    void selectedAddressIsKeptWhileAvailable();
    void fallsBackToTheFirstAvailableAddress();
    void returnsEmptyWithoutAnyAddress();
};

void IpConfigHelperTest::acceptsIpv4Addresses()
{
    QVERIFY(isValidServerHost("192.168.1.100"));
    QVERIFY(isValidServerHost("10.0.0.1"));
    QVERIFY(isValidServerHost("255.255.255.255"));
}

void IpConfigHelperTest::rejectsMalformedIpv4Addresses()
{
    QVERIFY(!isValidServerHost("256.1.1.1"));
    QVERIFY(!isValidServerHost("192.168.1"));
    QVERIFY(!isValidServerHost("192.168.1.1.1"));
    QVERIFY(!isValidServerHost("192.168.1."));
}

void IpConfigHelperTest::acceptsHostnames()
{
    QVERIFY(isValidServerHost("my-mac"));
    QVERIFY(isValidServerHost("my-mac.local"));
    QVERIFY(isValidServerHost("server1.home.example.com"));
}

void IpConfigHelperTest::rejectsInvalidHostnames()
{
    QVERIFY(!isValidServerHost(""));
    QVERIFY(!isValidServerHost(" 192.168.1.100"));
    QVERIFY(!isValidServerHost("my mac"));
    QVERIFY(!isValidServerHost("-mac.local"));
    QVERIFY(!isValidServerHost("mac-.local"));
    QVERIFY(!isValidServerHost("mac..local"));
    QVERIFY(!isValidServerHost("http://mac"));
    QVERIFY(!isValidServerHost("mac:8080"));
    QVERIFY(!isValidServerHost(QString(64, 'a')));
}

void IpConfigHelperTest::candidatesCoverMdnsDnsAndShortName()
{
    QCOMPARE(localHostNameCandidates("my-pc", "lan"),
             (QStringList { "my-pc.local", "my-pc", "my-pc.lan" }));
    // No DNS domain: only the mDNS and the short name
    QCOMPARE(localHostNameCandidates("my-pc", QString()),
             (QStringList { "my-pc.local", "my-pc" }));
    // The ".local" domain is not repeated, and stray dots/spaces are cleaned up
    QCOMPARE(localHostNameCandidates("  my-pc ", ".local."),
             (QStringList { "my-pc.local", "my-pc" }));
}

void IpConfigHelperTest::candidatesHandleFullyQualifiedNames()
{
    // macOS/Linux often report "name.local" or "name.lan" as the host name
    QCOMPARE(localHostNameCandidates("My-Mac.local", QString()),
             (QStringList { "My-Mac.local", "My-Mac" }));
    QCOMPARE(localHostNameCandidates("nas.lan", "lan"),
             (QStringList { "nas.local", "nas", "nas.lan" }));
    // Duplicates differing only by case are dropped
    QCOMPARE(localHostNameCandidates("MyPC", "LOCAL").size(), 2);
}

void IpConfigHelperTest::candidatesRejectUnusableNames()
{
    QVERIFY(localHostNameCandidates("", "lan").isEmpty());
    QVERIFY(localHostNameCandidates("localhost", "lan").isEmpty());
    QVERIFY(localHostNameCandidates("LOCALHOST.localdomain", QString()).isEmpty());
    QVERIFY(localHostNameCandidates("Zander's Mac", QString()).isEmpty());
}

void IpConfigHelperTest::resolvedNameMustBelongToThisComputer()
{
    const QList<QHostAddress> local { QHostAddress("192.168.1.100"), QHostAddress("::1"), QHostAddress("127.0.0.1") };
    QVERIFY(resolvesToLocalAddress({ QHostAddress("192.168.1.100") }, local));
    QVERIFY(resolvesToLocalAddress({ QHostAddress("10.9.9.9"), QHostAddress("192.168.1.100") }, local));
    // Another machine answered (stale DNS record, name collision)
    QVERIFY(!resolvesToLocalAddress({ QHostAddress("192.168.1.77") }, local));
    // "127.0.1.1 myhost" in /etc/hosts says nothing about the network
    QVERIFY(!resolvesToLocalAddress({ QHostAddress("127.0.1.1") }, local));
    QVERIFY(!resolvesToLocalAddress({ QHostAddress("127.0.0.1") }, local));
    QVERIFY(!resolvesToLocalAddress({ }, local));
}

void IpConfigHelperTest::pinnedAddressAlwaysWins()
{
    const QStringList available { "192.168.1.100", "10.0.0.5" };
    QCOMPARE(resolveServerAddress("192.168.1.50", available, "10.0.0.5"), QString("192.168.1.50"));
    // Pinned even when this machine has no interface on that network right now
    QCOMPARE(resolveServerAddress("my-mac.local", { }, QString()), QString("my-mac.local"));
}

void IpConfigHelperTest::selectedAddressIsKeptWhileAvailable()
{
    const QStringList available { "192.168.1.100", "10.0.0.5" };
    QCOMPARE(resolveServerAddress(QString(), available, "10.0.0.5"), QString("10.0.0.5"));
}

void IpConfigHelperTest::fallsBackToTheFirstAvailableAddress()
{
    const QStringList available { "192.168.1.100", "10.0.0.5" };
    // Selected address disappeared (DHCP, other network)
    QCOMPARE(resolveServerAddress(QString(), available, "172.16.0.9"), QString("192.168.1.100"));
    QCOMPARE(resolveServerAddress(QString(), available, QString()), QString("192.168.1.100"));
}

void IpConfigHelperTest::returnsEmptyWithoutAnyAddress()
{
    QVERIFY(resolveServerAddress(QString(), { }, "10.0.0.5").isEmpty());
}

QTEST_GUILESS_MAIN(IpConfigHelperTest)

#include "main.moc"
