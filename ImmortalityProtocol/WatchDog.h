#pragma once
#include <Windows.h>
#include <atomic>

extern ULARGE_INTEGER gTimeDiff;

extern std::atomic_bool killthread;

//void ThreadWatchdog();
DWORD WINAPI ThreadWatchdog(LPVOID lpParameter);

