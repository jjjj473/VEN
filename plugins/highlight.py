"""Highlight plugin for Tuxpad."""

def run(gui=None):
    print("Running highlight plugin")
    # Placeholder for actual highlighting logic
    if gui:
        try:
            from tkinter import messagebox
            messagebox.showinfo("Highlight", "Highlight plugin executed")
        except Exception:
            pass

