/*
 * Copyright (C) 2025 Thamatip Chitpong <tangaming123456@outlook.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 * License-Filename: LICENSE.txt
 */

#include "svcblocker.h"

/* Ref: https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-createservicew */
#define SVC_NAME_MAX_LEN 256

static DWORD g_BlockerCount;
static BLOCKER *g_BlockerList;

static
void
TrimString(
    LPWSTR str)
{
    size_t len = wcslen(str);
    WCHAR *pStart = str;
    WCHAR *pEnd = str + (len > 0 ? len - 1 : 0);

    while (iswspace(*pStart))
    {
        pStart++;
        len--;
    }

    while (pEnd > pStart && iswspace(*pEnd))
    {
        pEnd--;
        len--;
    }

    if (len > 0)
    {
        memmove(str, pStart, len * sizeof(WCHAR));
    }

    str[len] = L'\0';
}

BOOL
InitBlockList(void)
{
    FILE *pBlockListFile;
    WCHAR LineBuf[SVC_NAME_MAX_LEN];
    DWORD BlockerCount = 0;
    BLOCKER *BlockerList = NULL;
    BOOL bSuccess = TRUE;

    pBlockListFile = fopen("blocklist.txt", "r");
    if (!pBlockListFile)
        return FALSE;

    while (fgetws(LineBuf, _countof(LineBuf), pBlockListFile))
    {
        TrimString(LineBuf);

        /* Skip empty lines and comments */
        if (LineBuf[0] == L'\0' || LineBuf[0] == L'#')
            continue;

        if (BlockerCount == 0)
        {
            /* This is the first blocker */
            BlockerList = HeapAlloc(GetProcessHeap(), 0, sizeof(BLOCKER));
            if (!BlockerList)
            {
                bSuccess = FALSE;
                break;
            }
        }
        else
        {
            BLOCKER *BlockerList2;

            BlockerList2 = HeapReAlloc(GetProcessHeap(),
                                       0,
                                       BlockerList,
                                       (BlockerCount + 1) * sizeof(BLOCKER));
            if (!BlockerList2)
            {
                bSuccess = FALSE;
                break;
            }

            BlockerList = BlockerList2;
        }

        BlockerList[BlockerCount] = CreateAsyncServiceBlocker(BlockerCount + 1, LineBuf);
        if (!BlockerList[BlockerCount])
        {
            bSuccess = FALSE;
            break;
        }

        LogPrintf("[Main] Blocker created. ID: %lu, Target service: %ls\n",
                  BlockerCount + 1,
                  LineBuf);

        BlockerCount++;
    }

    if (!bSuccess)
    {
        DestroyServiceBlockers(BlockerCount, BlockerList, INFINITE);
        HeapFree(GetProcessHeap(), 0, BlockerList);
    }
    else
    {
        g_BlockerCount = BlockerCount;
        g_BlockerList = BlockerList;
    }

    fclose(pBlockListFile);

    return bSuccess;
}

/* Timeout in ms, INFINITE is allowed, the caller can retry if fail */
BOOL
UninitBlockList(
    DWORD Timeout)
{
    if (!DestroyServiceBlockers(g_BlockerCount, g_BlockerList, Timeout))
        return FALSE;

    HeapFree(GetProcessHeap(), 0, g_BlockerList);

    g_BlockerList = NULL;
    g_BlockerCount = 0;

    return TRUE;
}
