#ifndef LOGGER_H
#define LOGGER_H

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>

void LogInitialize(void);
void LogShutdown(void);
void LogHeartbeat(void);
BOOL LogOpenFolder(HWND owner);
LONG WINAPI LogUnhandledExceptionFilter(EXCEPTION_POINTERS *exceptionInfo);

void LogInfo(LPCWSTR format, ...);
void LogWarn(LPCWSTR format, ...);
void LogError(LPCWSTR format, ...);
void LogLastError(LPCWSTR context);

#endif
