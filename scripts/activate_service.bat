@echo off

:: Set service startup type to "Automatic"
sc config WinSvcBlocker start= auto > nul || goto Error
echo Service startup type changed

:: Start service after enable
sc start WinSvcBlocker > nul || goto Error
echo Service started

:: No error
echo Service activated successfully
pause
exit /b 0

:Error
echo Failed to activate service
pause
