# Daily Cross-Platform Wallpaper Changer

A C-based program that automatically changes your desktop wallpaper once a day. Works on **Windows**, **macOS**, and **Linux** using native or system-level commands.

---

## Features

* Cross-platform support (Windows/macOS/Linux)
* Sequential and random wallpaper selection
* Lightweight and dependency-free (optional `feh` on Linux)
* Persistent state tracking across runs
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
| macOS   | `./Wallpapers/` (relative to executable) |
| Linux   | `/home/<username>/Wallpapers/` |

---

## Supported Image Formats

* `.jpg` / `.jpeg`
* `.png`
* `.bmp`
* `.gif`
* `.webp`
* `.tiff` / `.tif`

---

# Quick Commands

* First time setup
  * `gcc wallpaper.c -o wallpaper_changer`
  * `./wallpaper_changer --list`  # Verify images found
  * `./wallpaper_changer --force` # Set first wallpaper

* Daily usage
  * `./wallpaper_changer`            # Auto-changes after 24h
  * `./wallpaper_changer --status`   # Check current state
  * `./wallpaper_changer --random`   # Pick random wallpaper

---
