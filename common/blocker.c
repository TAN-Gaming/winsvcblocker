/*
 * Copyright (C) 2025 Thamatip Chitpong <tangaming123456@outlook.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 * License-Filename: LICENSE.txt
 */

#include "svcblocker.h"

typedef struct _BLOCKER_DATA {
    DWORD dwBlockerId;
    LPWSTR pszServiceName;
    HANDLE hThread;
    volatile LONG bRunning;
} BLOCKER_DATA, *PBLOCKER_DATA;

static
DWORD
WINAPI
ServiceBlockerThread(
    LPVOID lpParameter)
{
    PBLOCKER_DATA pData = lpParameter;
    DWORD dwError = ERROR_SUCCESS;
    SC_HANDLE hSCManager = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS_PROCESS ServiceInfo;
    DWORD dwSize;
    LONG bRunning;

    LogPrintf("[Blocker: %lu] Thread started\n", pData->dwBlockerId);

    hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager)
    {
        dwError = GetLastError();
        goto Quit;
    }

    hService = OpenServiceW(hSCManager,
                            pData->pszServiceName,
                            SERVICE_QUERY_STATUS | SERVICE_STOP);
    if (!hService)
    {
        dwError = GetLastError();
        goto Quit;
    }

    /* Blocker loop */
    do
    {
        if (!QueryServiceStatusEx(hService,
                                  SC_STATUS_PROCESS_INFO,
                                  (LPBYTE)&ServiceInfo,
                                  sizeof(ServiceInfo),
                                  &dwSize))
        {
            dwError = GetLastError();
            goto Quit;
        }

        if (ServiceInfo.dwCurrentState == SERVICE_RUNNING)
        {
            LogPrintf("[Blocker: %lu] Target service is running. Sending stop request\n", pData->dwBlockerId);

            /* Send stop request */
            if (!ControlService(hService,
                                SERVICE_CONTROL_STOP,
                                (LPSERVICE_STATUS)&ServiceInfo))
            {
                dwError = GetLastError();

                if (dwError != ERROR_SERVICE_NOT_ACTIVE)
                    goto Quit;

                dwError = ERROR_SUCCESS;
            }
        }

        /* Poll every 1 second */
        Sleep(1000);

        /* Check for destroy event */
        bRunning = InterlockedCompareExchange(&pData->bRunning, 0, 0);

    } while (bRunning != 0);

Quit:
    if (hService)
        CloseServiceHandle(hService);

    if (hSCManager)
        CloseServiceHandle(hSCManager);

    LogPrintf("[Blocker: %lu] Thread exited. Error code: 0x%x\n", pData->dwBlockerId, dwError);
    return 0;
}

BLOCKER
CreateAsyncServiceBlocker(
    DWORD dwBlockerId,
    LPCWSTR lpServiceName)
{
    PBLOCKER_DATA pData;
    SIZE_T cbSize;

    pData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*pData));
    if (!pData)
        return NULL;

    pData->dwBlockerId = dwBlockerId;

    cbSize = (wcslen(lpServiceName) + 1) * sizeof(WCHAR);
    pData->pszServiceName = HeapAlloc(GetProcessHeap(), 0, cbSize);
    if (!pData->pszServiceName)
    {
        goto ErrCleanup;
    }
    StringCbCopyW(pData->pszServiceName, cbSize, lpServiceName);

    pData->bRunning = 1;
    pData->hThread = CreateThread(NULL, 0, ServiceBlockerThread, pData, 0, NULL);
    if (!pData->hThread)
        goto ErrCleanup;

    /* Creation success */
    return pData;

ErrCleanup:
    HeapFree(GetProcessHeap(), 0, pData->pszServiceName);
    HeapFree(GetProcessHeap(), 0, pData);

    return NULL;
}

/* Timeout in ms, INFINITE is allowed, the caller can retry if fail */
BOOL
DestroyServiceBlocker(
    BLOCKER Blocker,
    DWORD Timeout)
{
    PBLOCKER_DATA pData = Blocker;
    DWORD dwResult;

    /* Send stop request to the thread */
    InterlockedExchange(&pData->bRunning, 0);

    /* Wait for the thread to terminate */
    dwResult = WaitForSingleObject(pData->hThread, Timeout);
    if (dwResult != WAIT_OBJECT_0)
        return FALSE;

    /* The thread has been stopped, do cleanup */
    CloseHandle(pData->hThread);
    HeapFree(GetProcessHeap(), 0, pData->pszServiceName);
    HeapFree(GetProcessHeap(), 0, pData);

    return TRUE;
}

/* Timeout in ms, INFINITE is allowed, the caller can retry if fail */
BOOL
DestroyServiceBlockers(
    DWORD Count,
    BLOCKER *Blockers,
    DWORD Timeout)
{
    DWORD dwResult;
    HANDLE *WaitList;

    if (Count == 0)
    {
        /* Nothing to do */
        return TRUE;
    }

    WaitList = HeapAlloc(GetProcessHeap(), 0, Count * sizeof(HANDLE));
    if (!WaitList)
        return FALSE;

    for (DWORD i = 0; i < Count; ++i)
    {
        PBLOCKER_DATA pData = Blockers[i];

        /* Add thread to wait list */
        WaitList[i] = pData->hThread;

        /* Send stop request to the thread */
        InterlockedExchange(&pData->bRunning, 0);
    }

    /* Wait for all threads to terminate */
    dwResult = WaitForMultipleObjects(Count, WaitList, TRUE, Timeout);

    /* We don't need the wait list anymore */
    HeapFree(GetProcessHeap(), 0, WaitList);

    if (dwResult != WAIT_OBJECT_0)
        return FALSE;

    /* All threads have been stopped, do cleanup */
    for (DWORD i = 0; i < Count; ++i)
    {
        PBLOCKER_DATA pData = Blockers[i];

        CloseHandle(pData->hThread);
        HeapFree(GetProcessHeap(), 0, pData->pszServiceName);
        HeapFree(GetProcessHeap(), 0, pData);
    }

    return TRUE;
}
