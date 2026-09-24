# LGA FolderSwitch

A small Windows tray app that takes your Open and Save dialogs straight to the folder you are
already looking at in your file manager.

You have a folder open in Explorer or XYplorer. You switch to an app and hit *Open* or *Save as*.
The dialog opens somewhere else, and you browse to the same folder all over again. FolderSwitch
removes that step: when you go back to the dialog, it is already in your folder.

## What it does

- **Automatic switch.** Open a file dialog in any app, go to Explorer or XYplorer, find the
  folder, and come back to the dialog. The dialog jumps to that folder on its own.
- **Manual shortcut: `Ctrl+Alt+O`.** Press it inside a file dialog to jump right away to the
  folder of the file manager you used last. Handy when the dialog was opened after you left the
  file manager, or when automatic switching is off.
- **Recent folders: `Ctrl+Alt+Shift+O`.** Press it inside a file dialog to open a menu, right at
  the mouse pointer, with your last 5 folders, numbered 1 to 5. Click one, press its number, or
  move with the arrow keys and press Enter, and the dialog goes there. Esc or a click outside
  closes the menu. If there is no history yet, the menu says so.

Both shortcuts only act when a file dialog is the active window, and they keep working while
switching is paused.

### Where the recent folders come from

Every time you leave an Explorer or XYplorer window, FolderSwitch remembers the folder it was
showing. Every folder applied to a dialog is added too. The list keeps the 5 most recent
folders, without duplicates, and is kept across app and computer restarts.

## Supported dialogs and file managers

- **Dialogs:** the standard Windows Open/Save dialogs, and Qt file dialogs (Nuke's, for example).
- **File managers:** Windows Explorer and XYplorer. FolderSwitch reads the XYplorer folder from
  its window title, so XYplorer has to show the current path in its title bar.

## The window

FolderSwitch lives in the system tray. Click the tray icon to open its window:

- **Switching is on / paused.** Pause and resume the automatic switch. While paused, dialogs
  keep their own folder and only the shortcuts work. The tray icon shows the state: in color while
  switching is on, a dimmed plain shape while it is paused.
- **Switch automatically.** Turns the automatic switch on or off.
- **Manual shortcut / Recent folders.** The two shortcuts. If another app already uses one of
  them, it is marked *In use* and the other features keep working.
- **Last folder.** The last folder applied to a dialog, where it came from (Explorer, XYplorer
  or Recent), when, and whether the dialog took it.
- **Start with Windows.** Launch FolderSwitch when you sign in.
- **Check for updates at startup.** Look for a new version when the app starts. *Check now*
  looks right away.
- **`?`** opens the help.

Closing the window keeps FolderSwitch running in the tray. Use *Quit* in the tray menu to exit.

## Install

Download the installer (`LGA_FolderSwitch_Setup_v<version>.exe`) from the
[Releases](https://github.com/legandrop/LGA_FolderSwitch/releases) page and run it. The app
turns on *Start with Windows* the first time it runs.

Updates are offered from the app itself. The installer is downloaded, its SHA-256 checksum is
verified, and only then is it run.

## Build from source

Requirements: Windows, Qt 6.5.3 for MinGW 64-bit (`C:\Qt\6.5.3\mingw_64`), the MinGW 13.1
toolchain and Ninja that come with Qt, and CMake 3.16 or newer.

```bat
compilar.bat               :: Debug build in build\, then runs the app
compilar.bat --no-run      :: build only
compilar.bat --release     :: Release build in build-release\
compilar.bat --force-clean :: rebuild from scratch
```

`compilar.bat` copies the Qt runtime next to the executable. Before building, it closes any
running copy of the app. With `--no-run` it only closes the copy it is about to overwrite.
`deploy.bat` and `instalador.bat` produce the distributable folder and the Inno Setup installer.

## Settings and logs

- Settings and the recent folders live in `%APPDATA%\LGA\LGA_FolderSwitch\settings.ini`.
  Earlier versions kept them in the registry (`HKEY_CURRENT_USER\Software\LGA\FolderSwitch`);
  newer versions move them to that file on first start and remove the old registry key.
- Uninstalling removes that folder, the downloaded update installers, the old registry key if it
  is still there, and the *Start with Windows* entry when it points to the uninstalled copy.
- Set `log=true` in `config/debug_flags.txt` to write a `debug.log` for troubleshooting.

## Author

Developed by Lega Pugliese · [github.com/legandrop](https://github.com/legandrop)
