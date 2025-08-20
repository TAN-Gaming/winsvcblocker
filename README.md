# WinSvcBlocker
A simple program for blocking unwanted Windows services.

[release_link]: https://github.com/TAN-Gaming/winsvcblocker/releases

## Features
- Blocks all the services you don't want in one go.
- Each service blocker works asynchronously.
- The program is available in 2 types, as an application and a service.
- The target services will be blocked regardless of software updates or reinstalls. As long as the service names don't change, they will be blocked.
- Allows you to escape from the situation where Windows or software updates couldn't be disabled and a buggy update trying to reinstall itself every time it is uninstalled.

## How to use
- Download release builds from the [release section][release_link] or build from source.
- Extract the downloaded files to any folder you want.
- Create a `blocklist.txt` file based on the provided template, specify the service names you want to block.
- Put the created `blocklist.txt` file in the same folder as `svcblocker.exe` (for application) or `svcblocker_svc.exe` (for service).
- If you want to run the program as a portable application without installing anything, run `svcblocker.exe`.
- If you want to run the program as a Windows service and automatically launch when your computer starts up, run `install_service.bat` as administrator to install the service.
- To uninstall the service, run `uninstall_service.bat` as administrator.

## Notes
- Administrator privileges are required to use this program.
- Only one instance running is allowed, either app or service.
- `install_service.bat` must be in the same folder as `svcblocker_svc.exe` to work.
- If you're running Windows 11 with fast startup enabled, don't install service on another drive other than the system drive (C:), otherwise automatic start may not work correctly.

## Build instructions
Only Code::Blocks IDE with TDM-GCC-64 is supported for now. CMake support is WIP.
