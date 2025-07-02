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

To install the optional Python dependencies in a virtual environment run:

```sh
make venv
```

Launch the editor with the environment using:

```sh
make run FILE=path/to/file.txt
```

If `FILE` is omitted it defaults to `README.md`.

When running `tuxpad` directly, the program looks for `./.venv/bin/python3`
and uses it if found. This means the GUI will automatically use the packages
installed by `make venv` without needing to adjust your `PATH`.

The program counts the number of lines in the file using a C++ helper and then
launches a Python Tkinter GUI that demonstrates a dozen editor tools. If no
graphical display is available (i.e. the `DISPLAY` variable is unset) the GUI
is skipped and a message is printed instead.

### Launching the GUI directly

You can also run the GUI on its own:

```sh
python3 scripts/editor_gui.py
```

The GUI exposes the following fifteen tools:

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
13. Spell Check
14. Format Python
15. Word Frequency

The **Search** and **Replace** commands open dialogs to locate text or globally
replace it within the document. The **Preferences** dialog lets you adjust the
editor font size.

### Plugins

Plugins are simple Python modules placed in the `plugins/` directory. Each
plugin must provide a `run(gui)` function. The plugin manager (accessible from
`Tools -> Plugins`) lists available plugins and lets you execute them on the
current document. Selecting a plugin shows its short description taken from the
module's docstring.

#### Highlight plugin

The bundled **highlight** plugin uses the [Pygments](https://pygments.org)
library to provide syntax highlighting for twelve languages. The language is
chosen from a drop-down list of names in the GUI:

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

#### Spell check plugin

The **spellcheck** plugin relies on the `pyspellchecker` library to detect
misspelled words and underline them in red. Install it with:

```sh
pip install pyspellchecker
```

#### Auto-format plugin

The **autoformat** plugin uses [Black](https://black.readthedocs.io/) to format
Python code in the editor. Install Black with:

```sh
pip install black
```

#### Word frequency plugin

The **freq_plot** plugin uses `matplotlib` to display a bar chart of the most
common words in the current document. You can install matplotlib via:

```sh
pip install matplotlib
```

#### Word count plugin

An additional **wordcount** plugin counts the number of words in the current
document. It serves as a minimal example of the plugin interface and can be
invoked through the plugin manager.

### Build & Run

The *Build & Run* command runs `make` in the repository root and then launches
the freshly built `tuxpad` binary on the currently loaded file, if any.

## Cleaning

To clean build artifacts run:

```sh
make clean
```
