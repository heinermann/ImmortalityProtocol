#pragma once
#include <csignal>

template < class CB_T, CB_T (*FN)(CB_T) >
class CallbackManager
{
public:
  // Set new callback
  CallbackManager(CB_T newCallback) : pOldCallback( FN(newCallback) ) {};
  
  // Restore original callback
  ~CallbackManager() { FN(pOldCallback); };
private:
  CB_T pOldCallback;
};

typedef void (*sig_callback)(int);

template <int SIG>
class SignalManager
{
public:
  SignalManager(sig_callback newCallback) : pOldCallback( signal(SIG,newCallback) ) {};
  ~SignalManager() { signal(SIG,pOldCallback); };

private:
  sig_callback pOldCallback;
};