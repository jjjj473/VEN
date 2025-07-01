"""Syntax highlighting plugin for Tuxpad."""

import tkinter as tk
from tkinter import messagebox
from pygments import lex
from pygments.lexers import get_lexer_by_name
from pygments.token import Token

LANGUAGES = {
    "Python": "python",
    "C": "c",
    "C++": "cpp",
    "Java": "java",
    "JavaScript": "javascript",
    "HTML": "html",
    "CSS": "css",
    "Ruby": "ruby",
    "Go": "go",
    "Rust": "rust",
    "Bash": "bash",
    "Markdown": "markdown",
}

COLOR_MAP = {
    Token.Keyword: "blue",
    Token.Name: "black",
    Token.Comment: "grey",
    Token.String: "green",
    Token.Number: "purple",
    Token.Operator: "red",
}


def _get_color(tok):
    while tok not in COLOR_MAP and tok.parent is not None:
        tok = tok.parent
    return COLOR_MAP.get(tok, "black")


def apply_highlight(gui, lang):
    text_widget = gui.text
    code = text_widget.get("1.0", "end-1c")
    lexer = get_lexer_by_name(lang, stripall=True)

    # Remove old highlight tags
    for tag in text_widget.tag_names():
        if tag.startswith("tok_"):
            text_widget.tag_delete(tag)

    offset = 0
    for tok, value in lex(code, lexer):
        length = len(value)
        if length == 0:
            continue
        start = f"1.0 + {offset}c"
        end = f"1.0 + {offset + length}c"
        color = _get_color(tok)
        tag = f"tok_{tok}"
        if not text_widget.tag_cget(tag, "foreground"):
            text_widget.tag_config(tag, foreground=color)
        text_widget.tag_add(tag, start, end)
        offset += length


def run(gui=None):
    """Run the highlight plugin with a simple language selector."""
    if gui is None:
        print("Highlight plugin requires a GUI context")
        return

    win = tk.Toplevel(gui.root)
    win.title("Highlight")

    var = tk.StringVar(win)
    # Show friendly language names in the dropdown
    var.set("Python")
    options = tk.OptionMenu(win, var, *LANGUAGES.keys())
    options.pack(padx=10, pady=10)

    def _do_highlight():
        lang_name = var.get()
        lang = LANGUAGES.get(lang_name, lang_name)
        apply_highlight(gui, lang)
        try:
            messagebox.showinfo("Highlight", f"Applied {lang_name} highlighting")
        except Exception:
            pass
        win.destroy()

    tk.Button(win, text="Highlight", command=_do_highlight).pack(pady=5)

