#pragma once
#include <Windows.h>
#include <string>

namespace x86
{
  class Register
  {
  public:
    Register(DWORD &dwValue);

    bool isBadCodePtr() const;
    bool isBadReadPtr(size_t size = 4) const;
    bool isBadWritePtr(size_t size = 4) const;
    bool isBadStringPtr(size_t size = 0x7FFF) const;

    bool isOnStack() const;

    Register &operator =(const Register &other);

    template <typename T>
    Register &operator =(const T &val)
    {
      this->value = (DWORD)val;
      return *this;
    }
    Register &operator +=(const DWORD &val);
    Register &operator -=(const DWORD &val);
    Register &operator ++();
    Register &operator --();

    operator unsigned int() const;
    operator void*() const;

    DWORD &value;
  };

  class ExceptionContext
  {
  public:
    ExceptionContext(EXCEPTION_POINTERS *pEP);

    void push(const DWORD &v);
    template < typename T >
    void pop(T &v)
    {
      v = *(DWORD*)(ep->ContextRecord->Esp);
      ep->ContextRecord->Esp += 4;
    };
    DWORD pop();

    const char * const getName() const;
  private:
    EXCEPTION_POINTERS *ep;
  };

}
