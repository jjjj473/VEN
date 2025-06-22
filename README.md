# VEN

VEN is a lightweight code editor inspired by Vim but built with a GTK3 GUI.
It aims to provide Linux users with familiar command-driven editing along
with a simple graphical interface.

Project website: <https://linuxksdteam.site>

## Features

- GTK3 based window with a text editing area
- Insert mode and command mode similar to Vim
- Status bar showing the current mode and messages
- Basic commands:
  - `:w [file]` to save the current buffer
  - `:q` to quit the editor
  - `:help` to show a help dialog
  - `:o [file]` to open a file
  - `:wq` to save and quit
  - `:new` to create a new buffer
  - `:cut`, `:copy`, `:paste` for clipboard actions
  - `:/pattern` to search text
  - `:!cmd` to run a shell command
  - `:goto N` to jump to a line
  - `:replace A B` to replace text
  - `:date` to insert the current date and time
  - `:visit` to open [Linuxksdteam.site](https://linuxksdteam.site)
  - `:about` to see application info
  - `:dup` to duplicate the current line
  - `:del` to delete the current line
  - `:upper`/`:lower` to change case of the selection
  - `:trim` to trim trailing whitespace
  - `:insertpath` to insert the current file path
  - `:stats` to show line, word, and character counts
  - `:tabs2spaces` and `:spaces2tabs` to convert whitespace
  - `:wrap` to toggle text wrapping
  - `:todo` to insert a TODO label
  - `:addlnum` to prefix lines with numbers

## Requirements

- GTK3 development libraries (e.g. `libgtk-3-dev` on Debian/Ubuntu)

## Building

Compile the editor using `gcc` and `pkg-config`:

```bash
gcc -Wall $(pkg-config --cflags gtk+-3.0) -o ven src/ven.c $(pkg-config --libs gtk+-3.0)
```

## Running

Launch the editor with:

```bash
./ven
```

Press `Esc` to enter command mode and type one of the commands above.
Use `Esc` again to return to insert mode.

This project is a prototype and welcomes contributions.
