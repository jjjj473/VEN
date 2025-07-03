# VEN Text Editor

A lightweight desktop text editor written in C using GTK and GtkSourceView. It is intended to run on many Linux distributions including Ubuntu, Debian, Fedora, openSUSE, Arch, Manjaro, Linux Mint, Elementary OS, Slackware and Gentoo.

## Features
- Undo and redo
- Find and replace
- Format selected text as **bold** or *italic*
- Convert selection to UPPERCASE or lowercase
- Basic file open/save support
- Right-click context menu for quick access to all actions

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
Right-click inside the editor to open a menu with actions like Open, Save, Undo,
Redo and formatting commands.
