import os
import sys
import subprocess
import importlib.util
import tkinter as tk
from tkinter import filedialog, messagebox


class PluginManager:
    """Simple plugin manager that loads plugins from the ../plugins directory."""

    def __init__(self, gui):
        self.gui = gui
        self.plugins_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'plugins'))

    def list_plugins(self):
        return [f[:-3] for f in os.listdir(self.plugins_dir)
                if f.endswith('.py') and f != '__init__.py']

    def load_plugin(self, name):
        path = os.path.join(self.plugins_dir, f"{name}.py")
        if not os.path.exists(path):
            return None
        spec = importlib.util.spec_from_file_location(name, path)
        mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)
        return mod

    def run_plugin(self, name):
        mod = self.load_plugin(name)
        if mod and hasattr(mod, 'run'):
            try:
                mod.run(self.gui)
            except Exception as exc:
                messagebox.showerror('Plugin Error', str(exc))
        else:
            messagebox.showwarning('Plugin', f'Plugin {name} has no run()')

    def open_window(self):
        win = tk.Toplevel(self.gui.root)
        win.title('Plugins')
        lb = tk.Listbox(win)
        for p in self.list_plugins():
            lb.insert(tk.END, p)
        lb.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        def run_selected():
            sel = lb.curselection()
            if sel:
                name = lb.get(sel[0])
                self.run_plugin(name)

        run_btn = tk.Button(win, text='Run', command=run_selected)
        run_btn.pack(side=tk.RIGHT)


class TuxpadGUI:
    def __init__(self, root):
        self.root = root
        root.title("Tuxpad GUI")
        self.text = tk.Text(root, undo=True)
        self.text.pack(expand=True, fill=tk.BOTH)
        self.filename = None
        self.create_menus()

        # plugin manager
        self.plugin_manager = PluginManager(self)

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
        tools_menu.add_command(label="Spell Check", command=self.spell_check)
        tools_menu.add_command(label="Format Python", command=self.auto_format)
        tools_menu.add_command(label="Word Frequency", command=self.word_frequency)
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
        self.plugin_manager.run_plugin('highlight')

    def spell_check(self):
        self.plugin_manager.run_plugin('spellcheck')

    def auto_format(self):
        self.plugin_manager.run_plugin('autoformat')

    def word_frequency(self):
        self.plugin_manager.run_plugin('freq_plot')

    def preferences(self):
        messagebox.showinfo("Preferences", "Preferences dialog not implemented yet.")

    def plugins(self):
        self.plugin_manager.open_window()

    def build_run(self):
        root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
        subprocess.run(['make'], cwd=root_dir)
        if self.filename:
            subprocess.run([os.path.join(root_dir, 'tuxpad'), self.filename])
        else:
            messagebox.showinfo("Build & Run", "No file loaded to run.")


def main():
    root = tk.Tk()
    app = TuxpadGUI(root)
    root.mainloop()


if __name__ == '__main__':
    main()
