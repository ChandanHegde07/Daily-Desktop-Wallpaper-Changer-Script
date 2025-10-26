# Daily Cross-Platform Wallpaper Changer

A C-based program that automatically changes your desktop wallpaper once a day. Works on **Windows**, **macOS**, and **Linux** using native or system-level commands.

---

## Features

* Cross-platform support (Windows/macOS/Linux)
* Random wallpaper selection
* Lightweight and dependency-free (optional `feh` on Linux)
* Easily automatable with Task Scheduler, cron, or launchd

---

## Requirements

| Operating System | Dependencies |
|:-----------------|:-------------|
| **Windows**      | Works out of the box via WinAPI |
| **macOS**        | Uses `osascript` (AppleScript) |
| **Linux**        | Requires `gsettings` (GNOME) or [feh](https://wiki.archlinux.org/title/feh) |

---

## Folder Setup

Create a folder containing your wallpapers:

| OS | Path |
|:---|:-----|
| Windows | `C:\Wallpapers\` |
| macOS   | `/Users/Shared/Wallpapers/` |
| Linux   | `/home/<username>/Wallpapers/` |

---


---

## How It Works

* The program scans the wallpaper folder for images
* Chooses one randomly
* Calls OS-specific tools:
  * **Windows:** `SystemParametersInfo` (WinAPI)
  * **macOS:** `osascript` (AppleScript)
  * **Linux:** `gsettings` or `feh`
* Updates the desktop wallpaper instantly

---

## Supported Image Formats

* `.jpg` / `.jpeg`
* `.png`
* `.bmp`

---

