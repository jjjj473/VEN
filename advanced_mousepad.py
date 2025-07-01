
import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog
import os

class AdvancedMousepad(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title('Advanced Mousepad')
        self.geometry('800x600')
        self.filename = None
        self.dark_mode = False
        self._create_widgets()
        self._create_bindings()

    def _create_widgets(self):
        self.text = tk.Text(self, undo=True)
        self.text.pack(fill=tk.BOTH, expand=1)

        self.menubar = tk.Menu(self)

        file_menu = tk.Menu(self.menubar, tearoff=0)
        file_menu.add_command(label='New', command=self.new_file)
        file_menu.add_command(label='Open', command=self.open_file)
        file_menu.add_command(label='Save', command=self.save_file)
        file_menu.add_command(label='Save As', command=self.save_as)
        file_menu.add_separator()
        file_menu.add_command(label='Exit', command=self.quit)
        self.menubar.add_cascade(label='File', menu=file_menu)

        edit_menu = tk.Menu(self.menubar, tearoff=0)
        edit_menu.add_command(label='Undo', command=self.text.edit_undo)
        edit_menu.add_command(label='Redo', command=self.text.edit_redo)
        edit_menu.add_separator()
        edit_menu.add_command(label='Cut', command=lambda: self.text.event_generate('<<Cut>>'))
        edit_menu.add_command(label='Copy', command=lambda: self.text.event_generate('<<Copy>>'))
        edit_menu.add_command(label='Paste', command=lambda: self.text.event_generate('<<Paste>>'))
        edit_menu.add_separator()
        edit_menu.add_command(label='Find', command=self.find_text)
        self.menubar.add_cascade(label='Edit', menu=edit_menu)

        tools_menu = tk.Menu(self.menubar, tearoff=0)
        tools_menu.add_command(label='Change Font', command=self.change_font)
        tools_menu.add_command(label='Insert Date/Time', command=self.insert_datetime)
        tools_menu.add_command(label='Word Count', command=self.word_count)
        tools_menu.add_command(label='Toggle Dark Mode', command=self.toggle_dark_mode)
        self.menubar.add_cascade(label='Tools', menu=tools_menu)

        self.status = tk.StringVar(value='Ln 1, Col 0')
        self.statusbar = tk.Label(self, textvariable=self.status, anchor='w')
        self.statusbar.pack(fill=tk.X, side=tk.BOTTOM)

        self.config(menu=self.menubar)

    def _create_bindings(self):
        self.bind('<Control-o>', lambda e: self.open_file())
        self.bind('<Control-s>', lambda e: self.save_file())
        self.bind('<Control-n>', lambda e: self.new_file())
        self.bind('<Control-f>', lambda e: self.find_text())
        self.text.bind('<KeyRelease>', self._update_status)
        self.text.bind('<ButtonRelease>', self._update_status)
        self._update_status()

    def new_file(self):
        if self._confirm_discard_changes():
            self.text.delete('1.0', tk.END)
            self.filename = None
            self.title('Advanced Mousepad')
            self._update_status()

    def open_file(self):
        if not self._confirm_discard_changes():
            return
        file = filedialog.askopenfilename()
        if file:
            try:
                with open(file, 'r') as f:
                    data = f.read()
                self.text.delete('1.0', tk.END)
                self.text.insert(tk.END, data)
                self.filename = file
                self.title(f'Advanced Mousepad - {os.path.basename(file)}')
                self._update_status()
            except Exception as e:
                messagebox.showerror('Error', str(e))

    def save_file(self):
        if self.filename:
            try:
                with open(self.filename, 'w') as f:
                    f.write(self.text.get('1.0', tk.END))
                messagebox.showinfo('Saved', 'File saved successfully')
            except Exception as e:
                messagebox.showerror('Error', str(e))
        else:
            self.save_as()

    def save_as(self):
        file = filedialog.asksaveasfilename(defaultextension='txt')
        if file:
            try:
                with open(file, 'w') as f:
                    f.write(self.text.get('1.0', tk.END))
                self.filename = file
                self.title(f'Advanced Mousepad - {os.path.basename(file)}')
                messagebox.showinfo('Saved', 'File saved successfully')
            except Exception as e:
                messagebox.showerror('Error', str(e))

    def find_text(self):
        target = simpledialog.askstring('Find', 'Enter text to find:')
        if target:
            start = self.text.search(target, '1.0', tk.END)
            if start:
                end = f"{start}+{len(target)}c"
                self.text.tag_add('found', start, end)
                self.text.tag_config('found', background='yellow')
                self.text.mark_set(tk.INSERT, end)
                self.text.see(start)
            else:
                messagebox.showinfo('Find', 'Text not found')

    def change_font(self):
        font_family = simpledialog.askstring('Font', 'Enter font family:')
        if font_family:
            size = simpledialog.askinteger('Size', 'Enter font size:', initialvalue=12)
            if size:
                try:
                    self.text.config(font=(font_family, size))
                except tk.TclError:
                    messagebox.showerror('Error', 'Invalid font')

    def insert_datetime(self):
        import datetime
        now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M')
        self.text.insert(tk.INSERT, now)

    def word_count(self):
        data = self.text.get('1.0', tk.END)
        words = len(data.split())
        chars = len(data) - 1
        messagebox.showinfo('Word Count', f'Words: {words}\nCharacters: {chars}')

    def toggle_dark_mode(self):
        self.dark_mode = not self.dark_mode
        if self.dark_mode:
            self.text.config(bg='black', fg='white', insertbackground='white')
            self.statusbar.config(bg='black', fg='white')
        else:
            self.text.config(bg='white', fg='black', insertbackground='black')
            self.statusbar.config(bg=self.cget('bg'), fg='black')

    def _update_status(self, event=None):
        index = self.text.index(tk.INSERT)
        line, col = map(int, index.split('.'))
        self.status.set(f'Ln {line}, Col {col}')

    def _confirm_discard_changes(self):
        if self.text.edit_modified():
            result = messagebox.askyesnocancel('Unsaved Changes', 'Save changes before closing?')
            if result:
                self.save_file()
                return True
            elif result is False:
                return True
            else:
                return False
        return True

if __name__ == '__main__':
    app = AdvancedMousepad()
    app.mainloop()
