@echo off
SETLOCAL
CD /D %~dp0

rem prepare the release, put the needed files in a folder so i can zip it and upload it to github

set "release_dir=%~dp0Release"
if exist "%release_dir%" rmdir /Q /S "%release_dir%"
mkdir "%release_dir%" 

copy "%~dp0build\Release\bin\KuShellExtension.dll" "%release_dir%\KuShellExtension.dll"
copy "%~dp0build\x64\Release\bin\KuShellExtension.dll" "%release_dir%\KuShellExtension64.dll"


copy "%~dp0resources\uninstall.cmd" "%release_dir%"
copy "%~dp0resources\config.cht.xml" "%release_dir%"
copy "%~dp0resources\config.xml" "%release_dir%"
copy "%~dp0resources\install.cmd" "%release_dir%"

copy "%~dp0LICENSE.txt" "%release_dir%"
copy "%~dp0README.md" "%release_dir%"

if not "%DoNotPause%"=="yes" pause
