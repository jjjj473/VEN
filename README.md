# VEN

VEN is a lightweight code editor inspired by Vim but built with a GTK3 GUI.
It aims to provide Linux users with familiar command-driven editing along
with a simple graphical interface.

Project website: <https://linuxksdteam.site>

## Features

- GTK3 based window with a text editing area
- Insert mode and command mode similar to Vim
- Status bar showing the current mode and messages
- Multiple windows via the `:newwin` command
- Syntax highlighting using GtkSourceView
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
  - `:comment`/`:uncomment` to comment or uncomment lines
  - `:sort` to sort selected lines
  - `:reverse` to reverse line order
  - `:indent`/`:unindent` to adjust indentation
  - `:clear` to erase the buffer
  - `:uuid` to insert a generated UUID
  - `:runfile` to execute the current file with `sh`
  - `:openconf` to open `~/.venrc`
  - `:dupword` to duplicate the current word
  - `:join` to join selected lines
  - `:noblank` to remove blank lines
  - `:wordcount` to show a word count dialog
  - `:dupselect` to duplicate the selected text
  - `:countsel` to count lines, words, and characters in the selection
  - `:timestamp` to insert the current timestamp
  - `:rand` to insert a random number
  - `:basename` to insert the current file's basename
  - `:dirname` to insert the current file's directory
  - `:openrecent` to reopen the last opened file
  - `:lowerall`/`:upperall` to change case of the whole buffer
  - `:swapcase` to flip the case of the selection
  - `:trimleading` to remove leading whitespace
  - `:insertuser` to insert your username
  - `:inserthost` to insert the host name
  - `:linecount` to show the total number of lines
  - `:transpose` to swap the current line with the next
  - `:insertfile [file]` to insert another file's contents
  - `:readonly` to toggle read-only mode
  - `:record NAME` to start recording a macro
  - `:stop` to stop recording
  - `:play NAME` to play a recorded macro
  - `:pipe CMD` to pipe the buffer through a shell command
  - `:newwin` to open a new editor window
  - Error dialogs for failed file or command actions
  - Search dialog available from the Tools menu

## Requirements

- GTK3 development libraries. You can use the helper script
  `scripts/install_deps.sh` to automatically install the correct package for
  many popular Linux distributions including Debian, Ubuntu, Linux Mint,
  Fedora, CentOS, RHEL, Arch Linux, Manjaro, openSUSE and Gentoo.

## Building

Install the dependencies and build the editor using `gcc` and `pkg-config`:

```bash
./scripts/install_deps.sh
```

Then compile the source:

```bash
gcc -Wall $(pkg-config --cflags gtk+-3.0 gtksourceview-4) -o ven src/ven.c $(pkg-config --libs gtk+-3.0 gtksourceview-4)
```

## Running

Launch the editor with:

```bash
./ven
```

Press `Esc` to enter command mode and type one of the commands above.
Use `Esc` again to return to insert mode.

This project is a prototype and welcomes contributions.
