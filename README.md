# VEN Web Editor

VEN is now a browser-based code editor built with HTML, CSS and JavaScript.
It uses the [CodeMirror](https://codemirror.net/) library to provide a
feature-rich editing experience similar to Vim but with a graphical interface.

## Features

- Syntax highlighted editing for JavaScript, HTML and CSS
- Command field for executing Vim-like commands
- Toolbar buttons for common actions (New, Open, Save, Help)
- File open and save using the browser's file APIs
- Help dialog describing available commands

## Commands

In the command field (bottom of the page) you can type `:command` names:

- `:w [file]` – save the current buffer, optionally specifying a file name
- `:q` – close the editor window
- `:o [file]` – open a local file
- `:new` – clear the editor
- `:help` – show the help dialog
- `:goto N` – jump to a specific line number
- `:replace A B` – replace text A with B in the entire buffer

## Usage

Open `index.html` in any modern web browser. No build step is required.

This is a simple prototype and contributions are welcome.
