#ifndef COVER_REQUEST_H
#define COVER_REQUEST_H

#include <QString>

#include <optional>

// the file of a cover request: `fileName` under the covers folder of the library; nothing when it resolves outside it
std::optional<QString> coverRequestFile(const QString &libraryPath, const QString &fileName);

#endif // COVER_REQUEST_H
