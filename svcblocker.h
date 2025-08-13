/*
 * Copyright (C) 2025 Thamatip Chitpong <tangaming123456@outlook.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 * License-Filename: LICENSE.txt
 */

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include <windows.h>
#include <strsafe.h>

#ifndef _countof
#define _countof(Arr) (sizeof(Arr) / sizeof(Arr[0]))
#endif

typedef PVOID BLOCKER;

/* blocker.c */
BLOCKER
CreateAsyncServiceBlocker(
    LPCWSTR lpServiceName);

BOOL
DestroyServiceBlocker(
    BLOCKER Blocker,
    DWORD Timeout);

BOOL
DestroyServiceBlockers(
    DWORD Count,
    BLOCKER *Blockers,
    DWORD Timeout);

/* blocklist.c */
BOOL
InitBlockList(void);

BOOL
UninitBlockList(
    DWORD Timeout);

/* log.c */
void
InitLog(void);

void
UninitLog(void);

void
LogPrintf(
    const char *Format,
    ...);

void
LogPrintf2(
    const char *Format,
    ...);
