#pragma once
#include <Windows.h>

class TopLevelExceptionFilter
{
public:
  TopLevelExceptionFilter();
  TopLevelExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpNewExceptionFilter);
  ~TopLevelExceptionFilter();
  
  LONG DefFilterProc(EXCEPTION_POINTERS *ep);
private:
  LPTOP_LEVEL_EXCEPTION_FILTER pOldExceptionFilter;
};
