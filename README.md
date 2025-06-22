# VEN

VEN is a minimal server-side scripting language inspired by PHP. It allows you to mix HTML with VEN code blocks.

## Running a .ven file

Use the `ven.py` interpreter to render a `.ven` file:

```bash
python ven.py index.ven
```

## Syntax

Code blocks are wrapped in `<?VEN ... ?>` tags. If the tag starts with `<?VEN=` the expression is evaluated and inserted into the output. Otherwise the code is executed and anything printed becomes part of the result. A few helper functions are available inside each block:

- `echo(*args)` prints text without adding spaces or newlines
- `include(path)` renders another `.ven` file relative to the current one
- `escape(text)` escapes HTML special characters

```html
<p>The time is <?VEN= time.strftime("%H:%M:%S") ?></p>
<?VEN
for i in range(3):
    print(f"<p>{i}</p>")
?>
```

See `index.ven` for a complete example that includes other files.
