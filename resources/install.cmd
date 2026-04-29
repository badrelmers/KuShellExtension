@echo off

if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
	regsvr32 /s KuShellExtension64.dll
) else if "%PROCESSOR_ARCHITEW6432%"=="AMD64" (
	regsvr32 /s KuShellExtension64.dll
)

regsvr32 /s KuShellExtension.dll
