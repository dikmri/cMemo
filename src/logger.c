#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <shellapi.h>
#include <strsafe.h>
#include <stdarg.h>

#include "logger.h"

#define LOG_APP_DIRECTORY L"cMemo"
#define LOG_DIRECTORY_NAME L"logs"
#define LOG_FILE_NAME L"cMemo.log"
#define LOG_MAX_SIZE_BYTES (1024 * 1024)
#define LOG_MAX_ROTATIONS 5
#define LOG_WATCHDOG_INTERVAL_MS 5000
#define LOG_WATCHDOG_HANG_MS 30000

static CRITICAL_SECTION g_logLock;
static BOOL g_logLockInitialized = FALSE;
static BOOL g_logInitialized = FALSE;
static WCHAR g_logDirectory[MAX_PATH];
static WCHAR g_logFilePath[MAX_PATH];
static HANDLE g_watchdogStopEvent = NULL;
static HANDLE g_watchdogThread = NULL;
static volatile LONG g_heartbeat = 0;

static BOOL AppendPathComponent(WCHAR *buffer, DWORD cchBuffer,
                                LPCWSTR component)
{
    size_t bufferLength = 0;
    size_t componentLength = 0;

    if (!buffer || !component || cchBuffer == 0) {
        return FALSE;
    }

    bufferLength = lstrlenW(buffer);
    componentLength = lstrlenW(component);
    if (bufferLength > 0 &&
        buffer[bufferLength - 1] != L'\\' &&
        buffer[bufferLength - 1] != L'/') {
        if (bufferLength + 1 >= cchBuffer) {
            return FALSE;
        }
        buffer[bufferLength++] = L'\\';
        buffer[bufferLength] = L'\0';
    }

    if (bufferLength + componentLength >= cchBuffer) {
        return FALSE;
    }

    return SUCCEEDED(StringCchCatW(buffer, cchBuffer, component));
}

static BOOL EnsureDirectory(LPCWSTR path)
{
    if (CreateDirectoryW(path, NULL)) {
        return TRUE;
    }
    return GetLastError() == ERROR_ALREADY_EXISTS;
}

static BOOL BuildLogDirectory(void)
{
    DWORD result = 0;

    result = GetEnvironmentVariableW(L"LOCALAPPDATA",
                                     g_logDirectory,
                                     ARRAYSIZE(g_logDirectory));
    if (result == 0 || result >= ARRAYSIZE(g_logDirectory)) {
        result = GetEnvironmentVariableW(L"APPDATA",
                                         g_logDirectory,
                                         ARRAYSIZE(g_logDirectory));
    }
    if (result == 0 || result >= ARRAYSIZE(g_logDirectory)) {
        result = GetTempPathW(ARRAYSIZE(g_logDirectory), g_logDirectory);
    }
    if (result == 0 || result >= ARRAYSIZE(g_logDirectory)) {
        return FALSE;
    }

    if (!AppendPathComponent(g_logDirectory, ARRAYSIZE(g_logDirectory),
                             LOG_APP_DIRECTORY) ||
        !EnsureDirectory(g_logDirectory) ||
        !AppendPathComponent(g_logDirectory, ARRAYSIZE(g_logDirectory),
                             LOG_DIRECTORY_NAME) ||
        !EnsureDirectory(g_logDirectory)) {
        return FALSE;
    }

    if (FAILED(StringCchCopyW(g_logFilePath, ARRAYSIZE(g_logFilePath),
                              g_logDirectory)) ||
        !AppendPathComponent(g_logFilePath, ARRAYSIZE(g_logFilePath),
                             LOG_FILE_NAME)) {
        return FALSE;
    }

    return TRUE;
}

static BOOL GetFileSizeBytes(LPCWSTR path, ULONGLONG *size)
{
    WIN32_FILE_ATTRIBUTE_DATA data;

    if (!path || !size) {
        return FALSE;
    }

    if (!GetFileAttributesExW(path, GetFileExInfoStandard, &data)) {
        return FALSE;
    }

    *size = ((ULONGLONG)data.nFileSizeHigh << 32) | data.nFileSizeLow;
    return TRUE;
}

static BOOL BuildRotatedLogPath(int index, WCHAR *buffer, DWORD cchBuffer)
{
    return SUCCEEDED(StringCchPrintfW(buffer, cchBuffer,
                                     L"%s.%d",
                                     g_logFilePath,
                                     index));
}

static void RotateLogsIfNeeded(void)
{
    ULONGLONG size = 0;
    WCHAR oldPath[MAX_PATH];
    WCHAR newPath[MAX_PATH];
    int i = 0;

    if (!GetFileSizeBytes(g_logFilePath, &size) ||
        size <= LOG_MAX_SIZE_BYTES) {
        return;
    }

    if (BuildRotatedLogPath(LOG_MAX_ROTATIONS, oldPath, ARRAYSIZE(oldPath))) {
        DeleteFileW(oldPath);
    }

    for (i = LOG_MAX_ROTATIONS - 1; i >= 1; --i) {
        if (BuildRotatedLogPath(i, oldPath, ARRAYSIZE(oldPath)) &&
            BuildRotatedLogPath(i + 1, newPath, ARRAYSIZE(newPath))) {
            MoveFileExW(oldPath, newPath, MOVEFILE_REPLACE_EXISTING);
        }
    }

    if (BuildRotatedLogPath(1, newPath, ARRAYSIZE(newPath))) {
        MoveFileExW(g_logFilePath, newPath, MOVEFILE_REPLACE_EXISTING);
    }
}

static void WriteLogLineLocked(LPCWSTR level, LPCWSTR message)
{
    SYSTEMTIME now;
    WCHAR lineW[2048];
    CHAR lineA[4096];
    HANDLE file = INVALID_HANDLE_VALUE;
    DWORD bytes = 0;
    DWORD written = 0;

    if (!g_logInitialized || !level || !message) {
        return;
    }

    RotateLogsIfNeeded();
    GetLocalTime(&now);
    if (FAILED(StringCchPrintfW(
            lineW,
            ARRAYSIZE(lineW),
            L"%04u-%02u-%02u %02u:%02u:%02u.%03u [%s] pid=%lu tid=%lu %s\r\n",
            now.wYear,
            now.wMonth,
            now.wDay,
            now.wHour,
            now.wMinute,
            now.wSecond,
            now.wMilliseconds,
            level,
            GetCurrentProcessId(),
            GetCurrentThreadId(),
            message))) {
        return;
    }

    bytes = (DWORD)WideCharToMultiByte(CP_UTF8,
                                       0,
                                       lineW,
                                       -1,
                                       lineA,
                                       sizeof(lineA),
                                       NULL,
                                       NULL);
    if (bytes == 0) {
        return;
    }

    file = CreateFileW(g_logFilePath,
                       FILE_APPEND_DATA,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }

    WriteFile(file, lineA, bytes - 1, &written, NULL);
    CloseHandle(file);
}

static void LogWriteV(LPCWSTR level, LPCWSTR format, va_list args)
{
    WCHAR message[1536];

    if (!g_logInitialized || !format) {
        return;
    }

    if (FAILED(StringCchVPrintfW(message, ARRAYSIZE(message), format, args))) {
        StringCchCopyW(message, ARRAYSIZE(message),
                       L"Failed to format log message.");
    }

    EnterCriticalSection(&g_logLock);
    WriteLogLineLocked(level, message);
    LeaveCriticalSection(&g_logLock);
}

static DWORD WINAPI WatchdogThreadProc(LPVOID parameter)
{
    LONG lastHeartbeat = 0;
    DWORD stalledMs = 0;
    BOOL warned = FALSE;

    UNREFERENCED_PARAMETER(parameter);
    lastHeartbeat = InterlockedCompareExchange(&g_heartbeat, 0, 0);

    for (;;) {
        DWORD waitResult = WaitForSingleObject(g_watchdogStopEvent,
                                               LOG_WATCHDOG_INTERVAL_MS);
        LONG currentHeartbeat = 0;

        if (waitResult == WAIT_OBJECT_0) {
            break;
        }

        currentHeartbeat = InterlockedCompareExchange(&g_heartbeat, 0, 0);
        if (currentHeartbeat == lastHeartbeat) {
            stalledMs += LOG_WATCHDOG_INTERVAL_MS;
            if (stalledMs >= LOG_WATCHDOG_HANG_MS && !warned) {
                LogWarn(L"UI heartbeat has not changed for %lu ms.",
                        stalledMs);
                warned = TRUE;
            }
        } else {
            lastHeartbeat = currentHeartbeat;
            stalledMs = 0;
            warned = FALSE;
        }
    }

    return 0;
}

static BOOL BuildCrashDumpPath(WCHAR *buffer, DWORD cchBuffer)
{
    SYSTEMTIME now;

    if (!buffer || cchBuffer == 0) {
        return FALSE;
    }

    GetLocalTime(&now);
    return SUCCEEDED(StringCchPrintfW(
        buffer,
        cchBuffer,
        L"%s\\crash-%04u%02u%02u-%02u%02u%02u-%lu.dmp",
        g_logDirectory,
        now.wYear,
        now.wMonth,
        now.wDay,
        now.wHour,
        now.wMinute,
        now.wSecond,
        GetCurrentProcessId()));
}

static BOOL WriteCrashDump(EXCEPTION_POINTERS *exceptionInfo)
{
    WCHAR dumpPath[MAX_PATH];
    HANDLE file = INVALID_HANDLE_VALUE;
    MINIDUMP_EXCEPTION_INFORMATION dumpException;
    BOOL ok = FALSE;

    if (!BuildCrashDumpPath(dumpPath, ARRAYSIZE(dumpPath))) {
        return FALSE;
    }

    file = CreateFileW(dumpPath,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (file == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    dumpException.ThreadId = GetCurrentThreadId();
    dumpException.ExceptionPointers = exceptionInfo;
    dumpException.ClientPointers = FALSE;
    ok = MiniDumpWriteDump(GetCurrentProcess(),
                           GetCurrentProcessId(),
                           file,
                           MiniDumpNormal,
                           exceptionInfo ? &dumpException : NULL,
                           NULL,
                           NULL);
    CloseHandle(file);
    if (ok) {
        LogError(L"Crash dump written: %s", dumpPath);
    } else {
        DeleteFileW(dumpPath);
    }
    return ok;
}

void LogInitialize(void)
{
    if (g_logInitialized) {
        return;
    }

    InitializeCriticalSection(&g_logLock);
    g_logLockInitialized = TRUE;

    if (!BuildLogDirectory()) {
        return;
    }

    g_logInitialized = TRUE;
    g_watchdogStopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (g_watchdogStopEvent) {
        g_watchdogThread = CreateThread(NULL,
                                        0,
                                        WatchdogThreadProc,
                                        NULL,
                                        0,
                                        NULL);
    }

    LogInfo(L"Logger initialized. directory=%s", g_logDirectory);
}

void LogShutdown(void)
{
    if (!g_logInitialized) {
        if (g_logLockInitialized) {
            DeleteCriticalSection(&g_logLock);
            g_logLockInitialized = FALSE;
        }
        return;
    }

    LogInfo(L"Logger shutting down.");
    if (g_watchdogStopEvent) {
        SetEvent(g_watchdogStopEvent);
    }
    if (g_watchdogThread) {
        WaitForSingleObject(g_watchdogThread, 2000);
        CloseHandle(g_watchdogThread);
        g_watchdogThread = NULL;
    }
    if (g_watchdogStopEvent) {
        CloseHandle(g_watchdogStopEvent);
        g_watchdogStopEvent = NULL;
    }

    g_logInitialized = FALSE;
    DeleteCriticalSection(&g_logLock);
    g_logLockInitialized = FALSE;
}

void LogHeartbeat(void)
{
    InterlockedIncrement(&g_heartbeat);
}

BOOL LogOpenFolder(HWND owner)
{
    HINSTANCE result;

    if (!g_logInitialized) {
        LogInitialize();
    }
    if (g_logDirectory[0] == L'\0') {
        return FALSE;
    }

    result = ShellExecuteW(owner,
                           L"open",
                           g_logDirectory,
                           NULL,
                           NULL,
                           SW_SHOWNORMAL);
    return (INT_PTR)result > 32;
}

LONG WINAPI LogUnhandledExceptionFilter(EXCEPTION_POINTERS *exceptionInfo)
{
    DWORD code = exceptionInfo && exceptionInfo->ExceptionRecord
                     ? exceptionInfo->ExceptionRecord->ExceptionCode
                     : 0;
    PVOID address = exceptionInfo && exceptionInfo->ExceptionRecord
                        ? exceptionInfo->ExceptionRecord->ExceptionAddress
                        : NULL;

    LogError(L"Unhandled exception. code=0x%08lx address=%p",
             code,
             address);
    WriteCrashDump(exceptionInfo);
    return EXCEPTION_EXECUTE_HANDLER;
}

void LogInfo(LPCWSTR format, ...)
{
    va_list args;

    va_start(args, format);
    LogWriteV(L"INFO", format, args);
    va_end(args);
}

void LogWarn(LPCWSTR format, ...)
{
    va_list args;

    va_start(args, format);
    LogWriteV(L"WARN", format, args);
    va_end(args);
}

void LogError(LPCWSTR format, ...)
{
    va_list args;

    va_start(args, format);
    LogWriteV(L"ERROR", format, args);
    va_end(args);
}

void LogLastError(LPCWSTR context)
{
    DWORD error = GetLastError();
    WCHAR message[1024];
    DWORD formatted = 0;

    formatted = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                                   FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL,
                               error,
                               0,
                               message,
                               ARRAYSIZE(message),
                               NULL);
    if (formatted == 0) {
        LogError(L"%s failed. GetLastError=%lu",
                 context ? context : L"Operation",
                 error);
        return;
    }

    LogError(L"%s failed. GetLastError=%lu message=%s",
             context ? context : L"Operation",
             error,
             message);
}
