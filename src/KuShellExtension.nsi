!include "MUI.nsh"
!include "x64.nsh"
!include "Library.nsh"

!define PRODUCT_NAME "KuShellExtension"
!define PRODUCT_VERSION "0.01"
!define PRODUCT_FULL_NAME "${PRODUCT_NAME} ${PRODUCT_VERSION}"
!define REG_DLL_FILE "KuShellExtension.dll"
!define REG_DLL_FILE64 "KuShellExtension64.dll"

OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}.exe"
SetCompressor /SOLID LZMA

ShowInstDetails show
ShowUninstDetails show

InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"
InstallDirRegKey HKCU "Software\${PRODUCT_NAME}" ""

Name "${PRODUCT_FULL_NAME}"

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages
	!insertmacro MUI_PAGE_WELCOME
;	!insertmacro MUI_PAGE_LICENSE "GPL.txt"
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES

	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES

	!insertmacro MUI_LANGUAGE "English"
	!insertmacro MUI_LANGUAGE "Albanian"
	!insertmacro MUI_LANGUAGE "Arabic"
	!insertmacro MUI_LANGUAGE "Basque"
	!insertmacro MUI_LANGUAGE "Belarusian"
	!insertmacro MUI_LANGUAGE "Bosnian"
	!insertmacro MUI_LANGUAGE "Breton"
	!insertmacro MUI_LANGUAGE "Bulgarian"
	!insertmacro MUI_LANGUAGE "Catalan"
	!insertmacro MUI_LANGUAGE "Croatian"
	!insertmacro MUI_LANGUAGE "Czech"
	!insertmacro MUI_LANGUAGE "Danish"
	!insertmacro MUI_LANGUAGE "Dutch"
	!insertmacro MUI_LANGUAGE "Estonian"
	!insertmacro MUI_LANGUAGE "Farsi"
	!insertmacro MUI_LANGUAGE "Finnish"
	!insertmacro MUI_LANGUAGE "French"
	!insertmacro MUI_LANGUAGE "German"
	!insertmacro MUI_LANGUAGE "Greek"
	!insertmacro MUI_LANGUAGE "Hebrew"
	!insertmacro MUI_LANGUAGE "Hungarian"
	!insertmacro MUI_LANGUAGE "Icelandic"
	!insertmacro MUI_LANGUAGE "Indonesian"
	!insertmacro MUI_LANGUAGE "Irish"
	!insertmacro MUI_LANGUAGE "Italian"
	!insertmacro MUI_LANGUAGE "Japanese"
	!insertmacro MUI_LANGUAGE "Korean"
	!insertmacro MUI_LANGUAGE "Kurdish"
	!insertmacro MUI_LANGUAGE "Latvian"
	!insertmacro MUI_LANGUAGE "Lithuanian"
	!insertmacro MUI_LANGUAGE "Luxembourgish"
	!insertmacro MUI_LANGUAGE "Macedonian"
	!insertmacro MUI_LANGUAGE "Malay"
	!insertmacro MUI_LANGUAGE "Mongolian"
	!insertmacro MUI_LANGUAGE "Norwegian"
	!insertmacro MUI_LANGUAGE "NorwegianNynorsk"
	!insertmacro MUI_LANGUAGE "Polish"
	!insertmacro MUI_LANGUAGE "Portuguese"
	!insertmacro MUI_LANGUAGE "PortugueseBR"
	!insertmacro MUI_LANGUAGE "Romanian"
	!insertmacro MUI_LANGUAGE "Russian"
	!insertmacro MUI_LANGUAGE "Serbian"
	!insertmacro MUI_LANGUAGE "SerbianLatin"
	!insertmacro MUI_LANGUAGE "SimpChinese"
	!insertmacro MUI_LANGUAGE "Slovak"
	!insertmacro MUI_LANGUAGE "Slovenian"
	!insertmacro MUI_LANGUAGE "Spanish"
	!insertmacro MUI_LANGUAGE "Swedish"
	!insertmacro MUI_LANGUAGE "Thai"
	!insertmacro MUI_LANGUAGE "TradChinese"
	!insertmacro MUI_LANGUAGE "Turkish"
	!insertmacro MUI_LANGUAGE "Ukrainian"
	!insertmacro MUI_LANGUAGE "Welsh"

Section "Install" Install
	SetOutPath "$INSTDIR"

	!define LIBRARY_SHELL_EXTENSION
	!insertmacro InstallLib REGDLL NOTSHARED REBOOT_NOTPROTECTED "Release\${REG_DLL_FILE}" "$INSTDIR\${REG_DLL_FILE}" "$INSTDIR"
	${If} ${RunningX64}
		!define LIBRARY_X64
		!insertmacro InstallLib REGDLL NOTSHARED REBOOT_NOTPROTECTED "x64\Release\${REG_DLL_FILE}" "$INSTDIR\${REG_DLL_FILE64}" "$INSTDIR"
		!undef LIBRARY_X64
	${EndIf}
	!undef LIBRARY_SHELL_EXTENSION

	File "config.xml"

	;Store installation folder
	WriteRegStr HKCU "Software\${PRODUCT_NAME}" "" $INSTDIR
	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"
	; Show up in Add/Remove programs
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayName" "${PRODUCT_FULL_NAME}"
	WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstall"
	!define LIBRARY_SHELL_EXTENSION
	!insertmacro UnInstallLib REGDLL NOTSHARED REBOOT_NOTPROTECTED "$INSTDIR\${REG_DLL_FILE}"
	${If} ${RunningX64}
		!define LIBRARY_X64
		!insertmacro UnInstallLib REGDLL NOTSHARED REBOOT_NOTPROTECTED "$INSTDIR\${REG_DLL_FILE64}"
		!undef LIBRARY_X64
	${EndIf}
	!undef LIBRARY_SHELL_EXTENSION
	
	Delete "$INSTDIR\Uninstall.exe"
	;Delete "$INSTDIR\config.xml"
	;RMDir /r /REBOOTOK "$INSTDIR"

	DeleteRegKey HKCU "Software\${PRODUCT_NAME}"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
SectionEnd
