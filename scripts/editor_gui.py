import os
import sys
import subprocess
import importlib.util
import tkinter as tk
from tkinter import filedialog, messagebox

# Exit gracefully if no display is available. This avoids a confusing Tkinter
# traceback when running in a headless environment.
if not os.environ.get("DISPLAY"):
    print("Error: no DISPLAY environment variable. GUI cannot start.")
    sys.exit(1)


class PluginManager:
    """Simple plugin manager that loads plugins from the ../plugins directory."""

    def __init__(self, gui):
        self.gui = gui
        self.plugins_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'plugins'))

    def list_plugins(self):
        return [f[:-3] for f in os.listdir(self.plugins_dir)
                if f.endswith('.py') and f != '__init__.py']

    def get_description(self, name):
        """Return the first line of a plugin's docstring without importing it."""
        import ast
        path = os.path.join(self.plugins_dir, f"{name}.py")
        try:
            with open(path, 'r') as f:
                tree = ast.parse(f.read(), filename=path)
            doc = ast.get_docstring(tree)
            return doc.splitlines()[0] if doc else ''
        except Exception:
            return ''

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
        frame = tk.Frame(win)
        frame.pack(fill=tk.BOTH, expand=True)

        lb = tk.Listbox(frame)
        for p in self.list_plugins():
            lb.insert(tk.END, p)
        lb.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        desc_var = tk.StringVar(frame)
        desc_label = tk.Label(frame, textvariable=desc_var, wraplength=200, justify=tk.LEFT)
        desc_label.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5)

        def show_desc(event=None):
            sel = lb.curselection()
            if sel:
                name = lb.get(sel[0])
                desc_var.set(self.get_description(name))
            else:
                desc_var.set('')

        lb.bind('<<ListboxSelect>>', show_desc)

        def run_selected():
            sel = lb.curselection()
            if sel:
                name = lb.get(sel[0])
                self.run_plugin(name)

        run_btn = tk.Button(win, text='Run', command=run_selected)
        run_btn.pack(side=tk.BOTTOM, pady=5)


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
        """Find text in the current document and highlight matches."""
        win = tk.Toplevel(self.root)
        win.title("Search")
        tk.Label(win, text="Find:").pack(side=tk.LEFT, padx=5)
        entry = tk.Entry(win)
        entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)

        def do_search():
            self.text.tag_remove("search_match", "1.0", tk.END)
            query = entry.get()
            if not query:
                return
            start = "1.0"
            while True:
                pos = self.text.search(query, start, stopindex=tk.END)
                if not pos:
                    break
                end = f"{pos}+{len(query)}c"
                self.text.tag_add("search_match", pos, end)
                start = end
            self.text.tag_config("search_match", background="yellow")
            win.destroy()

        tk.Button(win, text="Find", command=do_search).pack(side=tk.RIGHT, padx=5)

    def replace(self):
        """Replace occurrences of text in the current document."""
        win = tk.Toplevel(self.root)
        win.title("Replace")

        tk.Label(win, text="Find:").grid(row=0, column=0, padx=5, pady=2)
        find_entry = tk.Entry(win)
        find_entry.grid(row=0, column=1, padx=5, pady=2)

        tk.Label(win, text="Replace with:").grid(row=1, column=0, padx=5, pady=2)
        replace_entry = tk.Entry(win)
        replace_entry.grid(row=1, column=1, padx=5, pady=2)

        def do_replace():
            find_text = find_entry.get()
            replace_text = replace_entry.get()
            if not find_text:
                return
            content = self.text.get("1.0", tk.END)
            new_content = content.replace(find_text, replace_text)
            self.text.delete("1.0", tk.END)
            self.text.insert(tk.END, new_content)
            win.destroy()

        tk.Button(win, text="Replace", command=do_replace).grid(row=2, column=0, columnspan=2, pady=4)

    def highlight(self):
        self.plugin_manager.run_plugin('highlight')

    def spell_check(self):
        self.plugin_manager.run_plugin('spellcheck')

    def auto_format(self):
        self.plugin_manager.run_plugin('autoformat')

    def word_frequency(self):
        self.plugin_manager.run_plugin('freq_plot')

    def preferences(self):
        """Open a simple preferences dialog to change font size."""
        win = tk.Toplevel(self.root)
        win.title("Preferences")
        tk.Label(win, text="Font size:").pack(side=tk.LEFT, padx=5, pady=5)
        size_var = tk.StringVar(win)
        current_size = self.text.cget("font").split()[-1] if " " in self.text.cget("font") else "12"
        size_var.set(current_size)
        entry = tk.Entry(win, textvariable=size_var, width=5)
        entry.pack(side=tk.LEFT, padx=5, pady=5)

        def apply_prefs():
            size = entry.get()
            try:
                new_font = ("TkDefaultFont", int(size))
                self.text.config(font=new_font)
                win.destroy()
            except ValueError:
                messagebox.showerror("Preferences", "Invalid font size")

        tk.Button(win, text="Apply", command=apply_prefs).pack(side=tk.RIGHT, padx=5, pady=5)

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
