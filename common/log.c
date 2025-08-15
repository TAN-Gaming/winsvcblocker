/*
 * Copyright (C) 2025 Thamatip Chitpong <tangaming123456@outlook.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 * License-Filename: LICENSE.txt
 */

#include "svcblocker.h"

static CRITICAL_SECTION g_csLogFile;
static FILE *g_pLogFile;

void
InitLog(void)
{
    g_pLogFile = fopen("svcblocker.log", "w");
    InitializeCriticalSection(&g_csLogFile);
}

void
UninitLog(void)
{
    DeleteCriticalSection(&g_csLogFile);
    if (g_pLogFile)
        fclose(g_pLogFile);
}

/* Return empty string on failure */
static
void
GetLogTimestamp(
    char *Buffer,
    size_t cchSize)
{
    time_t RawTime = time(NULL);
    struct tm *LocalTime = localtime(&RawTime);

    if (!LocalTime)
    {
        Buffer[0] = '\0';
        return;
    }

    /* YYYY-MM-DD HH:MM:SS */
    snprintf(Buffer, cchSize, "%04d-%02d-%02d %02d:%02d:%02d",
             LocalTime->tm_year + 1900,
             LocalTime->tm_mon + 1,
             LocalTime->tm_mday,
             LocalTime->tm_hour,
             LocalTime->tm_min,
             LocalTime->tm_sec);
}

void
LogPrintf(
    const char *Format,
    ...)
{
    char Timestamp[32];
    va_list Args;

    GetLogTimestamp(Timestamp, _countof(Timestamp));

    va_start(Args, Format);

    EnterCriticalSection(&g_csLogFile);
    if (g_pLogFile)
    {
        fprintf(g_pLogFile, "%s ", Timestamp);
        vfprintf(g_pLogFile, Format, Args);
        fflush(g_pLogFile);
    }
    LeaveCriticalSection(&g_csLogFile);

    va_end(Args);
}

/* Like LogPrintf, but without timestamp */
void
LogPrintf2(
    const char *Format,
    ...)
{
    va_list Args;

    va_start(Args, Format);

    EnterCriticalSection(&g_csLogFile);
    if (g_pLogFile)
    {
        vfprintf(g_pLogFile, Format, Args);
        fflush(g_pLogFile);
    }
    LeaveCriticalSection(&g_csLogFile);

    va_end(Args);
}
