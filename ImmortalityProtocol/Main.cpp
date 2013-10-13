#include "Main.h"

#include <Windows.h>
#include <Commctrl.h>
#include <new.h>

#include "ImmortalExceptionFilter.h"
#include "TopLevelExceptionFilter.h"
#include "ThreadManager.h"
#include "WatchDog.h"

#include <thread>

// Register the exception filter as soon as possible
TopLevelExceptionFilter TopExceptionFilter(&ImmortalExceptionFilter);


std::thread watchdogThread;
HANDLE hWatchdogThread;

// Check stuff in DllMain
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  static int oldMode;
  static const INITCOMMONCONTROLSEX commonCtrls = { sizeof(INITCOMMONCONTROLSEX), 0xFFFFFF }; // all controls

  switch ( fdwReason )
  {
  case DLL_PROCESS_ATTACH:      // Called when this DLL is attached
    InitializeCriticalSection(&crit);
    oldMode = _set_new_mode(1); // enable the "new" handler when malloc is called
    InitCommonControlsEx(&commonCtrls); // initialize common controls (doesn't hurt, makes more stuff available)
    RegisterAllThreads();       // Register all existing threads
    //watchdogThread = std::thread(ThreadWatchdog);
    hWatchdogThread = CreateThread(nullptr, 0, &ThreadWatchdog, nullptr, 0, nullptr);
    break;
  case DLL_PROCESS_DETACH:      // Called when this DLL is detached
    _set_new_mode(oldMode);
    writeExceptionCount();
    TerminateThread(hWatchdogThread, 0);

    //watchdogThread.join();  // Let the object destroy itself (global scope) instead of waiting for it
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

