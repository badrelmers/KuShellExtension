# Configuration


## Variables

KuShellExtension allows users define some variables to simplify the configuration, which will be substituted in attributes and text nodes of XML elements \*after\* it is defined in the configuration file.

The names of user defined and built-in variables are case-sensitive, but environment variables are case-insensitive. All variables are static-binding, which means it is only expanded when KuShellExtension parses this file.

KuShellExtension re-parses configuration whenever the configuration file is changed.

### Syntax
```
<var name="NAME">VALUE</var>
```

Define a variable named NAME and the value of it is VALUE.
```
${var:NAME}
```
Expand to VALUE which is defined as above or some built-in ones.
```
${env:NAME}
```
Expand to environment variables named NAME. `(%NAME%)`
```
${NAME}
```
Expand to `${var:NAME}` first, or expand to `${env:NAME}` if `${var:NAME}` is not exist.

### Built-in variables

```
• CONFIG: The path of configuration file.
• KU_SHELL_EXTENSION_DIR: The directory which contains KuShellExtension.
• KU_SHELL_EXTENSION_DRIVE: The drive (e.g. C:) which contains KuShellExtension.

• Arch: Expand to "32" on 32bits Windows, "64" on 64bits Windows (even in WOW64 mode).
• ArchName: "x86" or "x64", depend on OS (not CPU).
• SysDir:
	○ "C:\Windows\System32" (Windows 32bits, represent as Win32 following)
	○ "C:\Windows\SysWOW64" (Windows 64bits, KuShellExtension 32bits, represent as WOW64 following)
	○ "C:\Windows\System32" (Windows 64bits, KuShellExtension 64bits, represent as Win64 following)
• SysDir32:
	○ "C:\Windows\System32" (Win32)
	○ "C:\Windows\SysWOW64" (WOW64 and Win64)
• SysDir64:
	○ "C:\Windows\System32" (all platforms)
• ProgramFiles:
	○ "C:\Program Files" (Win32)
	○ "C:\Program Files (x86)" (WOW64)
	○ "C:\Program Files" (Win64)
• ProgramFiles32:
	○ "C:\Program Files" (Win32)
	○ "C:\Program Files (x86)" (WOW64 and Win64)
• ProgramFiles64:
	○ "C:\Program Files" (all platforms)
• Documents: "My Documents" folder
• CommonDocuments: "Common Documents" folder
```

Other available varables:

Pictures, CommonPictures, Music, CommonMusic, Video, CommonVideo, Desktop, CommonDesktop, Startup, CommonStartup, Downloads, CommonDownloads

### Varables for configration

These varables will affect KuShellExtension in some way, you shouldn't use them other than configure KuShellExtension.

-   LEGACY\_STYLE (boolean):

Enable this if you are using a third-party theme in Windows Vista and the icons aren't drawn properly.

-   HIDE\_MISSING (boolean):

Enable this to hide the entries which point to some programs aren't existed.

## Menus

### Synopsis
```xml
   <menu name="NAME"
       class="CLASSES"
       icon="ICON">
       <menuitem name="NAME"
           class="CLASSES"
           icon="ICON"
           action="execute|builtin"
           console="true|false"
           multiple="N"
           workdir="PATH"
           >
           ACTION
       </menuitem>
   </menu>

   <for V="V1 V2 V3..." U="U1 U2 U3" ...>...</for>
```

### Description

-   \<menu>...</menu>

Define a submenu.

-   \<menuitem>...</menutiem>

Define a leaf entry of menu. To create a separator, use four hyphens ('-') as its name.

-   \<for V="V1 V2 V3..." U="U1 U2 U3 ..." ...>...</for>

For ... in loop, `${var:V}` and `${var:U}` are available in the `<for></for>` scope.

### Attributes

#### name

The string shown on the menu.

#### icon

The path of icon shown on the the menu, it can be a DLL, excuatable file (EXE), or icon file (ICO). If there are multiple icons in a file, an optional index can be append to the path with a comma. If the icon is not specified in a menu entry, the icon of the application would be used.

For example, specify `icon="C:\Windows\System32\shell32.dll,2"` will use the third icon in "shell32.dll", the index is counted from zero.

#### class

Specify the menu entry or the submenu should show on what situations. The following formats are allowed.

-   .???

file extensions

-   \* (an asterisk)

files

-   folder

folders (directories, including drives)

-   drive

drives

-   drive:removable

removable drives

-   drive:fixed

fixed drives

-   drive:network

network drives

-   drive:optical

optical drives

-   drive:ramdisk

network drives

-   at:PATH

the file at PATH, can use wildcard.

-   in:DIR

the file in DIR

If the class of a submenu(`<menu>`) is not specified, it will be shown if there is a child entry should be shown. On the other hand, if the class of a menu(`<menuitem>`) is not specified, it will use it parent setting if it exists, or "`*`" will be used.

#### action

The type of the action when user click on the menu entry, current only two type are available. The first is "execute", which is the default, it means execute the command line defined in the (text) node. The second is "builtin", which means the action is pre-defined by KuShellExtension, see [#Built-in actions] section for details.

#### console

Set this to true if the application is a console program and you would like to hide the command prompt window. Don't use it with normal GUI programs.

#### multiple

Specify the number of arguments the action supports. The menu entry will only be shown when the number of files/folders is selected. Set it to 0 means it is unlimited. If this value is not specified, KuShellExtension tries to detect it by the command line.

#### workdir

The working directory to execute the command. "`.`" (a single dot) causes KuShellExtension to use the directory of the executable file. "`.\`" causes KuShellExtension always use the selected directory.

### Actions

Normal actions is described as a command line to launch a program, for the usability, some (dynamic) variables are allowed (see [#% expansion] below). Note you may need to use quotes for the strings that contains spaces, but do not use `"` directly, because it is not allowed in XML, use `&quot;` instead.

### % expansion

#### %1 %2 %3 ... %9

Expand to the first, second, third, ... file the user selected.

#### %w

Expand to the working directory.

#### %z

Expand to archive name, useful with WinZIP/WinRAR/7-Zip etc. See the example below.

#### %\*

Expand to all files the user selected. DO NOT USE QUOTES WITH THIS.

#### %@

The same with the above, except the command will be executed separately. For example, the user selected 10 files and click the menu entry, then the command would be executed 10 times. DO NOT USE QUOTES WITH THIS.

#### %u\[N\]

Generates a UTF-8 encoded file name list and expand to the path of the list file. N is a number that tells KuShellExtension should skip first N files. This is used by WinRAR, which need to skip first file.

#### %L\[N\]

Generates a UTF-16 encoded file name list and expand to the path of the list file.

#### %l\[N\]

Generates a system codepage encoded file name list and expand to the path of the list file.

#### %c\[N\]

Expand to the file path in the clipboard, you could copy the files from explorer or copy them as text. N is the index of files in clipboard, which is counted from zero. (`%c0` -> first file, `%c1` -> second file, ...)

#### %%

Expand to '%' character.

### Modifiers of % expansion

Some modifiers can be use in [#% expansion], the format is similar to Windows Command Prompt. The following are available modifiers characters.

#### d

Drive letter.

#### p

Directory without drive letter.

#### n

File name without extension.

#### x

File extension.

#### s

System code page compatible path, substitutes incompatible parts with 8.3 short file name.

For example, given `%1 = "C:\path\to\the\file.txt"`

```batch
%~d1 = "C:"
%~p1 = "\path\to\the\"
%~dp1 = "C:\path\to\the\"
%~nx1 = "file.txt"
%~dpn1 = "C:\path\to\the\file"
```

### Built-in actions

#### DropSymLinks \[0|1\]

Create symbolic links (>= Vista), the target of links are taken from clipboard. This command takes one argument with the value 0 or 1. Use 0 to create the links with relative path, 1 to create the links with absolute path.

#### DropJunctions

Create junctions.

#### DropHardLinks

Create hard links.

#### Reload

Reload the config file.
