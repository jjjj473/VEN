# VEN Text Editor

A lightweight desktop text editor written in C using GTK and GtkSourceView. It is intended to run on many Linux distributions including Ubuntu, Debian, Fedora, openSUSE, Arch, Manjaro, Linux Mint, Elementary OS, Slackware and Gentoo.

## Features
- Undo and redo
- Find and replace
- Format selected text as **bold** or *italic*
- Convert selection to UPPERCASE or lowercase
- Basic file open/save support
- Right-click context menu for quick access to all actions
- Menu bar with keyboard shortcuts
- Adjustable font size via Preferences
- Status bar showing cursor position
- Optional Windows 2000 style theme for a retro look

## Requirements
- GTK+3 development files
- GtkSourceView 3.0 development files
- GCC and pkg-config

Debian-based systems can install these via:
```bash
sudo apt-get install build-essential libgtk-3-dev libgtksourceview-3.0-dev
```

## Building
Run:
```bash
make
```
This creates the `editor` binary.

## Running
```bash
./editor
```
Use the menu bar or right-click context menu for actions like Open, Save, Undo
and formatting. The status bar shows the current line and column. Open
Preferences from the View menu to change the editor font size.

The program requires an X11/Wayland display. If the `DISPLAY` environment
variable is not set, the editor will exit with an error message instead of
crashing.
