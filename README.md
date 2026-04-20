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

## JavaScript API (No local clone required)

VEN also provides a browser/Node-compatible JavaScript library so developers can
use file-metric APIs and plugin hooks through CDNs without downloading this
repository.

### GitHub CDN options

Use one of these endpoints (replace `<owner>`, `<repo>`, and `<tag>`):

```html
<!-- jsDelivr via GitHub -->
<script src="https://cdn.jsdelivr.net/gh/<owner>/<repo>@<tag>/js/ven.js"></script>

<!-- GitHub raw CDN style -->
<script src="https://raw.githubusercontent.com/<owner>/<repo>/<tag>/js/ven.js"></script>
```

### Browser example

```html
<script src="https://cdn.jsdelivr.net/gh/<owner>/<repo>@<tag>/js/ven.js"></script>
<script>
  const result = VEN.scan('hello\nworld\n', { plugins: 'none' });
  console.log(result.metrics); // { lines: 2, words: 2, bytes: 12 }
</script>
```

### Node.js example

```js
const VEN = require('./js/ven.js');

VEN.use('uppercaseWords', ({ text }) => text.toUpperCase().split(/\s+/).length);
const result = VEN.scan('ven makes terminal tools simple', { plugins: 'all' });
console.log(result);
```

### JS API reference

- `VEN.countLines(text)`
- `VEN.countWords(text)`
- `VEN.countBytes(text)`
- `VEN.scan(text, { plugins: 'none' | 'all' | string | string[] })`
- `VEN.use(name, pluginFn)`
- `VEN.listPlugins()`
- `VEN.runPlugin(name, payload)`

## Plugin model (C CLI)

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
