#include "CallbackManager.h"

#include <Windows.h>
#include <new.h>
#include <eh.h>
#include <csignal>
#include <cstdlib>
#include <string>
#include <sstream>

#include "ImmortalExceptionFilter.h"
#include "Main.h"

int immortal_new_handler(size_t size)
{
  writeExceptionCount();
  ErrBox("Ran out of memory. A garbage collection feature is not yet implemented.");
  return 0;
}
void immortal_purecall_handler()
{
  writeExceptionCount();
  ErrBox("Purecall handler. Not yet implemented.");
}
void immortal_terminate_handler()
{
  writeExceptionCount();
  ErrBox("Terminate handler. Not yet implemented.");
  abort();
}
void immortal_unexpected_handler()
{
  writeExceptionCount();
  ErrBox("Unexpected handler. Not yet implemented.");
  terminate();
}
void immortal_invalid_parameter_handler(const wchar_t *expr, const wchar_t *func, const wchar_t *file, unsigned int line, uintptr_t pReserved)
{
  writeExceptionCount();
  ErrBox("Invalid parameter handler. Not yet implemented.");
}
void immortal_sigabrt_handler(int sig)
{
  writeExceptionCount();
  ErrBox("Abort signal.");
}
void immortal_unused_sig_handler(int sig)
{
  std::stringstream ss("Unused signal: ");
  ss << sig;
  ErrBox(ss.str().c_str());
}

CallbackManager<_PNH,_set_new_handler> newHandler(&immortal_new_handler);
CallbackManager<_purecall_handler,_set_purecall_handler> purecallHandler(&immortal_purecall_handler);
CallbackManager<terminate_function,set_terminate> terminateHandler(&immortal_terminate_handler);
CallbackManager<unexpected_function,set_unexpected> unexpectedHandler(&immortal_unexpected_handler);
CallbackManager<_invalid_parameter_handler,_set_invalid_parameter_handler> invalidParamHandler(&immortal_invalid_parameter_handler);
SignalManager<SIGINT> sigintHandler(&immortal_unused_sig_handler);
SignalManager<SIGILL> sigillHandler(&immortal_unused_sig_handler);
SignalManager<SIGFPE> sigfpeHandler(&immortal_unused_sig_handler);
SignalManager<SIGSEGV> sigsegvHandler(&immortal_unused_sig_handler);
SignalManager<SIGTERM> sigtermHandler(&immortal_unused_sig_handler);
SignalManager<SIGBREAK> sigbreakHandler(&immortal_unused_sig_handler);
SignalManager<SIGABRT> sigabrtHandler(&immortal_sigabrt_handler);
