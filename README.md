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
loading a plugin from `plugins/wordcount.so` if it exists. Plugins must export
a `void run(const char *filename)` function. The provided wordcount plugin
prints the number of words in the file.

### Plugins

Plugins are C shared libraries stored in the `plugins/` directory. Each library
should define a `run` function accepting the file name to operate on. The
`tuxpad` executable demonstrates loading `wordcount.so` after counting lines.

Only the simple wordcount plugin is provided for now.

### Build & Run

`make` will also build `plugins/wordcount.so` so running `./tuxpad FILE` will
both count lines and, if the plugin exists, count words via the plugin.

## Cleaning

To clean build artifacts run:

```sh
make clean
```
