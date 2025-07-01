import os
import sys
import subprocess
import tkinter as tk
from tkinter import filedialog, messagebox


class TuxpadGUI:
    def __init__(self, root):
        self.root = root
        root.title("Tuxpad GUI")
        self.text = tk.Text(root, undo=True)
        self.text.pack(expand=True, fill=tk.BOTH)
        self.filename = None
        self.create_menus()

    def create_menus(self):
        menubar = tk.Menu(self.root)

        file_menu = tk.Menu(menubar, tearoff=0)
        file_menu.add_command(label="New", command=self.new_file)
        file_menu.add_command(label="Open", command=self.open_file)
        file_menu.add_command(label="Save", command=self.save_file)
        file_menu.add_separator()
        file_menu.add_command(label="Close", command=self.root.quit)
        menubar.add_cascade(label="File", menu=file_menu)

        edit_menu = tk.Menu(menubar, tearoff=0)
        edit_menu.add_command(label="Search", command=self.search)
        edit_menu.add_command(label="Replace", command=self.replace)
        edit_menu.add_command(label="Undo", command=self.text.edit_undo)
        edit_menu.add_command(label="Redo", command=self.text.edit_redo)
        menubar.add_cascade(label="Edit", menu=edit_menu)

        tools_menu = tk.Menu(menubar, tearoff=0)
        tools_menu.add_command(label="Highlight", command=self.highlight)
        tools_menu.add_command(label="Preferences", command=self.preferences)
        tools_menu.add_command(label="Plugins", command=self.plugins)
        tools_menu.add_command(label="Build & Run", command=self.build_run)
        menubar.add_cascade(label="Tools", menu=tools_menu)

        self.root.config(menu=menubar)

    def new_file(self):
        self.text.delete('1.0', tk.END)
        self.filename = None

    def open_file(self):
        fname = filedialog.askopenfilename()
        if fname:
            try:
                with open(fname, 'r') as f:
                    data = f.read()
                self.text.delete('1.0', tk.END)
                self.text.insert(tk.END, data)
                self.filename = fname
            except OSError:
                messagebox.showerror("Error", f"Cannot open file {fname}")

    def save_file(self):
        if not self.filename:
            fname = filedialog.asksaveasfilename()
            if not fname:
                return
            self.filename = fname
        try:
            with open(self.filename, 'w') as f:
                f.write(self.text.get('1.0', tk.END))
        except OSError:
            messagebox.showerror("Error", f"Cannot save file {self.filename}")

    def search(self):
        messagebox.showinfo("Search", "Search feature not implemented yet.")

    def replace(self):
        messagebox.showinfo("Replace", "Replace feature not implemented yet.")

    def highlight(self):
        script = os.path.join(os.path.dirname(__file__), 'highlight.py')
        subprocess.run([sys.executable, script])

    def preferences(self):
        messagebox.showinfo("Preferences", "Preferences dialog not implemented yet.")

    def plugins(self):
        messagebox.showinfo("Plugins", "Plugin manager not implemented yet.")

    def build_run(self):
        messagebox.showinfo("Build & Run", "Build system integration not implemented yet.")


def main():
    root = tk.Tk()
    app = TuxpadGUI(root)
    root.mainloop()


if __name__ == '__main__':
    main()
