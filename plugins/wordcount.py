"""Word count plugin for Tuxpad."""

import re

def run(gui=None):
    if not gui:
        print("No GUI context provided")
        return
    text = gui.text.get("1.0", "end-1c")
    words = re.findall(r"\w+", text)
    count = len(words)
    try:
        from tkinter import messagebox
        messagebox.showinfo("Word Count", f"Document has {count} words")
    except Exception:
        print(f"Word count: {count}")

