/*
 * Copyright (C) 2025 Thamatip Chitpong <tangaming123456@outlook.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 * License-Filename: LICENSE.txt
 */

#include "../common/svcblocker.h"

#define SVC_PENDING_OP_TIMEOUT 10000

static WCHAR g_SvcName[] = L"WinSvcBlocker";

static SERVICE_STATUS g_SvcStatus;
static SERVICE_STATUS_HANDLE g_SvcStatusHandle;

static HANDLE g_hStopEvent;

/* The default working directory for a Windows service is %WinDir%\System32
 * so this is needed for relative paths to work.
 */
static
BOOL
SetSvcWorkingDir(void)
{
    WCHAR szPath[MAX_PATH];
    WCHAR *ptr;

    if (GetModuleFileNameW(NULL, szPath, _countof(szPath)) == 0)
        return FALSE;

    /* Remove file name from the path */
    ptr = wcsrchr(szPath, L'\\');
    if (ptr)
    {
        /* Keep trailing backslash if this is the drive root folder */
        if (ptr > szPath && *(ptr - 1) == L':')
        {
            ptr++;
        }

        *ptr = L'\0';
    }

    return SetCurrentDirectoryW(szPath);
}

static
BOOL
ReportSvcStatus(
    DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint)
{
    g_SvcStatus.dwCurrentState = dwCurrentState;
    g_SvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    g_SvcStatus.dwWaitHint = dwWaitHint;

    /* Accepted controls */
    if (dwCurrentState == SERVICE_RUNNING)
    {
        g_SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    }
    else
    {
        g_SvcStatus.dwControlsAccepted = 0;
    }

    /* Checkpoint */
    if (dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED)
    {
        g_SvcStatus.dwCheckPoint = 0;
    }
    else
    {
        g_SvcStatus.dwCheckPoint++;
    }

    return SetServiceStatus(g_SvcStatusHandle, &g_SvcStatus);
}

VOID
WINAPI
ServiceCtrlHandler(
    DWORD dwCtrl)
{
    switch (dwCtrl)
    {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
        {
            ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, SVC_PENDING_OP_TIMEOUT);
            SetEvent(g_hStopEvent);
            break;
        }
    }
}

VOID
WINAPI
ServiceMain(
    DWORD dwArgc,
    LPWSTR *lpszArgv)
{
    DWORD dwSvcError = 1;
    HANDLE hMainMutex;

    g_SvcStatusHandle = RegisterServiceCtrlHandlerW(g_SvcName, ServiceCtrlHandler);
    if (g_SvcStatusHandle == 0)
        return;

    /* Report initial status and set init timeout */
    ZeroMemory(&g_SvcStatus, sizeof(g_SvcStatus));
    g_SvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, SVC_PENDING_OP_TIMEOUT);

    if (!SetSvcWorkingDir())
        goto Quit1;

    hMainMutex = CreateMutexW(NULL, TRUE, L"Global\\WinSvcBlocker");
    if (!hMainMutex)
    {
        goto Quit1;
    }
    else if (hMainMutex && GetLastError() == ERROR_ALREADY_EXISTS)
    {
        /* Only one instance is allowed, either app or service */
        CloseHandle(hMainMutex);
        goto Quit1;
    }

    InitLog();
    LogPrintf2("Windows Service Blocker v1.0.0 (Running as service)\n");

    g_hStopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!g_hStopEvent)
    {
        LogPrintf("[Main] Failed to create stop event. Error code: 0x%x\n", GetLastError());
        goto Quit2;
    }

    if (!InitBlockList())
    {
        LogPrintf("[Main] Failed to load blocker list\n");
        goto Quit2;
    }

    /* Init OK, service is now running */
    dwSvcError = 0;
    LogPrintf("[Main] Service init OK\n");
    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    /* Wait for the service to stop */
    WaitForSingleObject(g_hStopEvent, INFINITE);

    UninitBlockList(INFINITE);

Quit2:
    if (g_hStopEvent)
        CloseHandle(g_hStopEvent);

    LogPrintf("[Main] Service stopped with code %lu\n", dwSvcError);
    UninitLog();

    ReleaseMutex(hMainMutex);
    CloseHandle(hMainMutex);

Quit1:
    g_SvcStatus.dwServiceSpecificExitCode = dwSvcError;
    ReportSvcStatus(SERVICE_STOPPED,
                    dwSvcError != 0 ? ERROR_SERVICE_SPECIFIC_ERROR : NO_ERROR,
                    0);
}

/* Service program entry point */
int
wmain(
    int argc,
    wchar_t *argv[])
{
    SERVICE_TABLE_ENTRYW SvcTable[] = {
        { g_SvcName, ServiceMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcherW(SvcTable))
        return 1;

    return 0;
}
