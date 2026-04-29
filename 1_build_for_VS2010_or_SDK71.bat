@echo off
SETLOCAL
CD /D %~dp0

rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
rem summary:
rem you can build the sln with vs2010 or sdk71, you can install one of them or both
rem if you install vs2010 use vcvarsall.bat to set the env vars
rem if you install sdk71 use SetEnv.cmd to set the env vars
rem if you install vs2010+sdk71 use vcvarsall.bat to set the env vars
rem to build use msbuild to build from command line for vs2010 or sdk71, or use vs2010 gui

rem ▆▆▆▆vars▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
set "sln_file=src\KuShellExtension.sln"

rem only one of this is needed
set "VS2010_PATH=%SystemDrive%\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
set "SDK71_PATH=%SystemDrive%\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd"


rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
rem ___VS2010 vs SDK71______________________________________
rem The Windows SDK provides headers and libraries for creating Windows applications that use native code. These are the same as those that come with Visual Studio 2010, except that the Windows SDK provides newer versions of some tools.
rem From <https://learn.microsoft.com/en-us/previous-versions/visualstudio/windows-sdk/ff660764(v=vs.100)?redirectedfrom=MSDN> 

rem chatgpt: SetEnv.cmd from Windows SDK 7.1 is mainly for building with the SDK environment. It can work for some builds, but you may get path conflicts when Visual Studio is installed too.

rem so if you have vs2010 and SDK71 installed then call "D:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" to set the env vars, if you need to use sdk71 from vs2010:
rem   - on vs2010 gui > select your project>Configuration properties>General>Platform Toolset.
rem     This value defaults to v100 which is the VS 2010 compiler toolset and the Windows SDK 7.0A. With the Windows SDK 7.1, you get a new option Windows7.1SDK which uses the VS 2010 compiler toolset that ships in the Windows SDK along with the updated headers and libraries.
rem   - from cli, with msbuild you can set /p:PlatformToolset="Windows7.1SDK" or maybe WindowsSDK7.1 , or v90/v100

rem if you have installed only SDK 7.1 then call "D:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" to set the env vars, it is specifically designed to register the Windows7.1SDK toolset with MSBuild, but the setenv script have this sentence:
    rem - Platform(x86/x64/ia64) and PlatformToolSet(v90/v100/WindowsSDK7.1) set in project or solution will override the environment
    rem - To use the Visual Studio 2010 tools/headers/libraries, "set PlatformToolset=100"


rem _________________________________________
rem Up to Visual Studio version 2015 (?), the installer set the environment variable %VSxxxCOMNTOOLS% with which it was possible to set the environment variables. The xxx refers to the Visual Studio version (without dots), for example 100, thus the variable being %VS100COMNTOOLS%. The value of the variable points to the Commont7\Tools directory under the respective installation root.

rem VS100COMNTOOLS=D:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools\
rem VS110COMNTOOLS=D:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\Tools\
rem VS140COMNTOOLS=D:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\

rem %VS100COMNTOOLS% is added by vs2010 but i do not use it yet


rem _________________________________________
rem devenv come with VS2010 not SDK7.1

rem in the offline Visual Studio 2010 documentation i found: "For build-related tasks, it is now recommended that you use MSBuild instead of devenv. For more information, see MSBuild Command Line Reference. " so MSBuild is recommended from a long time ago
rem From <https://stackoverflow.com/questions/498106/how-do-i-compile-a-visual-studio-project-from-the-command-line> 
rem anyway it works fine too and its log is exactly similar to the log i get in the visual studio gui


rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
rem manual: manually choose to method

rem call :for_VS2010_vcvarsall_met1_msbuild
rem call :for_VS2010_vcvarsall_met2_devenv
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
echo Build finished
echo.
pause
exit /b



rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
:for_VS2010_vcvarsall_met1_msbuild
echo for_VS2010_vcvarsall_met1_msbuild

rem use vcvarsall if you installed visual studio 2010, or VS2010+sdk7.1

call "%VS2010_PATH%" x86

msbuild "%sln_file%" /t:Rebuild /p:Configuration=Release /p:Platform="Win32" || ( echo BUILD ERROR & pause & exit )
msbuild "%sln_file%" /t:Rebuild /p:Configuration=Debug /p:Platform="Win32" || ( echo BUILD ERROR & pause & exit )

call "%VS2010_PATH%" x64

msbuild "%sln_file%" /t:Rebuild /p:Configuration=Release /p:Platform="x64" || ( echo BUILD ERROR & pause & exit )
msbuild "%sln_file%" /t:Rebuild /p:Configuration=Debug /p:Platform="x64" || ( echo BUILD ERROR & pause & exit )

exit /b



rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
:for_VS2010_vcvarsall_met2_devenv
echo for_VS2010_vcvarsall_met2_devenv

rem devenv come with VS2010 not SDK7.1

rem in the offline Visual Studio 2010 documentation i found: "For build-related tasks, it is now recommended that you use MSBuild instead of devenv. For more information, see MSBuild Command Line Reference. " so MSBuild is recommended from a long time ago
rem From <https://stackoverflow.com/questions/498106/how-do-i-compile-a-visual-studio-project-from-the-command-line> 
rem anyway it works fine too and its log is exactly similar to the log i get in the visual studio gui

rem ______
rem The first argument for devenv is usually a solution file or project file. You can also use any other file as the first argument if you want to have the file open automatically in an editor. When you enter a project file, the IDE looks for an .sln file with the same base name as the project file in the parent directory for the project file. If no such .sln file exists, then the IDE looks for a single .sln file that references the project. If no such single .sln file exists, then the IDE creates an unsaved solution with a default .sln file name that has the same base name as the project file.

rem /Build          Builds the solution or project with the specified solution
                rem configuration. For example "Debug". If multiple platforms
                rem are possible, the configuration name must be enclosed in quotes
                rem and contain platform name. For example: "Debug|Win32".
rem /Clean          Deletes build outputs.
rem /Project        Specifies the project to build, clean, or deploy.
                rem Must be used with /Build, /Rebuild, /Clean, or /Deploy.
rem /ProjectConfig  Overrides the project configuration specified in the solution
                rem configuration. For example "Debug". If multiple platforms are
                rem possible, the configuration name must be enclosed in quotes
                rem and contain platform name. For example: "Debug|Win32".
                rem Must be used with /Project.
rem /Rebuild        Cleans and then builds the solution or project with the
                rem specified configuration.

rem Note that if you are doing command-line builds with DEVENV, vcvars32 settings will have no affect on your builds, unless you also specify the /useenv option. 


rem /project projectnameorfile /projectconfig name

call "%VS2010_PATH%" x86

devenv "%sln_file%" /Rebuild "Release|Win32" || ( echo BUILD ERROR & pause & exit )
devenv "%sln_file%" /Rebuild "Debug|Win32" || ( echo BUILD ERROR & pause & exit )

call "%VS2010_PATH%" x64

devenv "%sln_file%" /Rebuild "Release|x64" || ( echo BUILD ERROR & pause & exit )
devenv "%sln_file%" /Rebuild "Debug|x64" || ( echo BUILD ERROR & pause & exit )

exit /b



rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
:for_sdk71_SetEnv
echo for_sdk71_SetEnv

rem use SetEnv if you installed only sdk7.1 and you did not install visual studio 2010
rem sdk71 have also a script called vcvarsall.bat which can call vcvars32.bat and vcvars64.bat...etc but only vcvars32 exist in the sdk, so we must use SetEnv

:: File : SetEnv.cmd
::
:: Abstract: This batch file sets the appropriate environment variables for the Windows SDK
::           build environment with respect to OS and platform type.
::
:: Usage : Setenv [/Debug | /Release][/x86 | /x64 | /ia64 ][/vista | /xp | /2003 | /2008 | /win7][-h | /?]
::
::                /Debug   - Create a Debug configuration build environment
::                /Release - Create a Release configuration build environment
::                /x86     - Create 32-bit x86 applications
::                /x64     - Create 64-bit x64 applications
::                /ia64    - Create 64-bit ia64 applications
::                /vista   - Windows Vista applications
::                /xp      - Create Windows XP SP2 applications
::                /2003    - Create Windows Server 2003 applications
::                /2008    - Create Windows Server 2008 or Vista SP1 applications
::                /win7    - Create Windows 7 applications
rem SetEnv configures the build environment to create Windows 7 applications by default on win 7 OS if you do not specify the operating system. and xp in xp...etc
:: Note:
:: * Platform(x86/x64/ia64) and PlatformToolSet(v90/v100/WindowsSDK7.1) set in project or solution will override the environment
:: * To upgrade VC6 or later projects to VC2010 format use the VCUpgrade.exe tool.

rem ___________
rem /Debug /Release
rem You don't need SetEnv.cmd /Release or /Debug at all when using MSBuild with /p:Configuration=Debug and /p:Configuration=Release , they are passed directly to MSBuild, which handles everything internally — compiler flags (/Od vs /O2), preprocessor defines (_DEBUG vs NDEBUG), output directories, etc. It reads all of that from the .vcxproj file.
rem The Configuration=Debug/Release that SetEnv.cmd sets is only meaningful for NMAKE-based builds that check that environment variable manually in their makefiles.

rem • Configuration Fallback: It defaults the build configuration to Debug if you don't specify /Release. 


rem ___________
rem /vista /xp /2003 /2008 /win7
rem do i need to use /win7 and /xp ...etc if i m using msbuild?
rem TARGET_PLATFORM only sets two things:
	rem 1. APPVER — e.g. 5.01 for XP, 6.1 for Win7
	rem 2. TITLE — just the console window title (cosmetic)
rem APPVER is consumed by the Windows SDK headers — specifically sdkddkver.h and winsdkver.h — to define macros like WINVER, _WIN32_WINNT, and _WIN32_WINDOWS but only if you haven't already defined them yourself.

rem If you use MSBuild with a .vcxproj, the project file has its own <TargetVersion> / <WinTargetVer> properties that feed WINVER and _WIN32_WINNT directly through the compiler command line (/D_WIN32_WINNT=0x0601 etc.). In that case APPVER from setenv is irrelevant and ignored — MSBuild doesn't read it.
rem If you use NMAKE (or any other make tool that just inherits the environment), then APPVER is the only mechanism setting those macros, so /xp, /win7 etc. matter a lot.

rem What happens if you never set TARGET_PLATFORM?
rem The script auto-detects it from the running OS version (the FOR /F ... Cmd /c Ver block). So on a Win7 machine it becomes WIN7, on XP it becomes XP. This means:
	rem • Your binary's minimum OS requirement will silently match whatever machine you're building on, not what you intend to target
	rem • If you build on Win7 and never set it, _WIN32_WINNT=0x0601 — the binary will not run on XP even if your code has no Win7-specific calls
	rem • You won't get a warning about this

rem Scenario	Do you need /xp, /win7 etc.?
rem -MSBuild + .vcxproj                        No — set <TargetVersion> in the project instead
rem -NMAKE or raw cl.exe without /D flags      Yes — this is the only thing setting WINVER/_WIN32_WINNT
rem -You define _WIN32_WINNT yourself in       No — your explicit definition wins
 rem code or via /D on the compiler

rem For MSBuild the right place to control minimum OS target is the project property Configuration Properties → General → Platform Toolset and _WIN32_WINNT under C/C++ → Preprocessor definitions — not setenv at all.

rem MSBuild vs. NMAKE
	rem • For NMAKE: These switches are critical. NMAKE relies almost entirely on the environment variables (INCLUDE, LIB, PATH, APPVER) that this script exports. If you don't set /xp, NMAKE won't know to pass the correct version flags to the compiler and linker. 

	rem • For MSBuild: The script itself notes that Project or Solution settings will override the environment. If your .vcxproj file explicitly defines the PlatformToolset (e.g., v100) or target versions, it will ignore whatever you typed in the command line. However, if your project is configured to "Inherit from parent/project defaults," it may still look at these environment variables. 


rem ___________
rem the doc says:
rem To set specific targets in the build environment:
  rem · Launch the Windows SDK build environment - From the Start menu, click on  All Programs > Microsoft Windows SDK v7.1 > Windows SDK 7.1 Command Prompt
  rem · Set the build environment -  At the prompt, type:
rem setenv  [/Debug | /Release][/x86 | /x64 | /ia64 ][/vista | /xp | /2003 | /win7][-h | /?]

rem i have to call setenv From the Start menu as the doc said otherwise the environment variables will not be complete because the the Start menu call this:
rem D:\Windows\System32\cmd.exe /E:ON /V:ON /T:0E /K "D:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd"
rem /V:ON means delayed expantion, SetEnv.cmd use ! in some variables so those vars will not be set without delayed expantion
rem so i have to call always setenv from startmenu then run my build from it or if you want to use call then call SETLOCAL EnableDelayedExpansion before you call setenv, or use cmd /V:ON /C "setenv...." 
rem this is not needed with my portable sdk7.1 because i removed the necesity to use delayed expantion in setenv.cmd


rem ___________
rem debug build gave error
rem (ManifestResourceCompile target) ->  TRACKER : error TRK0005: Failed to locate: "rc.exe". The system cannot find the file specified. [F:\_bin\_src\KuShellExt\src\KuShellExtension.vcxproj]
rem rc was not installed in D:\Program Files\Microsoft SDKs\Windows\v7.1\Bin only 64 version is installed "D:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\x64\RC.Exe", this seems another bug when installing vs2010 and SDK7, when i install only sdk7 it is installed correctly. it is not installed maybe because vs2010 already install an older one here "D:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Bin\RC.Exe"
rem this bug does not happen with my portable sdk7.1 because i installed sdk in a clean win7 which did not contain VS2010

SETLOCAL EnableDelayedExpansion
call "%SDK71_PATH%" /x86 /xp

msbuild "%sln_file%" /t:Rebuild /p:Configuration=Release /p:Platform="Win32" || ( echo BUILD ERROR & pause & exit )
msbuild "%sln_file%" /t:Rebuild /p:Configuration=Debug /p:Platform="Win32" || ( echo BUILD ERROR & pause & exit )

call "%SDK71_PATH%" /x64 /xp
msbuild "%sln_file%" /t:Rebuild /p:Configuration=Release /p:Platform="x64" || ( echo BUILD ERROR & pause & exit )
msbuild "%sln_file%" /t:Rebuild /p:Configuration=Debug /p:Platform="x64" || ( echo BUILD ERROR & pause & exit )


endlocal
exit /b



rem ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆
rem not used
:other_method_direct
rem run cl and linker directly

rem vs 2010 show this commands

@rem c++
@rem /Zi /nologo /W3 /WX- /O2 /Oi /Oy- /GL /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "HAVE_ATL" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /Gm- /EHsc /MT /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Yu"StdAfx.h" /Fp"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\KuShellExtension.pch" /Fa"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\" /Fo"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\" /Fd"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\vc100.pdb" /Gd /analyze- /errorReport:queue 

@rem linker
@rem /OUT:"F:\_bin\_src\KuShellExt\src\..\build\Release\bin\KuShellExtension.dll" /INCREMENTAL:NO /NOLOGO /DLL "shlwapi.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /DEF:"KuShellExtension.def" /MANIFEST /ManifestFile:"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\KuShellExtension.dll.intermediate.manifest" /ALLOWISOLATION /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /DEBUG /PDB:"F:\_bin\_src\KuShellExt\build\Release\bin\KuShellExtension.pdb" /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /PGD:"F:\_bin\_src\KuShellExt\build\Release\bin\KuShellExtension.pgd" /LTCG /TLBID:1 /DYNAMICBASE /NXCOMPAT /MACHINE:X86 /ERRORREPORT:QUEUE 

@rem manifest
@rem /nologo /verbose "F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\KuShellExtension.dll.embed.manifest.res" 

rem and others....


rem __________________________________________
rem call "%SystemDrive%\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
rem msbuild "%sln_file%" /t:Rebuild /p:Configuration=Debug /p:Platform="Win32"
rem msbuild "%sln_file%" /t:Rebuild /p:Configuration=Release /p:Platform="Win32"

rem above commands run this commands
rem release32
rem ClCompile:
  rem D:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\CL.exe /c /Zi /nologo /W3 /WX- /O2 /Oi /Oy- /GL /D WIN32 /D NDEBUG /D _WINDOWS /D _USRDLL /D HAVE_ATL /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /MT /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Yc"StdAfx.h" /Fp"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\KuShellExtension.pch" /Fo"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\\" /Fd"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\vc100.pdb" /Gd /TP /analyze- /errorReport:queue stdafx.cpp
  rem stdafx.cpp
  rem D:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\CL.exe /c /Zi /nologo /W3 /WX- /O2 /Oi /Oy- /GL /D WIN32 /D NDEBUG /D _WINDOWS /D _USRDLL /D HAVE_ATL /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /MT /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Yu"StdAfx.h" /Fp"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\KuShellExtension.pch" /Fo"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\\" /Fd"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\vc100.pdb" /Gd /TP /analyze- /errorReport:queue dll.cpp globals.cpp KuContextMenu.cpp KuMenuSet.cpp KuShellExtension.cpp KuShellExtensionFactory.cpp KuShellExtInit.cpp log.cpp NTFSLink.cpp StringConv.cpp
   
rem Link:
  rem D:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\link.exe /ERRORREPORT:QUEUE /OUT:"F:\_bin\_src\KuShellExt\src\..\build\Release\bin\KuShellExtension.dll" /INCREMENTAL:NO /NOLOGO shlwapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /DEF:"KuShellExtension.def" /MANIFEST /ManifestFile:"F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\KuShellExtension.dll.intermediate.manifest" /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /DEBUG /PDB:"F:\_bin\_src\KuShellExt\build\Release\bin\KuShellExtension.pdb" /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /LTCG /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"F:\_bin\_src\KuShellExt\src\..\build\Release\bin\KuShellExtension.lib" /MACHINE:X86 /DLL "F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\dll.obj"
   
rem Manifest:
  rem D:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\bin\mt.exe /nologo /verbose /outputresource:"F:\_bin\_src\KuShellExt\src\..\build\Release\bin\KuShellExtension.dll;#2" /manifest "F:\_bin\_src\KuShellExt\src\..\build\Release\obj\KuShellExtension\KuShellExtension.dll.intermediate.manifest"
   
rem __________________________________________
rem debug32
rem ClCompile:
  rem D:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\CL.exe /c /ZI /nologo /W3 /WX- /Od /Oy- /D WIN32 /D _DEBUG /D _WINDOWS /D _USRDLL /D HAVE_ATL /D _WINDLL /D _UNICODE /D UNICODE /Gm /EHsc /RTC1 /MTd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Yc"StdAfx.h" /Fp"F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension.pch" /Fo"F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\\" /Fd"F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\vc100.pdb" /Gd /TP /analyze- /errorReport:queue stdafx.cpp
  rem stdafx.cpp
  rem D:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\CL.exe /c /ZI /nologo /W3 /WX- /Od /Oy- /D WIN32 /D _DEBUG /D _WINDOWS /D _USRDLL /D HAVE_ATL /D _WINDLL /D _UNICODE /D UNICODE /Gm /EHsc /RTC1 /MTd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Yu"StdAfx.h" /Fp"F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension.pch" /Fo"F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\\" /Fd"F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\vc100.pdb" /Gd /TP /analyze- /errorReport:queue dll.cpp globals.cpp KuContextMenu.cpp KuMenuSet.cpp KuShellExtension.cpp KuShellExtensionFactory.cpp KuShellExtInit.cpp log.cpp NTFSLink.cpp StringConv.cpp
  
rem ManifestResourceCompile:
  rem D:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\bin\rc.exe /nologo /fo"F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension.dll.embed.manifest.res" "F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension_manifest.rc" 

rem Link:
  rem D:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\link.exe /ERRORREPORT:QUEUE /OUT:"F:\_bin\_src\KuShellExt\src\..\build\Debug\bin\KuShellExtension.dll" /INCREMENTAL /NOLOGO shlwapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /DEF:"KuShellExtension.def" /MANIFEST /ManifestFile:"F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension.dll.intermediate.manifest" /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /DEBUG /PDB:"F:\_bin\_src\KuShellExt\build\Debug\bin\KuShellExtension.pdb" /SUBSYSTEM:WINDOWS /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"F:\_bin\_src\KuShellExt\src\..\build\Debug\bin\KuShellExtension.lib" /MACHINE:X86 /DLL "F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension.dll.embed.manifest.res"
  
rem Manifest:
  rem Deleting file "F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension.dll.embed.manifest".
  rem D:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\bin\mt.exe /nologo /verbose /out:"F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension.dll.embed.manifest" /manifest "F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension.dll.intermediate.manifest"
  rem D:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\bin\rc.exe /nologo /fo"F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension.dll.embed.manifest.res" "F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension_manifest.rc" 

rem LinkEmbedManifest:
  rem D:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\link.exe /ERRORREPORT:QUEUE /OUT:"F:\_bin\_src\KuShellExt\src\..\build\Debug\bin\KuShellExtension.dll" /INCREMENTAL /NOLOGO shlwapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /DEF:"KuShellExtension.def" /MANIFEST /ManifestFile:"F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension.dll.intermediate.manifest" /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /DEBUG /PDB:"F:\_bin\_src\KuShellExt\build\Debug\bin\KuShellExtension.pdb" /SUBSYSTEM:WINDOWS /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"F:\_bin\_src\KuShellExt\src\..\build\Debug\bin\KuShellExtension.lib" /MACHINE:X86 /DLL "F:\_bin\_src\KuShellExt\src\..\build\Debug\obj\KuShellExtension\KuShellExtension.dll.embed.manifest.res"



exit /b


