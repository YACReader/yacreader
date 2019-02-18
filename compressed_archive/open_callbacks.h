#ifndef OPEN_CALLBACKS_H
#define OPEN_CALLBACKS_H

#include "7z_includes.h"
#include <QDebug>
//////////////////////////////////////////////////////////////
// Archive Open callback class


class YCArchiveOpenCallback:
  public IArchiveOpenCallback,
  public ICryptoGetTextPassword,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

  STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes);
  STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes);

  STDMETHOD(CryptoGetTextPassword)(BSTR *password);

  bool PasswordIsDefined;
  UString Password;

  YCArchiveOpenCallback() : PasswordIsDefined(false) {}
};

STDMETHODIMP YCArchiveOpenCallback::SetTotal(const UInt64 * /* files */, const UInt64 * /* bytes */)
{
  return S_OK;
}

STDMETHODIMP YCArchiveOpenCallback::SetCompleted(const UInt64 * /* files */, const UInt64 * /* bytes */)
{
  return S_OK;
}
  
STDMETHODIMP YCArchiveOpenCallback::CryptoGetTextPassword(BSTR *password)
{
  if (!PasswordIsDefined)
  {
    // You can ask real password here from user
    // Password = GetPassword(OutStream);
    // PasswordIsDefined = true;
    qDebug() << "Password is not defined" << endl;
    return E_ABORT;
  }
  return StringToBstr(Password, password);
}



#endif
