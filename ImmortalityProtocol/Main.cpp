#include "Main.h"

#include <Windows.h>
#include <Commctrl.h>
#include <new.h>

#include "ImmortalExceptionFilter.h"
#include "TopLevelExceptionFilter.h"
#include "ThreadManager.h"
#include "WatchDog.h"

#include <atomic>

// Register the exception filter as soon as possible
TopLevelExceptionFilter TopExceptionFilter(&ImmortalExceptionFilter);

std::atomic<HINSTANCE> hDllInstance;

BOOL IncrementDLLReferenceCount(HINSTANCE hinst)
{
  HMODULE hmod;
  return GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
    reinterpret_cast<LPCTSTR>(hinst),
    &hmod);
}

// Check stuff in DllMain
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  static int oldMode;
  static const INITCOMMONCONTROLSEX commonCtrls = { sizeof(INITCOMMONCONTROLSEX), 0xFFFFFF }; // all controls

  hDllInstance = hinstDLL;

  switch ( fdwReason )
  {
  case DLL_PROCESS_ATTACH:      // Called when this DLL is attached
    InitializeCriticalSection(&crit);
    oldMode = _set_new_mode(1); // enable the "new" handler when malloc is called
    InitCommonControlsEx(&commonCtrls); // initialize common controls (doesn't hurt, makes more stuff available)
    RegisterAllThreads();       // Register all existing threads

    if (NULL != CreateThread(nullptr, 0, &ThreadWatchdog, nullptr, 0, nullptr))
    {
      IncrementDLLReferenceCount(hinstDLL);
    }
    break;
  case DLL_PROCESS_DETACH:      // Called when this DLL is detached
    _set_new_mode(oldMode);
    writeExceptionCount();
    killthread = true;
    break;
  case DLL_THREAD_ATTACH:   // called when new threads are created
    RegisterThread();
    break;
  case DLL_THREAD_DETACH:   // called when threads are destroyed
    UnregisterThread();
    break;
  }
  return TRUE;
}

