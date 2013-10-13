#pragma once
#include <map>
#include <algorithm>
#include <Windows.h>

extern CRITICAL_SECTION crit;

// Pauses the given thread, and unpauses it when destroyed
class PauseThread
{
public:
  PauseThread(HANDLE threadHandle);
  ~PauseThread();

  // True if paused
  operator bool() const;
private:
  HANDLE hThread;
  bool success;
};

// Thread class, used to manage threads
class Thread
{
public:
  Thread();

  // 0 = current thread
  Thread(DWORD threadID);
  ~Thread();

  void InitThread();

  static std::map<DWORD,Thread> threadMap;

  DWORD stackBegin() const;
  DWORD stackEnd() const;

  bool isValid() const;

  // Checks thread CPU times and returns true if it thinks the thread is in an infinite loop
  bool checkCPU(bool windowNotResponding);

  // Attempts to break out of an infinite loop by pausing the thread and modifying its context
  void breakInfiniteLoop();

  // Lock/unlock the threadManager mutex (for watchdog)
  static void lock_mutex();
  static void unlock_mutex();
private:
  HANDLE  hThread;
  DWORD   dwThreadId;

  ULARGE_INTEGER lastKernelTime;
  ULARGE_INTEGER lastUserTime;

  MEMORY_BASIC_INFORMATION stackMbi;
  bool success;
};

void RegisterThread(DWORD dwThreadId = 0);
void UnregisterThread(DWORD dwThreadId = 0);
void RegisterAllThreads();
