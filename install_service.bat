@echo off

:: Register service
sc create WinSvcBlocker binpath= "%~dp0\svcblocker_svc.exe" start= auto displayname= "WinSvcBlocker" > nul || goto Error
sc description WinSvcBlocker "Blocks unwanted Windows services." > nul
echo Service registered

:: Automatically start service after register (failure is non-critical)
sc start WinSvcBlocker > nul && ^
echo Service started

:: No error
echo Service installed successfully
pause
exit /b 0

:Error
echo Failed to install service
pause
