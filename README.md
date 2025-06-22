# VEN

VEN is a lightweight code editor inspired by Vim but built with a GTK3 GUI.
It aims to provide Linux users with familiar command-driven editing along
with a simple graphical interface.

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
  - `:about` to see application info

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
