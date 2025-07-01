"""Auto-format Python code using Black."""

import tkinter as tk
from tkinter import messagebox

try:
    import black
except ImportError:  # pragma: no cover - optional dependency
    black = None


def run(gui=None):
    if gui is None:
        print("Autoformat plugin requires a GUI context")
        return
    if black is None:
        messagebox.showerror("Format", "black is not installed")
        return

    text_widget = gui.text
    code = text_widget.get("1.0", "end-1c")
    try:
        formatted = black.format_str(code, mode=black.FileMode())
    except Exception as exc:
        messagebox.showerror("Format", str(exc))
        return

    text_widget.delete("1.0", tk.END)
    text_widget.insert(tk.END, formatted)
    messagebox.showinfo("Format", "Formatted with Black")
