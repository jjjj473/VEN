# VEN

VEN is a code editor built like Vim but have more to offer.

This repository now includes **TuxPad**, a simple GUI text editor written in Python with Tkinter.

## Running TuxPad

Ensure Python 3 with Tkinter is installed (most Linux distros provide this by default). Run the editor with:

```bash
python3 tuxpad.py
```

This launches a window with many editing features. Beyond open, save and find, the editor now provides a suite of advanced tools such as:

- Insert date/time
- Word count
- Toggle dark mode
- Search and replace
- Go to line
- Convert text to upper/lowercase
- Sort lines
- Toggle word wrap
- Duplicate line
- Remove trailing spaces
- Auto indent
- Insert line numbers
- Syntax highlighting for Python, JavaScript, HTML and C
- Comment/uncomment selection
- Convert between tabs and spaces
- Run the current Python file
- Indent or unindent selected text
- Bookmark lines and jump to them
- Open recent files
- Reload file if changed externally

An **About** option under the Help menu summarizes these capabilities.

A right-click context menu offers quick access to copy, paste, undo, redo and these tools. A status bar at the bottom shows the current cursor position and the editor autosaves periodically.

On startup TuxPad briefly shows a welcome splash screen with a loading effect.
The splash window mimics a Windows setup dialog with a blue theme and progress
bar. While typing, newly inserted characters flash to help track cursor
movement.

Mousepad is traditionally a simple text editor for the Xfce desktop environment. TuxPad extends this idea in a lightweight form that should work on any Linux distribution.
