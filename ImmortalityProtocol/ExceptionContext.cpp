#include "ExceptionContext.h"
#include "ThreadManager.h"

namespace x86
{
  Register::Register(DWORD &dwValue) : value(dwValue)
  {
  };
  Register &Register::operator =(const Register &other)
  {
    this->value = other.value;
    return *this;
  }

  bool Register::isBadCodePtr() const
  {
    return !!IsBadCodePtr((FARPROC)this->value);
  }
  bool Register::isBadReadPtr(size_t size) const
  {
    return !!IsBadReadPtr((void*)this->value, size);
  }
  bool Register::isBadWritePtr(size_t size) const
  {
    return !!IsBadWritePtr((void*)this->value,size);
  }
  bool Register::isBadStringPtr(size_t size) const
  {
    return !!IsBadStringPtr((LPCSTR)this->value, size);
  }
  bool Register::isOnStack() const
  {
    Thread::lock_mutex();

    // Check each thread's stack range
    for ( auto &it : Thread::threadMap )
    {
      // Check if this register is pointing to the stack
      if ( this->value >= it.second.stackBegin() && this->value < it.second.stackEnd() )
      {
        Thread::unlock_mutex();
        return true;
      }
    }

    Thread::unlock_mutex();
    return false;
  }

  Register &Register::operator +=(const DWORD &val)
  {
    this->value += val;
    return *this;
  }
  Register &Register::operator -=(const DWORD &val)
  {
    this->value -= val;
    return *this;
  }
  Register &Register::operator ++()
  {
    ++this->value;
    return *this;
  }
  Register &Register::operator --()
  {
    --this->value;
    return *this;
  }
  Register::operator unsigned int() const
  {
    return this->value;
  }
  Register::operator void*() const
  {
    return (void*)this->value;
  }

  ExceptionContext::ExceptionContext(EXCEPTION_POINTERS *pEP) 
    : ep(pEP)
  {};

  void ExceptionContext::push(const DWORD &v)
  {
    ep->ContextRecord->Esp -= 4;
    *(DWORD*)(ep->ContextRecord->Esp) = v;
  }
  DWORD ExceptionContext::pop()
  {
    DWORD rval;
    this->pop(rval);
    return rval;
  }

  #define CASE_NAME(x) case x: return #x;
  const char * const ExceptionContext::getName() const
  {
    if ( this->ep )
    {
      switch ( this->ep->ExceptionRecord->ExceptionCode )
      {
        CASE_NAME(STATUS_WAIT_0)
        CASE_NAME(STATUS_ABANDONED_WAIT_0)
        CASE_NAME(STATUS_USER_APC)
        CASE_NAME(STATUS_TIMEOUT)
        CASE_NAME(STATUS_PENDING)
        CASE_NAME(DBG_EXCEPTION_HANDLED)
        CASE_NAME(DBG_CONTINUE)
        CASE_NAME(STATUS_SEGMENT_NOTIFICATION)
        CASE_NAME(STATUS_FATAL_APP_EXIT)
        CASE_NAME(DBG_TERMINATE_THREAD)
        CASE_NAME(DBG_TERMINATE_PROCESS)
        CASE_NAME(DBG_CONTROL_C)
        CASE_NAME(DBG_PRINTEXCEPTION_C)
        CASE_NAME(DBG_RIPEXCEPTION)
        CASE_NAME(DBG_CONTROL_BREAK)
        CASE_NAME(DBG_COMMAND_EXCEPTION)
        CASE_NAME(STATUS_GUARD_PAGE_VIOLATION)
        CASE_NAME(STATUS_DATATYPE_MISALIGNMENT)
        CASE_NAME(STATUS_BREAKPOINT)
        CASE_NAME(STATUS_SINGLE_STEP)
        CASE_NAME(STATUS_LONGJUMP)
        CASE_NAME(STATUS_UNWIND_CONSOLIDATE)
        CASE_NAME(DBG_EXCEPTION_NOT_HANDLED)
        CASE_NAME(STATUS_ACCESS_VIOLATION)
        CASE_NAME(STATUS_IN_PAGE_ERROR)
        CASE_NAME(STATUS_INVALID_HANDLE)
        CASE_NAME(STATUS_INVALID_PARAMETER)
        CASE_NAME(STATUS_NO_MEMORY)
        CASE_NAME(STATUS_ILLEGAL_INSTRUCTION)
        CASE_NAME(STATUS_NONCONTINUABLE_EXCEPTION)
        CASE_NAME(STATUS_INVALID_DISPOSITION)
        CASE_NAME(STATUS_ARRAY_BOUNDS_EXCEEDED)
        CASE_NAME(STATUS_FLOAT_DENORMAL_OPERAND)
        CASE_NAME(STATUS_FLOAT_DIVIDE_BY_ZERO)
        CASE_NAME(STATUS_FLOAT_INEXACT_RESULT)
        CASE_NAME(STATUS_FLOAT_INVALID_OPERATION)
        CASE_NAME(STATUS_FLOAT_OVERFLOW)
        CASE_NAME(STATUS_FLOAT_STACK_CHECK)
        CASE_NAME(STATUS_FLOAT_UNDERFLOW)
        CASE_NAME(STATUS_INTEGER_DIVIDE_BY_ZERO)
        CASE_NAME(STATUS_INTEGER_OVERFLOW)
        CASE_NAME(STATUS_PRIVILEGED_INSTRUCTION)
        CASE_NAME(STATUS_STACK_OVERFLOW)
        CASE_NAME(STATUS_DLL_NOT_FOUND)
        CASE_NAME(STATUS_ORDINAL_NOT_FOUND)
        CASE_NAME(STATUS_ENTRYPOINT_NOT_FOUND)
        CASE_NAME(STATUS_CONTROL_C_EXIT)
        CASE_NAME(STATUS_DLL_INIT_FAILED)
        CASE_NAME(STATUS_FLOAT_MULTIPLE_FAULTS)
        CASE_NAME(STATUS_FLOAT_MULTIPLE_TRAPS)
        CASE_NAME(STATUS_REG_NAT_CONSUMPTION)
        CASE_NAME(STATUS_STACK_BUFFER_OVERRUN)
        CASE_NAME(STATUS_INVALID_CRUNTIME_PARAMETER)
        CASE_NAME(STATUS_ASSERTION_FAILURE)
        CASE_NAME(STATUS_ENCLAVE_VIOLATION)
        CASE_NAME(STATUS_INTERRUPTED)
        CASE_NAME(STATUS_THREAD_NOT_RUNNING)
        CASE_NAME(STATUS_ALREADY_REGISTERED)
        CASE_NAME(STATUS_SXS_EARLY_DEACTIVATION)
        CASE_NAME(STATUS_SXS_INVALID_DEACTIVATION)
      default:
        break;
      }
    }
    return "UNKNOWN";
  }

}
