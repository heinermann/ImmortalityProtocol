#pragma once
#include <Windows.h>

extern ULARGE_INTEGER gTimeDiff;

//void ThreadWatchdog();
DWORD WINAPI ThreadWatchdog(LPVOID lpParameter);

