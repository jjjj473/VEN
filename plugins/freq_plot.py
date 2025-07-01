"""Word frequency plot plugin using matplotlib."""

import re
from collections import Counter
import tkinter as tk
from tkinter import messagebox

try:
    import matplotlib.pyplot as plt
except ImportError:  # pragma: no cover - optional dependency
    plt = None


def run(gui=None):
    if gui is None:
        print("Frequency plot plugin requires a GUI context")
        return
    if plt is None:
        messagebox.showerror("Word Frequency", "matplotlib is not installed")
        return

    text = gui.text.get("1.0", "end-1c").lower()
    words = re.findall(r"\w+", text)
    counts = Counter(words)

    if not counts:
        messagebox.showinfo("Word Frequency", "No words to plot")
        return

    most = counts.most_common(10)
    labels, values = zip(*most)
    plt.figure(figsize=(6, 4))
    plt.bar(labels, values)
    plt.xticks(rotation=45, ha="right")
    plt.title("Word Frequency")
    plt.tight_layout()
    plt.show()
