@echo off

:: Stop service before unregister (failure is non-critical)
sc stop WinSvcBlocker > nul && ^
echo Service stopped

:: Unregister service
sc delete WinSvcBlocker > nul || goto Error
echo Service unregistered

:: No error
echo Service uninstalled successfully
pause
exit /b 0

:Error
echo Failed to uninstall service
pause
