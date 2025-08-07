/*
 * Copyright (c) 2025 Company Example <company@example.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 * License-Filename: LICENSE.txt
 */

#include <stdarg.h>
#include <stdio.h>

#include <windows.h>
#include <strsafe.h>

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

/* log.c */
void
InitLog(void);

void
UninitLog(void);

void
LogPrintf(
    const char *Format,
    ...);
