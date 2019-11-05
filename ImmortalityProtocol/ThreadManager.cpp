#include "ThreadManager.h"
#include "Main.h"

#include "ExceptionContext.h"
#include "Disassembler.h"
#include "Instruction.h"
#include "WatchDog.h"

#include <TlHelp32.h>
#include <sstream>
#include <mutex>
#include <iostream>

std::map<DWORD,Thread> Thread::threadMap;

std::mutex thread_mutex;
CRITICAL_SECTION crit;

void Thread::lock_mutex()
{
  //thread_mutex.lock();
  EnterCriticalSection(&crit);
}
void Thread::unlock_mutex()
{
  //thread_mutex.unlock();
  LeaveCriticalSection(&crit);
}

///////////////////////////////////////////////////////////////////////////// THREAD
Thread::Thread()
  : hThread(nullptr)
  , dwThreadId(0)
  , success(false)
  , lastUserTime()
  , lastKernelTime()
{
  memset(&stackMbi, 0, sizeof(stackMbi));
}

Thread::Thread(DWORD threadID)
  : dwThreadId(threadID ? threadID : GetCurrentThreadId() )
  , success(false)
  , lastUserTime()
  , lastKernelTime()
{
  this->InitThread();
}

Thread::~Thread()
{
  Thread::lock_mutex();
//  if ( this->hThread )
  //  CloseHandle(this->hThread);
  this->hThread = nullptr;
  Thread::unlock_mutex();
}

void ShowLastError(const char *pszStr)
{
  std::stringstream ss;
  const char *pszErr = "Unknown error";
  
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPSTR)&pszErr, 0, nullptr);
  ss << pszStr << " :" << std::endl;
  ss << pszErr << std::endl;
  ErrBox(ss.str().c_str());
}

void Thread::InitThread()
{
  LPCVOID stackAddr = &stackAddr;  // store pointer to current thread's stack
  bool isCurrentThread = this->dwThreadId == GetCurrentThreadId();

  // Retrieve the desired thread's handle (and error check)
  this->hThread = isCurrentThread ? GetCurrentThread() : OpenThread(THREAD_ALL_ACCESS, TRUE, dwThreadId);
  if ( !this->hThread )
    return ShowLastError("Unable to obtain thread's handle");

  // If threadID is not 0, meaning we are not accessing the current thread
  // In this case we must pause the thread in order to retrieve the thread's context containing the stack pointer
  if ( !isCurrentThread )
  {
    // Suspend the thread
    PauseThread paused(this->hThread);
    if ( !paused )
      return ShowLastError("Unable to suspend thread");

    // Get the thread's context information
    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_CONTROL;
    if ( !GetThreadContext(this->hThread, &ctx) )
      return ShowLastError("Unable to get thread context");

    // Save the stack pointer
    stackAddr = (LPCVOID)ctx.Esp;
  }

  // Query the thread's stack
  if ( VirtualQuery(stackAddr, &this->stackMbi, sizeof(this->stackMbi)) == 0 )
    return ShowLastError("Unable to query the stack");

  // Get current thread times
  FILETIME createTime, exitTime, kernelTime, userTime;
  if ( !GetThreadTimes(this->hThread, &createTime, &exitTime, &kernelTime, &userTime) )
    return ShowLastError("GetThreadTimes failed");
   
  // Convert times to ULARGE (QuadWord)
  ULARGE_INTEGER kerTime = make_ularge(kernelTime);
  ULARGE_INTEGER useTime = make_ularge(userTime);

  // Initialize times
  this->lastKernelTime  = kerTime;
  this->lastUserTime    = useTime;

  // Set success
  this->success = true;
}

bool Thread::isValid() const
{
  return this->success;
}

DWORD Thread::stackBegin() const
{
  return (DWORD)this->stackMbi.BaseAddress;
}
DWORD Thread::stackEnd() const
{
  return this->stackBegin() + this->stackMbi.RegionSize;
}

bool Thread::checkCPU(bool windowNotResponding)
{
  // Get current thread times
  FILETIME createTime, exitTime, kernelTime, userTime;
  if ( !GetThreadTimes(this->hThread, &createTime, &exitTime, &kernelTime, &userTime) )
  {
    ShowLastError("GetThreadTimes failed");
    return false;
  }

  ULARGE_INTEGER kerTime = make_ularge(kernelTime);
  ULARGE_INTEGER useTime = make_ularge(userTime);

  // debug
  ULARGE_INTEGER diff;
  diff.QuadPart = (kerTime.QuadPart - this->lastKernelTime.QuadPart)+(useTime.QuadPart - this->lastUserTime.QuadPart);

  if ( windowNotResponding )  // There is a window that isn't responding
  {
    // Thread is taking up most processing time/power
    if ( gTimeDiff.QuadPart <= diff.QuadPart + 80000 )
    {
      /*MessageBox(nullptr, 
                  std::to_string(diff.QuadPart).c_str(), 
                  std::to_string(this->dwThreadId).c_str(), 
                  MB_OK);*/

      // STILL gets the wrong instruction pointer
      //this->breakInfiniteLoop();
    }
  }

  // Set the last encountered time
  GetThreadTimes(this->hThread, &createTime, &exitTime, &kernelTime, &userTime);
  this->lastKernelTime = make_ularge(kernelTime);
  this->lastUserTime = make_ularge(userTime);

  return false;
}

void Thread::breakInfiniteLoop()
{
  // Suspend the thread
  PauseThread paused(this->hThread);
  if ( !paused )
    return ShowLastError("Unable to suspend thread");
  
  Sleep(100);

  // Get the thread's context information
  CONTEXT ctx;
  ctx.ContextFlags = CONTEXT_CONTROL;
  if ( !GetThreadContext(this->hThread, &ctx) )
    return ShowLastError("Unable to get thread context");

  // Iterate instructions from the instruction pointer
  x86::Register eip(ctx.Eip);

  {
    std::ostringstream ss;
    ss << (void*)eip;
    MessageBox(nullptr, ss.str().c_str(), nullptr, MB_OK);
  }

  bool hasBroken = false;
  while ( !hasBroken )
  {
    // Disassemble the instruction
    std::ostringstream ss;
    x86::Instruction insn;
    int size = x86::dasm.disasm(eip, &insn);

    // Switch for instruction type
    switch ( insn.type )
    {
    case x86it::insn_invalid:	  // invalid instruction
      // skip instruction
      ss << "Invalid instruction (in infinite loop)" << std::endl;
      ss << (void*)eip << ": " << insn.toString();
      ErrBox(ss.str().c_str());
      eip += 1;
      break;
    ////////////////////  insn_controlflow
    case x86it::insn_jcc:
      hasBroken = true;

      ss << "Correct!" << std::endl;
      ss << (void*)eip << ": " << insn.toString();
      ErrBox(ss.str().c_str());

      // Move eip past the conditional jump
      eip += size;
      break;
    case x86it::insn_call:
    case x86it::insn_callcc:
    case x86it::insn_jmp:
    case x86it::insn_return:
    ///////////////////  insn_stack
    case x86it::insn_push:
    case x86it::insn_pushflags:
    case x86it::insn_pop:
    case x86it::insn_popflags:
    case x86it::insn_popregs:
    case x86it::insn_pushregs:
    case x86it::insn_enter:
    case x86it::insn_leave:
      hasBroken = true;

      ss << "The infinite loop is too complex!" << std::endl;
      ss << (void*)eip << ": " << insn.toString();
      ErrBox(ss.str().c_str());
      
      break;

    default:
      // Move eip past current instruction
      ss << "Normal Instruction (infinite loop)" << std::endl;
      ss << (void*)eip << ": " << insn.toString();
      ErrBox(ss.str().c_str());

      eip += size;
      break;
    }

  }
  // we've broken past our conditional jump, set the new context
  if ( !SetThreadContext(this->hThread, &ctx) )
    return ShowLastError("Unable to set new thread context");

  // returns while resuming the thread
}

///////////////////////////////////////////////////////////////////////////// PAUSE THREAD
PauseThread::PauseThread(HANDLE threadHandle)
  : hThread( threadHandle )
  , success( threadHandle && SuspendThread(threadHandle) != (DWORD)-1 )
{}

PauseThread::~PauseThread()
{
  if ( this->success )
  {
    if ( ResumeThread(this->hThread) == (DWORD)-1 )
      ShowLastError("There was a problem resuming the thread");
  }
}

PauseThread::operator bool() const
{
  return this->success;
}

///////////////////////////////////////////////////////////////////////////// GLOBALS
void RegisterThread(DWORD dwThreadId)
{
  Thread::lock_mutex();

  Thread newThread = Thread(dwThreadId);
  if ( newThread.isValid() )
    Thread::threadMap[dwThreadId ? dwThreadId : GetCurrentThreadId()] = newThread;

  Thread::unlock_mutex();
}

void UnregisterThread(DWORD dwThreadId)
{
  Thread::lock_mutex();

  if ( !dwThreadId ) dwThreadId = GetCurrentThreadId();
  Thread::threadMap.erase(dwThreadId);

  Thread::unlock_mutex();
}

void RegisterAllThreads()
{
  DWORD dwProcId = GetCurrentProcessId();
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if ( hSnapshot == nullptr )
  {
    ShowLastError("Unable to create thread snapshot.");
    return;
  }

  THREADENTRY32 te = { 0 };
  te.dwSize = sizeof(te);

  int inum = 0;
  for ( BOOL s = Thread32First(hSnapshot, &te); s; s = Thread32Next(hSnapshot, &te) )
  {
    if ( te.th32OwnerProcessID != dwProcId || te.th32ThreadID == GetCurrentThreadId() )
      continue;

    RegisterThread(te.th32ThreadID);
    ++inum;
  }
  CloseHandle(hSnapshot);
}
