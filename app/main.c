/*
 * Copyright (C) 2025 Thamatip Chitpong <tangaming123456@outlook.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 * License-Filename: LICENSE.txt
 */

#include "../common/svcblocker.h"

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
                BOOL bClean;

                LogPrintf("[Main] Windows session is being ended. Exiting\n");

                /* MS Doc: WM_ENDSESSION respond timeout is 5 seconds */
                bClean = UninitBlockList(4000);
                if (!bClean)
                {
                    LogPrintf("[Main] Failed to stop service blockers\n");
                    /* A blocker thread might be running, it's not safe to call UninitLog here */
                }
                else
                {
                    UninitLog();
                }
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
    HANDLE hMainMutex;
    int iRet = 1;

    hMainMutex = CreateMutexW(NULL, TRUE, L"Global\\WinSvcBlocker");
    if (!hMainMutex)
    {
        return 1;
    }
    else if (hMainMutex && GetLastError() == ERROR_ALREADY_EXISTS)
    {
        /* Only one instance is allowed, either app or service */
        CloseHandle(hMainMutex);
        return 1;
    }

    InitLog();
    LogPrintf2("Windows Service Blocker v1.0.0 (Running as application)\n");

    if (!InitBlockList())
    {
        LogPrintf("[Main] Failed to load blocker list\n");
        goto Quit;
    }

    /* No error */
    iRet = 0;

    if (!ListenWindowsMsg(hInstance))
    {
        LogPrintf("[Main] Failed to listen Windows messages\n");
        iRet = 1;
    }

    UninitBlockList(INFINITE);

Quit:
    LogPrintf("[Main] Program exited with code %d\n", iRet);
    UninitLog();

    ReleaseMutex(hMainMutex);
    CloseHandle(hMainMutex);

    return iRet;
}
