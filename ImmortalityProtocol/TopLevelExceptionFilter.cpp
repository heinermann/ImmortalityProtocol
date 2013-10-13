#include "TopLevelExceptionFilter.h"
#include "Main.h"

TopLevelExceptionFilter::TopLevelExceptionFilter()
: pOldExceptionFilter(NULL)
{
}

TopLevelExceptionFilter::TopLevelExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpNewExceptionFilter)
: pOldExceptionFilter(NULL)
{
  if ( lpNewExceptionFilter )
    pOldExceptionFilter = SetUnhandledExceptionFilter(lpNewExceptionFilter);
}

TopLevelExceptionFilter::~TopLevelExceptionFilter()
{
  if ( pOldExceptionFilter )
    SetUnhandledExceptionFilter(pOldExceptionFilter);
}

LONG TopLevelExceptionFilter::DefFilterProc(EXCEPTION_POINTERS *ep)
{
  if ( pOldExceptionFilter )
    return pOldExceptionFilter(ep);
  return EXCEPTION_CONTINUE_SEARCH;
}
