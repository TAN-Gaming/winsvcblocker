@echo off
cd /d "%~dp0"
sc create WinSvcBlocker binpath= "%cd%\svcblocker_svc.exe" start= auto displayname= "WinSvcBlocker"
sc description WinSvcBlocker "Blocks unwanted Windows services."
pause
