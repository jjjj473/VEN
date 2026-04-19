# VEN

VEN is a prototype code editor in C with runtime plugins, plus a standalone
JavaScript **Markup** library for modern markdown-like parsing and rendering.

## Building the C editor

Use `make` to build `tuxpad` and the native example plugins:

```sh
make
```

Run the program with a file:

```sh
./tuxpad path/to/file.txt
```

Current native plugins:

- `wordcount.so` – counts words in the file
- `sysinfo.so` – prints CPU and memory information
- `diskusage.so` – shows disk usage for the file's filesystem

## Markup JavaScript library (all-JS support)

`Markup` is now fully JavaScript and does not require C bindings.

Library file:

- `scripts/markup.js`

APIs:

- `parseMarkup(input)` -> returns structured components
- `renderMarkupHtml(document)` -> returns escaped HTML-like output
- `diagnostics(document)` -> returns smoothness/complexity summaries

### Node.js usage

```js
const Markup = require('./scripts/markup.js');
const doc = Markup.parseMarkup('# Hello\n- Item');
console.log(Markup.diagnostics(doc));
console.log(Markup.renderMarkupHtml(doc));
```

Or run it as a CLI:

```sh
node scripts/markup.js README.md
```

### Browser usage from GitHub (no download)

You can link the library directly from your GitHub repo via jsDelivr:

```html
<script src="https://cdn.jsdelivr.net/gh/<OWNER>/<REPO>@<REF>/scripts/markup.js"></script>
<script>
  const doc = Markup.parseMarkup('# Loaded from GitHub CDN');
  document.body.innerHTML = Markup.renderMarkupHtml(doc);
</script>
```

- `<REF>` can be a branch (`main`), tag, or commit SHA.
- This allows users to call and link the library from JavaScript without downloading files locally.

A ready browser example is included at:

- `web/markup-browser-demo.html`

## Extra targets

Run the JS engine quickly from Make:

```sh
make markup-js
```

## Cleaning

```sh
make clean
```
