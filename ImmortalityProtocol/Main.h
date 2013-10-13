#include <Windows.h>

class TopLevelExceptionFilter;

#define ErrBox(x) MessageBox(nullptr, x, "Immortality Protocol", MB_ICONERROR | MB_OK)

inline ULARGE_INTEGER make_ularge(const FILETIME &ft)
{
  ULARGE_INTEGER ret = { ft.dwLowDateTime, ft.dwHighDateTime };
  return ret;
}

extern TopLevelExceptionFilter TopExceptionFilter;