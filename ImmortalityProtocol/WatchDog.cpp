#include "WatchDog.h"
#include <thread>
#include <sstream>

#include "ThreadManager.h"
#include "Main.h"

#define WATCHDOG_ERR (0x7DADAB0B)

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
  // Retrieve the process and thread id of the current window
  // NOTE: For some reason it catches the wrong thread for the window owner
  DWORD dwProcId, dwThreadId;
  dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcId);

  // If the window's owning process ID is the current process
  if ( dwProcId != GetCurrentProcessId() )
    return TRUE;
  
  // Check if the window is not responding
  if ( SendMessageTimeout(hwnd, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 5000, nullptr) != 0 )
    return TRUE;

  // The message failed
  
  /*
  // if ( GetLastError() == ERROR_TIMEOUT )
  Thread::lock_mutex();
  
  // WRONG THREAD!!!
  // If the thread exists in our thread pool
  auto i = Thread::threadMap.find(dwThreadId);
  if ( i != Thread::threadMap.end() )
    i->second.breakInfiniteLoop();  // skip all instructions until after the next conditional jump

  Thread::unlock_mutex();*/
  SetLastError(WATCHDOG_ERR);
  return FALSE;
}

ULARGE_INTEGER gTimeDiff;

//void ThreadWatchdog()
DWORD WINAPI ThreadWatchdog(LPVOID lpParameter)
{
  for (;;)  // always do this
  {
    FILETIME currentFileTime;
    // run once
    static ULARGE_INTEGER lastFileTime = (GetSystemTimeAsFileTime(&currentFileTime), make_ularge(currentFileTime) );

    //std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
    Sleep(5000);
    
    // Get the current system time
    GetSystemTimeAsFileTime(&currentFileTime);
    ULARGE_INTEGER curTime = make_ularge(currentFileTime);

    // Debug
    gTimeDiff.QuadPart = curTime.QuadPart - lastFileTime.QuadPart;
    //MessageBox(nullptr, std::to_string(gTimeDiff.QuadPart).c_str(), "Process", MB_OK);
    
    // If a window is not responding
    bool notResponding = EnumWindows(&EnumWindowsProc, 0) == FALSE && GetLastError() == WATCHDOG_ERR;
    
    // Iterate threads and check their time metrics
    Thread::lock_mutex();
    for ( auto &it : Thread::threadMap )
    {
      if ( it.second.checkCPU(notResponding) )
      {
        //  1. Pause the thread
        //  2. Retrieve its context
        //  3. Skip over next "jump if.."
        //  4. Set the new context
        //  5. Resume the thread
      }

    }
    Thread::unlock_mutex();
    
    // Save last system time
    GetSystemTimeAsFileTime(&currentFileTime);
    lastFileTime = make_ularge(currentFileTime);

  }
  return 0;
}
