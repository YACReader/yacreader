#ifndef OPEN_CALLBACKS_H
#define OPEN_CALLBACKS_H

#include "7z_includes.h"
#include <QDebug>
//////////////////////////////////////////////////////////////
// Archive Open callback class

class YCArchiveOpenCallback : public IArchiveOpenCallback,
                              public ICryptoGetTextPassword,
                              public CMyUnknownImp
{
    Z7_IFACES_IMP_UNK_2(IArchiveOpenCallback, ICryptoGetTextPassword)
public:
    bool PasswordIsDefined;
    UString Password;

    YCArchiveOpenCallback()
        : PasswordIsDefined(false) { }
};

Z7_COM7F_IMF(YCArchiveOpenCallback::SetTotal(const UInt64 * /* files */, const UInt64 * /* bytes */))
{
    return S_OK;
}

Z7_COM7F_IMF(YCArchiveOpenCallback::SetCompleted(const UInt64 * /* files */, const UInt64 * /* bytes */))
{
    return S_OK;
}

Z7_COM7F_IMF(YCArchiveOpenCallback::CryptoGetTextPassword(BSTR *password))
{
    if (!PasswordIsDefined) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        qDebug() << "Password is not defined" << Qt::endl;
        return E_ABORT;
    }
    return StringToBstr(Password, password);
}

#endif
