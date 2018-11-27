@echo off

REM ** First see if they requested something specifically, 
REM ** then default to the processor architecture.

if "%1" == "AMD64" Goto x64_ENV
if "%1" == "x86" Goto  x86_ENV
if "%PROCESSOR_ARCHITECTURE%" == "AMD64" Goto x64_ENV
if "%PROCESSOR_ARCHITECTURE%" == "x86" Goto  x86_ENV
Goto Exit

:x64_ENV
Set Lib=%DXSDK_DIR%Lib\x64;%Lib%
Set Include=%DXSDK_DIR%Include;%Include%
Set Path=%DXSDK_DIR%Utilities\Bin\x64;%Path%
echo Dx9 x64 enviroment is now enabled.
Goto Exit

:x86_ENV
Set Lib=%DXSDK_DIR%Lib\x86;%Lib%
Set Include=%DXSDK_DIR%Include;%Include%
Set Path=%DXSDK_DIR%Utilities\Bin\x86;%Path%
echo Dx9 x86 enviroment is now enabled.
Goto Exit


:Exit
