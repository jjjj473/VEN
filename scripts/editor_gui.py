import os
import glob
import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
import ctypes
import io
import contextlib

PLUGIN_DIR = os.path.join(os.path.dirname(__file__), '..', 'plugins')

class Plugin:
    def __init__(self, path):
        self.path = path
        self.name = os.path.basename(path)
        self.lib = ctypes.CDLL(path)
        self.func = self.lib.run
        self.func.argtypes = [ctypes.c_char_p]
        self.func.restype = None

    def run(self, filename):
        self.func(filename.encode('utf-8'))

class PluginManager:
    def __init__(self, directory=PLUGIN_DIR):
        self.plugins = [Plugin(p) for p in glob.glob(os.path.join(directory, '*.so'))]

    def names(self):
        return [p.name for p in self.plugins]

    def run(self, name, filename):
        for p in self.plugins:
            if p.name == name:
                p.run(filename)
                break

class EditorGUI:
    def __init__(self, root):
        self.root = root
        self.root.title('Tuxpad GUI')
        self.filename = None
        self.plugin_mgr = PluginManager()

        self.text = scrolledtext.ScrolledText(root, width=80, height=20)
        self.text.pack(fill=tk.BOTH, expand=True)

        self.output = scrolledtext.ScrolledText(root, width=80, height=10, bg='#f0f0f0')
        self.output.pack(fill=tk.BOTH, expand=True)

        menubar = tk.Menu(root)
        file_menu = tk.Menu(menubar, tearoff=0)
        file_menu.add_command(label='Open', command=self.open_file)
        menubar.add_cascade(label='File', menu=file_menu)

        plugin_menu = tk.Menu(menubar, tearoff=0)
        for name in self.plugin_mgr.names():
            plugin_menu.add_command(label=name, command=lambda n=name: self.run_plugin(n))
        menubar.add_cascade(label='Plugins', menu=plugin_menu)

        root.config(menu=menubar)

    def open_file(self):
        path = filedialog.askopenfilename()
        if path:
            self.filename = path
            with open(path, 'r', errors='ignore') as f:
                self.text.delete('1.0', tk.END)
                self.text.insert(tk.END, f.read())

    def run_plugin(self, name):
        if not self.filename:
            messagebox.showwarning('No file', 'Open a file first.')
            return
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            self.plugin_mgr.run(name, self.filename)
        self.output.insert(tk.END, f'[{name}]\n{buf.getvalue()}\n')

if __name__ == '__main__':
    if not os.environ.get('DISPLAY'):
        print('No display available. Skipping GUI.')
        raise SystemExit(0)
    root = tk.Tk()
    app = EditorGUI(root)
    root.mainloop()
