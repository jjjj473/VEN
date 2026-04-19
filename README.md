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

`Markup` is fully JavaScript and does not require C bindings.

Library file:

- `scripts/markup.js`

### Core features

- Block parsing: headings, paragraphs, blockquotes, list items, task items,
  table-like rows, fenced code blocks.
- Inline parsing: bold (`**`), italic (`*`), inline code (`` ` ``), links.
- Rich output metadata per component: `line`, `tokens`, `smoothness`, `complexity`.
- Validation + fix guidance: structured error logs with code, severity, and
  suggested fixes.
- Strict mode (`{ strict: true }` or `--strict`) that throws when fatal parse
  issues are detected.

### API

- `parseMarkup(input, options?)` -> `{ components, errors, meta }`
- `renderMarkupHtml(document)` -> escaped HTML-like output
- `diagnostics(document)` -> component diagnostics + error summary
- `formatErrors(errors)` -> human-readable fix log
- `suggestFixes(input, options?)` -> one-call lint/fix report

### Node.js usage

```js
const Markup = require('./scripts/markup.js');

const source = '# Title
#### Jump
Broken **bold
[bad](https://example.com';
const doc = Markup.parseMarkup(source);

console.log(Markup.diagnostics(doc));
console.log(Markup.formatErrors(doc.errors));
console.log(Markup.renderMarkupHtml(doc));
```

CLI usage:

```sh
node scripts/markup.js README.md
node scripts/markup.js README.md --strict
```

### Browser usage from GitHub (no download)

You can link directly from your GitHub repo through jsDelivr:

```html
<script src="https://cdn.jsdelivr.net/gh/<OWNER>/<REPO>@<REF>/scripts/markup.js"></script>
<script>
  const doc = Markup.parseMarkup('# Loaded from GitHub CDN');
  console.log(Markup.formatErrors(doc.errors));
  document.body.innerHTML = Markup.renderMarkupHtml(doc);
</script>
```

- `<REF>` can be `main`, a tag, or a commit SHA.
- This lets users call and link the library from JavaScript without downloading
  the file.

A browser demo is included at:

- `web/markup-browser-demo.html`

## Extra targets

Run the JS engine quickly:

```sh
make markup-js
```

## Cleaning

```sh
make clean
```
