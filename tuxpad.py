
import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog
import os
from pygments import lex
from pygments.lexers import get_lexer_by_name
from pygments.token import Token

class TuxPad(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title('TuxPad')
        self.geometry('800x600')
        self.filename = None
        self.dark_mode = False
        self.bookmarks = set()
        self.recent_files = []
        self.last_mtime = None
        self._create_widgets()
        self._create_bindings()
        self._start_auto_save()
        self._watch_file()

    def _create_widgets(self):
        self.text = tk.Text(self, undo=True, wrap=tk.WORD)
        self.text.pack(fill=tk.BOTH, expand=1)

        self.menubar = tk.Menu(self)

        file_menu = tk.Menu(self.menubar, tearoff=0)
        file_menu.add_command(label='New', command=self.new_file)
        file_menu.add_command(label='Open', command=self.open_file)
        file_menu.add_command(label='Save', command=self.save_file)
        file_menu.add_command(label='Save As', command=self.save_as)
        self.recent_menu = tk.Menu(file_menu, tearoff=0)
        file_menu.add_cascade(label="Open Recent", menu=self.recent_menu)
        file_menu.add_command(label="Reload", command=self.reload_file)
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
        tools_menu.add_separator()
        tools_menu.add_command(label='Search && Replace', command=self.search_replace)
        tools_menu.add_command(label='Go To Line', command=self.goto_line)
        tools_menu.add_command(label='To Uppercase', command=self.to_uppercase)
        tools_menu.add_command(label='To Lowercase', command=self.to_lowercase)
        tools_menu.add_command(label='Sort Lines', command=self.sort_lines)
        tools_menu.add_command(label='Toggle Wrap', command=self.toggle_wrap)
        tools_menu.add_command(label='Duplicate Line', command=self.duplicate_line)
        tools_menu.add_command(label='Remove Trailing Spaces', command=self.remove_trailing_spaces)
        tools_menu.add_command(label='Auto Indent', command=self.auto_indent)
        tools_menu.add_command(label='Insert Line Numbers', command=self.insert_line_numbers)
        syntax_menu = tk.Menu(tools_menu, tearoff=0)
        syntax_menu.add_command(label='Python', command=lambda: self.highlight_syntax('python'))
        syntax_menu.add_command(label='JavaScript', command=lambda: self.highlight_syntax('javascript'))
        syntax_menu.add_command(label='HTML', command=lambda: self.highlight_syntax('html'))
        syntax_menu.add_command(label='C', command=lambda: self.highlight_syntax('c'))
        tools_menu.add_cascade(label='Syntax Highlight', menu=syntax_menu)
        tools_menu.add_command(label='Clear Highlighting', command=self.clear_highlighting)
        tools_menu.add_command(label="Indent Selection", command=self.indent_selection)
        tools_menu.add_command(label="Unindent Selection", command=self.unindent_selection)
        tools_menu.add_command(label="Toggle Bookmark", command=self.toggle_bookmark)
        tools_menu.add_command(label="Goto Bookmark", command=self.goto_bookmark)
        tools_menu.add_separator()
        tools_menu.add_command(label='Comment Selection', command=self.comment_selection)
        tools_menu.add_command(label='Uncomment Selection', command=self.uncomment_selection)
        tools_menu.add_command(label='Tabs to Spaces', command=self.tabs_to_spaces)
        tools_menu.add_command(label='Spaces to Tabs', command=self.spaces_to_tabs)
        tools_menu.add_command(label='Run Python', command=self.run_python)
        self.menubar.add_cascade(label='Tools', menu=tools_menu)

        help_menu = tk.Menu(self.menubar, tearoff=0)
        help_menu.add_command(label='About', command=self.show_about)
        self.menubar.add_cascade(label='Help', menu=help_menu)

        self.status = tk.StringVar(value='Ln 1, Col 0')
        self.statusbar = tk.Label(self, textvariable=self.status, anchor='w')
        self.statusbar.pack(fill=tk.X, side=tk.BOTTOM)

        self.context_menu = tk.Menu(self, tearoff=0)
        self.context_menu.add_command(label='Undo', command=self.text.edit_undo)
        self.context_menu.add_command(label='Redo', command=self.text.edit_redo)
        self.context_menu.add_separator()
        self.context_menu.add_command(label='Cut', command=lambda: self.text.event_generate('<<Cut>>'))
        self.context_menu.add_command(label='Copy', command=lambda: self.text.event_generate('<<Copy>>'))
        self.context_menu.add_command(label='Paste', command=lambda: self.text.event_generate('<<Paste>>'))
        self.context_menu.add_separator()
        self.context_menu.add_command(label='Insert Date/Time', command=self.insert_datetime)
        self.context_menu.add_command(label='Word Count', command=self.word_count)
        self.context_menu.add_command(label='Toggle Dark Mode', command=self.toggle_dark_mode)
        self.context_menu.add_separator()
        self.context_menu.add_command(label='Search && Replace', command=self.search_replace)
        self.context_menu.add_command(label='Go To Line', command=self.goto_line)
        syntax_sub = tk.Menu(self.context_menu, tearoff=0)
        syntax_sub.add_command(label='Python', command=lambda: self.highlight_syntax('python'))
        syntax_sub.add_command(label='JavaScript', command=lambda: self.highlight_syntax('javascript'))
        syntax_sub.add_command(label='HTML', command=lambda: self.highlight_syntax('html'))
        syntax_sub.add_command(label='C', command=lambda: self.highlight_syntax('c'))
        self.context_menu.add_cascade(label='Syntax Highlight', menu=syntax_sub)
        self.context_menu.add_command(label='Clear Highlighting', command=self.clear_highlighting)
        self.context_menu.add_command(label="Indent Selection", command=self.indent_selection)
        self.context_menu.add_command(label="Unindent Selection", command=self.unindent_selection)
        self.context_menu.add_command(label="Toggle Bookmark", command=self.toggle_bookmark)
        self.context_menu.add_command(label="Goto Bookmark", command=self.goto_bookmark)
        self.context_menu.add_separator()
        self.context_menu.add_command(label='Comment Selection', command=self.comment_selection)
        self.context_menu.add_command(label='Uncomment Selection', command=self.uncomment_selection)

        self.config(menu=self.menubar)

    def _create_bindings(self):
        self.bind('<Control-o>', lambda e: self.open_file())
        self.bind('<Control-s>', lambda e: self.save_file())
        self.bind('<Control-n>', lambda e: self.new_file())
        self.bind('<Control-f>', lambda e: self.find_text())
        self.text.bind('<KeyRelease>', self._update_status)
        self.text.bind('<ButtonRelease>', self._update_status)
        self.text.bind('<Button-3>', self._show_context_menu)
        self.text.bind('<Motion>', self._highlight_line)
        self._update_status()

    def new_file(self):
        if self._confirm_discard_changes():
            self.text.delete('1.0', tk.END)
            self.filename = None
            self.title('TuxPad')
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
                self.last_mtime = os.path.getmtime(file)
                self._add_recent(file)
                self._update_recent_menu()
                self.title(f'TuxPad - {os.path.basename(file)}')
                self.last_mtime = os.path.getmtime(file)
                self._add_recent(file)
                self._update_recent_menu()
                self._update_status()
            except Exception as e:
                messagebox.showerror('Error', str(e))

    def save_file(self):
        if self.filename:
            try:
                with open(self.filename, 'w') as f:
                    f.write(self.text.get('1.0', tk.END))
                messagebox.showinfo('Saved', 'File saved successfully')
                self.last_mtime = os.path.getmtime(self.filename)
                self._add_recent(self.filename)
                self._update_recent_menu()
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
                self.last_mtime = os.path.getmtime(file)
                self._add_recent(file)
                self._update_recent_menu()
                self.title(f'TuxPad - {os.path.basename(file)}')
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

    def search_replace(self):
        target = simpledialog.askstring('Replace', 'Find text:')
        if target is None:
            return
        replacement = simpledialog.askstring('Replace', 'Replace with:')
        if replacement is None:
            return
        count = 0
        pos = '1.0'
        while True:
            pos = self.text.search(target, pos, tk.END)
            if not pos:
                break
            end = f"{pos}+{len(target)}c"
            self.text.delete(pos, end)
            self.text.insert(pos, replacement)
            pos = f"{pos}+{len(replacement)}c"
            count += 1
        messagebox.showinfo('Replace', f'Replaced {count} occurrence(s)')

    def goto_line(self):
        line = simpledialog.askinteger('Go to Line', 'Line number:')
        if line:
            max_index = int(self.text.index(tk.END).split('.')[0])
            if 1 <= line <= max_index:
                self.text.mark_set(tk.INSERT, f'{line}.0')
                self.text.see(f'{line}.0')

    def to_uppercase(self):
        try:
            sel = self.text.get(tk.SEL_FIRST, tk.SEL_LAST).upper()
            self.text.delete(tk.SEL_FIRST, tk.SEL_LAST)
            self.text.insert(tk.INSERT, sel)
        except tk.TclError:
            pass

    def to_lowercase(self):
        try:
            sel = self.text.get(tk.SEL_FIRST, tk.SEL_LAST).lower()
            self.text.delete(tk.SEL_FIRST, tk.SEL_LAST)
            self.text.insert(tk.INSERT, sel)
        except tk.TclError:
            pass

    def sort_lines(self):
        data = self.text.get('1.0', tk.END).splitlines()
        data.sort()
        self.text.delete('1.0', tk.END)
        self.text.insert(tk.END, '\n'.join(data))

    def toggle_wrap(self):
        current = self.text.cget('wrap')
        self.text.config(wrap=tk.NONE if current != tk.NONE else tk.WORD)

    def duplicate_line(self):
        index = self.text.index(tk.INSERT)
        line = index.split('.')[0]
        content = self.text.get(f'{line}.0', f'{line}.0 lineend')
        self.text.insert(f'{line}.0 lineend', '\n' + content)

    def remove_trailing_spaces(self):
        data = [line.rstrip() for line in self.text.get('1.0', tk.END).splitlines()]
        self.text.delete('1.0', tk.END)
        self.text.insert(tk.END, '\n'.join(data))

    def auto_indent(self):
        data = self.text.get('1.0', tk.END).splitlines()
        indented = ['    ' + line for line in data]
        self.text.delete('1.0', tk.END)
        self.text.insert(tk.END, '\n'.join(indented))

    def insert_line_numbers(self):
        lines = self.text.get('1.0', tk.END).splitlines()
        numbered = [f"{i+1}: {l}" for i, l in enumerate(lines)]
        self.text.delete('1.0', tk.END)
        self.text.insert(tk.END, '\n'.join(numbered))

    def highlight_syntax(self, lang):
        data = self.text.get('1.0', tk.END)
        for tag in self.text.tag_names():
            if tag.startswith('Token'):
                self.text.tag_delete(tag)
        try:
            lexer = get_lexer_by_name(lang)
        except Exception:
            messagebox.showerror('Error', f'Unsupported language: {lang}')
            return
        index = '1.0'
        for ttype, value in lex(data, lexer):
            end = self.text.index(f'{index}+{len(value)}c')
            tag = str(ttype)
            self.text.tag_add(tag, index, end)
            index = end
        self._configure_tags()

    def clear_highlighting(self):
        for tag in self.text.tag_names():
            if tag.startswith('Token'):
                self.text.tag_delete(tag)

    def _configure_tags(self):
        self.text.tag_configure('Token.Keyword', foreground='blue')
        self.text.tag_configure('Token.Name.Builtin', foreground='purple')
        self.text.tag_configure('Token.Comment', foreground='grey')
        self.text.tag_configure('Token.String', foreground='green')
        self.text.tag_configure('Token.Number', foreground='darkorange')
        self.text.tag_configure('Token.Operator', foreground='red')

    def comment_selection(self):
        try:
            start = self.text.index(tk.SEL_FIRST)
            end = self.text.index(tk.SEL_LAST)
        except tk.TclError:
            return
        lines = self.text.get(start, end).splitlines()
        commented = ['# ' + l if not l.lstrip().startswith('#') else l for l in lines]
        self.text.delete(start, end)
        self.text.insert(start, '\n'.join(commented))

    def uncomment_selection(self):
        try:
            start = self.text.index(tk.SEL_FIRST)
            end = self.text.index(tk.SEL_LAST)
        except tk.TclError:
            return
        lines = self.text.get(start, end).splitlines()
        uncommented = [l[2:] if l.lstrip().startswith('#') else l for l in lines]
        self.text.delete(start, end)
        self.text.insert(start, '\n'.join(uncommented))

    def tabs_to_spaces(self):
        data = self.text.get('1.0', tk.END).replace('\t', '    ')
        self.text.delete('1.0', tk.END)
        self.text.insert('1.0', data)

    def spaces_to_tabs(self):
        data = self.text.get('1.0', tk.END).replace('    ', '\t')
        self.text.delete('1.0', tk.END)
        self.text.insert('1.0', data)

    def run_python(self):
        if not self.filename:
            messagebox.showinfo('Run', 'Please save file before running.')
            return
        import subprocess, sys
        try:
            output = subprocess.check_output([sys.executable, self.filename], stderr=subprocess.STDOUT, text=True)
            messagebox.showinfo('Output', output)
        except Exception as e:
            messagebox.showerror('Run Error', str(e))

    def indent_selection(self):
        try:
            start = self.text.index(tk.SEL_FIRST)
            end = self.text.index(tk.SEL_LAST)
        except tk.TclError:
            return
        lines = self.text.get(start, end).splitlines()
        indented = ['    ' + l for l in lines]
        self.text.delete(start, end)
        self.text.insert(start, '\n'.join(indented))

    def unindent_selection(self):
        try:
            start = self.text.index(tk.SEL_FIRST)
            end = self.text.index(tk.SEL_LAST)
        except tk.TclError:
            return
        lines = self.text.get(start, end).splitlines()
        unindented = [l[4:] if l.startswith('    ') else l for l in lines]
        self.text.delete(start, end)
        self.text.insert(start, '\n'.join(unindented))

    def toggle_bookmark(self):
        line = int(self.text.index(tk.INSERT).split('.')[0])
        if line in self.bookmarks:
            self.bookmarks.remove(line)
        else:
            self.bookmarks.add(line)

    def goto_bookmark(self):
        if not self.bookmarks:
            messagebox.showinfo('Bookmarks', 'No bookmarks set')
            return
        line = simpledialog.askinteger('Bookmarks', 'Go to line:', initialvalue=min(self.bookmarks))
        if line and line in self.bookmarks:
            self.text.mark_set(tk.INSERT, f'{line}.0')
            self.text.see(f'{line}.0')

    def reload_file(self):
        if not self.filename:
            return
        if self.text.edit_modified() and not messagebox.askyesno('Reload', 'Discard changes and reload?'):
            return
        try:
            with open(self.filename, 'r') as f:
                data = f.read()
            self.text.delete('1.0', tk.END)
            self.text.insert(tk.END, data)
            self.last_mtime = os.path.getmtime(self.filename)
            self.text.edit_modified(False)
        except Exception as e:
            messagebox.showerror('Reload', str(e))

    def _add_recent(self, path):
        if path in self.recent_files:
            self.recent_files.remove(path)
        self.recent_files.insert(0, path)
        self.recent_files = self.recent_files[:5]

    def _update_recent_menu(self):
        self.recent_menu.delete(0, tk.END)
        for p in self.recent_files:
            self.recent_menu.add_command(label=os.path.basename(p), command=lambda f=p: self._open_recent(f))

    def _open_recent(self, path):
        if not self._confirm_discard_changes():
            return
        try:
            with open(path, 'r') as f:
                data = f.read()
            self.text.delete('1.0', tk.END)
            self.text.insert(tk.END, data)
            self.filename = path
            self.title(f'TuxPad - {os.path.basename(path)}')
            self.last_mtime = os.path.getmtime(path)
            self._add_recent(path)
            self._update_recent_menu()
        except Exception as e:
            messagebox.showerror('Error', str(e))

    def _watch_file(self):
        if self.filename and os.path.exists(self.filename):
            mtime = os.path.getmtime(self.filename)
            if self.last_mtime and mtime > self.last_mtime and not self.text.edit_modified():
                self.reload_file()
            self.last_mtime = mtime
        self.after(5000, self._watch_file)
    def _start_auto_save(self):
        if self.filename:
            try:
                with open(self.filename, 'w') as f:
                    f.write(self.text.get('1.0', tk.END))
            except Exception:
                pass
        self.after(300000, self._start_auto_save)

    def show_about(self):
        message = (
            'TuxPad is a feature-rich text editor with tools like syntax '
            'highlighting, bookmarks, indentation helpers, search and '
            'replace, line sorting, case conversion, autosave, recent '
            'files, file watching and more.'
        )
        messagebox.showinfo('About TuxPad', message)

    def _highlight_line(self, event=None):
        self.text.tag_remove('current_line', '1.0', tk.END)
        line = self.text.index(tk.INSERT).split('.')[0]
        self.text.tag_add('current_line', f'{line}.0', f'{line}.0 lineend')
        self.text.tag_config('current_line', background='#ffffcc')

    def _show_context_menu(self, event):
        self.context_menu.tk_popup(event.x_root, event.y_root)

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
    app = TuxPad()
    app.mainloop()
