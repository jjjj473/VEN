"""Spell check plugin using pyspellchecker."""

import tkinter as tk
from tkinter import messagebox

try:
    from spellchecker import SpellChecker
except ImportError:  # pragma: no cover - optional dependency
    SpellChecker = None


def run(gui=None):
    if gui is None:
        print("Spell check plugin requires a GUI context")
        return
    if SpellChecker is None:
        messagebox.showerror("Spell Check", "pyspellchecker is not installed")
        return

    text_widget = gui.text
    text = text_widget.get("1.0", "end-1c")
    spell = SpellChecker()
    misspelled = spell.unknown(text.split())

    text_widget.tag_remove("spell_error", "1.0", tk.END)

    if not misspelled:
        messagebox.showinfo("Spell Check", "No spelling mistakes found")
        return

    for word in misspelled:
        start = "1.0"
        while True:
            pos = text_widget.search(word, start, tk.END)
            if not pos:
                break
            end = f"{pos}+{len(word)}c"
            text_widget.tag_add("spell_error", pos, end)
            start = end

    text_widget.tag_config("spell_error", underline=True, foreground="red")
    messagebox.showinfo("Spell Check", f"Found {len(misspelled)} mistakes")
