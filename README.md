# VEN

VEN is a prototype code editor that experiments with a mix of C, C++ and Python.
This repository contains a minimal cross-language example to demonstrate how
these languages can work together.

## Building

Use `make` to build the `tuxpad` binary:

```sh
make
```

To open the GUI without running the C program you can run:

```sh
make gui
```

Run the program by providing a file name:

```sh
./tuxpad path/to/file.txt
```

The program counts the number of lines in the file using a C++ helper and then
launches a Python Tkinter GUI that demonstrates a dozen editor tools.

### Launching the GUI directly

You can also run the GUI on its own:

```sh
python3 scripts/editor_gui.py
```

The GUI exposes the following twelve tools:

1. New
2. Open
3. Save
4. Close
5. Search
6. Replace
7. Undo
8. Redo
9. Highlight
10. Preferences
11. Plugins
12. Build & Run

### Plugins

Plugins are simple Python modules placed in the `plugins/` directory. Each
plugin must provide a `run(gui)` function. The plugin manager (accessible from
`Tools -> Plugins`) lists available plugins and lets you execute them on the
current document.

#### Highlight plugin

The bundled **highlight** plugin uses the [Pygments](https://pygments.org)
library to provide syntax highlighting for twelve languages:

* Python
* C
* C++
* Java
* JavaScript
* HTML
* CSS
* Ruby
* Go
* Rust
* Bash
* Markdown

If Pygments is not installed you can get it with:

```sh
pip install pygments
```

### Build & Run

The *Build & Run* command runs `make` in the repository root and then launches
the freshly built `tuxpad` binary on the currently loaded file, if any.

## Cleaning

To clean build artifacts run:

```sh
make clean
```
