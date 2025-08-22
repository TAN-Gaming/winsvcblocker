@echo off

:: Stop service before disable
sc stop WinSvcBlocker > nul || goto Error
echo Service stopped

:: Set service startup type to "Disabled"
sc config WinSvcBlocker start= disabled > nul || goto Error
echo Service startup type changed

:: No error
echo Service deactivated successfully
pause
exit /b 0

:Error
echo Failed to deactivate service
pause
