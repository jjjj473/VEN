# VEN

VEN is a tiny server-side scripting language with a custom syntax inspired by PHP. The interpreter is implemented in C and processes HTML files with `<?VEN ... ?>` tags.

## Building

Compile the interpreter with gcc:

```bash
gcc -o ven ven.c
```

## Running a `.ven` file

Render a page by passing it to the `ven` executable:

```bash
./ven index.ven
```

## Syntax

Blocks are delimited by `<?VEN` and `?>`. If the tag begins with `<?VEN=` the interpreter inserts the value of a variable. Normal blocks support a few simple commands:

- `set NAME VALUE` &ndash; assign a variable
- `echo TEXT` &ndash; output text with `${var}` substitutions
- `include FILE` &ndash; include another `.ven` file
- `for VAR START END` ... `endfor` &ndash; loop over a range of numbers

The variable `time` is predefined with the current timestamp.

```html
<p>Current time: <?VEN= time ?></p>
<?VEN
for i 0 2
    echo "<p>${i}</p>"
endfor
?>
```

See `index.ven` for includes and loops. `webapp.ven` demonstrates embedding CSS and JavaScript.
