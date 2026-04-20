# VEN

VEN is a terminal-first, open-source toolkit for developers. It is built in C,
ships as a single CLI binary (`tuxpad`), and supports runtime plugins so teams
can extend local infrastructure checks without rebuilding the core.

## What it does

`./tuxpad` helps developers run local file and system checks from the terminal:

- scan files for lines, words, and bytes
- load Linux-compatible `.so` plugins dynamically
- keep core tooling minimal while allowing extension via plugins

## Build

```sh
make
```

## CLI usage

Quick scan (default mode):

```sh
./tuxpad path/to/file.txt
```

Explicit scan modes:

```sh
./tuxpad scan path/to/file.txt          # scan + run all plugins
./tuxpad scan path/to/file.txt none     # scan only
./tuxpad scan path/to/file.txt wordcount
```

List available plugins:

```sh
./tuxpad plugins
```

## Plugin model

Plugins are shared libraries in `plugins/` and must export:

```c
void run(const char *filename)
```

During `scan`, VEN discovers all `.so` files in `plugins/` and executes either:

- all plugins (`all` mode, default)
- no plugins (`none`)
- one plugin selected by name fragment

Provided reference plugins:

- `wordcount.so`: word metrics for the file
- `sysinfo.so`: Linux system and memory summary
- `diskusage.so`: filesystem space info

## Optional GUI

A lightweight Tkinter UI remains available for experimentation:

```sh
make gui
```

## Clean

```sh
make clean
```
