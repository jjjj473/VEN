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

## Cleaning

To clean build artifacts run:

```sh
make clean
```
