#ifndef YR_IP_CONFIG_HELPER
#define YR_IP_CONFIG_HELPER

#include <QHostAddress>
#include <QList>
#include <QString>
#include <QStringList>

QList<QString> getIpAddresses();

// True if `host` is a valid IPv4 address ("192.168.1.100") or a hostname
// (RFC 1123 labels, e.g. "my-mac.local"). Surrounding spaces are not allowed.
bool isValidServerHost(const QString &host);

// Address advertised to clients (QR code, web UI link).
// A pinned address always wins, even if it is not among the currently available ones
// (e.g. the machine is on another network). Otherwise the selected address is kept
// while it is still available, and the first available one is used as a fallback.
// Names other devices could use to reach this computer, best first:
// "<name>.local" (mDNS: macOS/iOS, Windows 10+, Linux with avahi or systemd-resolved),
// "<name>" (router DNS / NetBIOS / LLMNR) and "<name>.<domain>" (DNS domain of the network).
// Unlike an IP they survive a DHCP change. These are only *candidates*: whether each one
// really resolves must be checked with a lookup (see resolvesToLocalAddress).
QStringList localHostNameCandidates(const QString &hostName, const QString &domainName);

// True if a lookup of one of our names answered with an address that belongs to this
// computer (loopback does not count: "127.0.1.1 myhost" in /etc/hosts proves nothing).
bool resolvesToLocalAddress(const QList<QHostAddress> &resolved, const QList<QHostAddress> &localAddresses);

QString resolveServerAddress(const QString &pinned, const QList<QString> &available, const QString &selected);
#endif
