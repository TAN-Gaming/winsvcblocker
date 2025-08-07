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

void
LogPrintf(
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
