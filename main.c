/*
 * Copyright (C) 2025 Thamatip Chitpong <tangaming123456@outlook.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 * License-Filename: LICENSE.txt
 */

#include "svcblocker.h"

static
LRESULT
CALLBACK
ListeningWindowProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_QUERYENDSESSION:
            return TRUE;

        case WM_ENDSESSION:
        {
            if (wParam == TRUE)
            {
                LogPrintf("[Main] Windows session is being ended. Exiting\n");
                LogPrintf("[Main] WM_ENDSESSION cleanup is unimplemented\n");
                // TODO: Cleanup
            }

            return 0;
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static
BOOL
ListenWindowsMsg(
    HINSTANCE hInstance)
{
    WNDCLASSEXW wc;
    HWND hWnd;
    MSG msg;

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_SAVEBITS;
    wc.lpfnWndProc = ListeningWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"WinSvcBlocker";

    if (RegisterClassExW(&wc) == 0)
        return FALSE;

    hWnd = CreateWindowExW(0,
                           L"WinSvcBlocker",
                           L"WinSvcBlocker_Main",
                           WS_POPUP,
                           0, 0, 0, 0,
                           NULL,
                           NULL,
                           hInstance,
                           NULL);
    if (!hWnd)
        return FALSE;

    LogPrintf("[Main] Start listening Windows messages\n");

    while (GetMessageW(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    LogPrintf("[Main] End listening Windows messages\n");

    return TRUE;
}

int
WINAPI
wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nShowCmd)
{
    HANDLE hAppMutex;
    BLOCKER WUBlocker; // For testing only
    int iRet = 1;

    hAppMutex = CreateMutexW(NULL, TRUE, L"WinSvcBlocker");
    if (!hAppMutex)
    {
        return 1;
    }
    else if (hAppMutex && GetLastError() == ERROR_ALREADY_EXISTS)
    {
        /* This is a single instance application */
        CloseHandle(hAppMutex);
        return 0;
    }

    InitLog();
    LogPrintf("Windows Service Blocker v0.0.1\n");

    /* Block the Windows Update service */
    WUBlocker = CreateAsyncServiceBlocker(L"wuauserv");
    if (!WUBlocker)
    {
        LogPrintf("[Main] Failed to create blocker thread for service %ls\n", L"wuauserv");
        goto Quit;
    }
    LogPrintf("[Main] Blocker created for service %ls (%p)\n", L"wuauserv", WUBlocker);

    if (!ListenWindowsMsg(hInstance))
    {
        LogPrintf("[Main] Failed to listen Windows messages\n");
    }

    DestroyServiceBlocker(WUBlocker, INFINITE);

    /* No error */
    iRet = 0;

Quit:
    LogPrintf("[Main] Program exited with code %d\n", iRet);
    UninitLog();

    ReleaseMutex(hAppMutex);
    CloseHandle(hAppMutex);

    return iRet;
}
