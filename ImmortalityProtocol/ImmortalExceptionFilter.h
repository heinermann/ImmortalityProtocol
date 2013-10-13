#pragma once
#include <Windows.h>
#include <string>

void writeExceptionCount();
std::string getLogName();
LONG WINAPI ImmortalExceptionFilter(EXCEPTION_POINTERS *ep);
