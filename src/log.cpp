//
// @author magicxqq <xqq@xqq.im>
//

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include "log.hpp"

#ifdef _WIN32
    #include <Windows.h>
#endif

static const char* LOG_TAG = "BonDriver_EPGStation";
static const wchar_t* LOG_TAG_W = L"BonDriver_EPGStation";

void Log::Info(const char* str) {
    char buffer[512] = {0};
    sprintf(buffer, "[%s] INFO: %s\n", LOG_TAG, str);

#ifdef _WIN32
    OutputDebugStringA(buffer);
#else
    fputs(buffer, stdout);
#endif
}

void Log::Info(const wchar_t* str) {
    wchar_t buffer[512] = {0};
    swprintf_s(buffer, L"[%ls] INFO: %ls\n", LOG_TAG_W, str);

#ifdef _WIN32
    OutputDebugStringW(buffer);
#else
    fputws(buffer, stdout);
#endif
}

void Log::InfoF(const char* format, ...) {
    char formatted[512] = {0};
    va_list vaList;
    va_start(vaList, format);
    vsprintf(formatted, format, vaList);
    va_end(vaList);

    char buffer[512] = {0};
    sprintf(buffer, "[%s] INFO: %s\n", LOG_TAG, formatted);

#ifdef _WIN32
    OutputDebugStringA(buffer);
#else
    fputs(buffer, stdout);
#endif
}

void Log::InfoF(const wchar_t* format, ...) {
    wchar_t formatted[512] = {0};
    va_list vaList;
    va_start(vaList, format);
    vswprintf_s(formatted, format, vaList);
    va_end(vaList);

    wchar_t buffer[512] = {0};
    swprintf_s(buffer, L"[%ls] INFO: %ls\n", LOG_TAG_W, formatted);

#ifdef _WIN32
    OutputDebugStringW(buffer);
#else
    fputws(buffer, stdout);
#endif
}

void Log::Error(const char* str) {
    char buffer[512] = {0};
    sprintf(buffer, "[%s] ERROR: %s\n", LOG_TAG, str);

#ifdef _WIN32
    OutputDebugStringA(buffer);
#else
    fputs(buffer, stderr);
#endif
}

void Log::Error(const wchar_t* str) {
    wchar_t buffer[512] = {0};
    swprintf_s(buffer, L"[%ls] ERROR: %ls\n", LOG_TAG_W, str);

#ifdef _WIN32
    OutputDebugStringW(buffer);
#else
    fputws(buffer, stderr);
#endif
}

void Log::ErrorF(const char* format, ...) {
    char formatted[512] = {0};
    va_list vaList;
    va_start(vaList, format);
    vsprintf(formatted, format, vaList);
    va_end(vaList);

    char buffer[512] = {0};
    sprintf(buffer, "[%s] ERROR: %s\n", LOG_TAG, formatted);

#ifdef _WIN32
    OutputDebugStringA(buffer);
#else
    fputs(buffer, stderr);
#endif
}

void Log::ErrorF(const wchar_t* format, ...) {
    wchar_t formatted[512] = {0};
    va_list vaList;
    va_start(vaList, format);
    vswprintf_s(formatted, format, vaList);
    va_end(vaList);

    wchar_t buffer[512] = {0};
    swprintf_s(buffer, L"[%ls] ERROR: %ls\n", LOG_TAG_W, formatted);

#ifdef _WIN32
    OutputDebugStringW(buffer);
#else
    fputws(buffer, stderr);
#endif
}
