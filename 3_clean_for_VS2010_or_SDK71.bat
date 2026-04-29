@echo off
SETLOCAL
CD /D %~dp0

rem ▆▆▆▆vars▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
set "sln_file=src\KuShellExtension.sln"

rem only one of this is needed
set "VS2010_PATH=%SystemDrive%\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
set "SDK71_PATH=%SystemDrive%\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd"



rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
rem manual: manually choose to method
rem call :for_VS2010_vcvarsall_met1_msbuild
rem call :for_sdk71_SetEnv

rem pause
rem exit /b



rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
rem auto: automatically choose to method

:: we may have visual studio 2010 installed + sdk 7.1 , or only visual studio 2010 , or only sdk 7.1
:: case1: visual studio 2010 installed + sdk 7.1? use for_VS2010_vcvarsall_met1_msbuild
:: case2: only visual studio 2010? use for_VS2010_vcvarsall_met1_msbuild
:: case3: only sdk 7.1? use for_sdk71_SetEnv
:: --- Registry Paths from SetEnv.cmd ---
SET "WinSDKKey=HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.1"
SET "VSKey=HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\SxS\VS7"
SET "VSKey64=HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\SxS\VS7"

:: --- Reset Flags ---
SET SDK_INSTALLED=0
SET VS_INSTALLED=0

:: 1. Check for SDK 7.1 
REG QUERY "%WinSDKKey%" /v InstallationFolder >nul 2>&1
IF %ERRORLEVEL% EQU 0 SET SDK_INSTALLED=1

:: 2. Check for VS 2010 (x86 or x64 paths) 
REG QUERY "%VSKey%" /v 10.0 >nul 2>&1
IF %ERRORLEVEL% EQU 0 SET VS_INSTALLED=1
REG QUERY "%VSKey64%" /v 10.0 >nul 2>&1
IF %ERRORLEVEL% EQU 0 SET VS_INSTALLED=1

rem call the correct method based on the installed version
IF %VS_INSTALLED% EQU 1 (
  call :for_VS2010_vcvarsall_met1_msbuild
) ELSE IF %SDK_INSTALLED% EQU 1 (
  call :for_sdk71_SetEnv
) ELSE (
  echo.
  echo.
  echo you have to install VS 2010 or SDK 7.1
  echo.
  pause
  exit /b
)

echo.
echo.
echo cleanup finished
echo.
pause
exit /b



rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
:for_VS2010_vcvarsall_met1_msbuild
echo for_VS2010_vcvarsall_met1_msbuild

rem use vcvarsall if you installed visual studio 2010, or VS2010+sdk7.1

call "%VS2010_PATH%" x86

msbuild "%sln_file%" /t:Clean /p:Configuration=Debug /p:Platform="Win32"    
msbuild "%sln_file%" /t:Clean /p:Configuration=Release /p:Platform="Win32"

call "%VS2010_PATH%" x64

msbuild "%sln_file%" /t:Clean /p:Configuration=Debug /p:Platform="x64"    
msbuild "%sln_file%" /t:Clean /p:Configuration=Release /p:Platform="x64"

call :common
exit /b



rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
:for_VS2010_vcvarsall_met2_devenv
echo for_VS2010_vcvarsall_met2_devenv

call "%VS2010_PATH%" x86

devenv src\*.sln /Clean "Debug|Win32"
devenv src\*.sln /Clean "Release|Win32"

call "%VS2010_PATH%" x64

devenv src\*.sln /Clean "Debug|x64"
devenv src\*.sln /Clean "Release|x64"

call :common
exit /b



rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
:for_sdk71_SetEnv
echo for_sdk71_SetEnv
rem use SetEnv if you installed only sdk7.1 and you did not install visual studio 2010

SETLOCAL EnableDelayedExpansion

call "%SDK71_PATH%" /x86

msbuild "%sln_file%" /t:Clean /p:Configuration=Debug /p:Platform="Win32"    
msbuild "%sln_file%" /t:Clean /p:Configuration=Release /p:Platform="Win32"

call "%SDK71_PATH%" /x64

msbuild "%sln_file%" /t:Clean /p:Configuration=Debug /p:Platform="x64"    
msbuild "%sln_file%" /t:Clean /p:Configuration=Release /p:Platform="x64"

call :common
endlocal
exit /b



rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
:common
rem some files are not deleted; lets delete them manually

del src\*.sdf
del src\*.opensdf
del /AH src\*.suo
del src\*.user
rmdir /Q /S build
rmdir /Q /S Release

exit /b


