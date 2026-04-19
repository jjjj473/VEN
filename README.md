# VEN

VEN is a prototype code editor now implemented purely in C. It demonstrates a
very small plugin system that loads tools as shared libraries at runtime.

## Building

Use `make` to build the `tuxpad` binary and example plugin:

```sh
make
```

Run the program by providing a file name:

```sh
./tuxpad path/to/file.txt
```

The program counts the number of lines in the given file and then demonstrates
loading plugins from the `plugins/` directory if they exist. Plugins must
export a `void run(const char *filename)` function. The provided plugins
include a word counter as well as Linux tools that print system and disk
information, plus a new `markup` parser/renderer that extends basic markdown
into richer components.

### Plugins

Plugins are C shared libraries stored in the `plugins/` directory. Each library
should define a `run` function accepting the file name to operate on. The
`tuxpad` executable loads any available plugins after counting lines. Provided
plugins include:

* `wordcount.so` – counts words in the file
* `sysinfo.so` – prints basic CPU and memory information
* `diskusage.so` – shows disk usage for the file's filesystem
* `markup.so` – dispatches to the JavaScript `markup` engine for rich parsing and HTML preview output

### Build & Run

`make` also builds the example plugins so running `./tuxpad FILE` will count
lines and then invoke any plugins found in the `plugins/` directory.

## GUI

An optional Tkinter interface is included for experimenting with the plugins.
Build everything then launch the GUI with:

```sh
make gui
```

The GUI allows you to open a file and run the available plugins from the
`Plugins` menu. If no graphical display is available the GUI will exit with a
message.

## Cleaning

To clean build artifacts run:

```sh
make clean
```

## Markup JavaScript library

`scripts/markup.js` is the new reusable **markup JS library**. It replaces the
prior C-only parsing flow and provides:

- `parseMarkup(input)` -> structured components (heading, paragraph, list item,
  blockquote, code block)
- `renderMarkupHtml(document)` -> escaped HTML-like output
- `diagnostics(document)` -> smoothness/complexity summaries

You can run it directly:

```sh
node scripts/markup.js README.md
```

The `plugins/markup.so` plugin now calls this script so markup behavior is
powered by JavaScript while remaining compatible with the C plugin runtime.
