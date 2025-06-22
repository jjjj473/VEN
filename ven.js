let editor;
let currentFileName = '';
let fontSize = 14;
function setFontSize(size) {
    fontSize = size;
    editor.getWrapperElement().style.fontSize = fontSize + 'px';
    editor.refresh();
}
function incFont() { setFontSize(fontSize + 1); }
function decFont() { setFontSize(Math.max(8, fontSize - 1)); }

function init() {
    editor = CodeMirror(document.getElementById('editor'), {
        lineNumbers: true,
        mode: 'javascript',
        theme: 'default'
    });
    setFontSize(fontSize);
    document.getElementById('newBtn').addEventListener('click', newFile);
    document.getElementById('openBtn').addEventListener('click', () => document.getElementById('fileInput').click());
    document.getElementById('saveBtn').addEventListener('click', saveFile);
    document.getElementById('undoBtn').addEventListener('click', undo);
    document.getElementById('redoBtn').addEventListener('click', redo);
    document.getElementById('cutBtn').addEventListener('click', cutSelection);
    document.getElementById('copyBtn').addEventListener('click', copySelection);
    document.getElementById('pasteBtn').addEventListener('click', pasteFromClipboard);
    document.getElementById('findBtn').addEventListener('click', findPrompt);
    document.getElementById('replaceBtn').addEventListener('click', replacePrompt);
    document.getElementById('gotoBtn').addEventListener('click', gotoLinePrompt);
    document.getElementById('selectAllBtn').addEventListener('click', selectAll);
    document.getElementById('fontIncBtn').addEventListener('click', incFont);
    document.getElementById('fontDecBtn').addEventListener('click', decFont);
    document.getElementById('numbersBtn').addEventListener('click', toggleNumbers);
    document.getElementById('helpBtn').addEventListener('click', showHelp);
    document.getElementById('closeHelp').addEventListener('click', () => toggleHelp(false));
    document.getElementById('fileInput').addEventListener('change', openFile);
    document.getElementById('commandInput').addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
            handleCommand(e.target.value);
            e.target.value = '';
        }
    });
    fetch('help.txt').then(r => r.text()).then(t => {
        document.getElementById('helpText').textContent = t;
    });
}
function newFile() {
    editor.setValue('');
    currentFileName = '';
}
function openFile(e) {
    const file = e.target.files[0];
    if (file) {
        currentFileName = file.name;
        const reader = new FileReader();
        reader.onload = () => editor.setValue(reader.result);
        reader.readAsText(file);
    }
}
function undo() { editor.undo(); }
function redo() { editor.redo(); }
function cutSelection() {
    const sel = editor.getSelection();
    if (sel) {
        navigator.clipboard.writeText(sel).then(() => editor.replaceSelection(''));
    }
}
function copySelection() {
    const sel = editor.getSelection();
    if (sel) navigator.clipboard.writeText(sel);
}
function pasteFromClipboard() {
    navigator.clipboard.readText().then(t => editor.replaceSelection(t));
}
function findPrompt() {
    const text = prompt('Find:');
    if (!text) return;
    const cur = editor.getSearchCursor(text);
    if (cur.findNext()) {
        editor.setSelection(cur.from(), cur.to());
        editor.scrollIntoView({from: cur.from(), to: cur.to()});
    } else {
        alert('Not found');
    }
}
function replacePrompt() {
    const a = prompt('Find text:');
    if (a === null) return;
    const b = prompt('Replace with:');
    if (b === null) return;
    handleCommand(`replace ${a} ${b}`);
}
function gotoLinePrompt() {
    const line = parseInt(prompt('Goto line:') || '', 10);
    if (!isNaN(line)) editor.setCursor({line: line-1, ch: 0});
}
function selectAll() { editor.execCommand('selectAll'); }
function toggleNumbers() { editor.setOption('lineNumbers', !editor.getOption('lineNumbers')); }
function preview() {
    const blob = new Blob([editor.getValue()], {type: 'text/plain'});
    const url = URL.createObjectURL(blob);
    window.open(url, '_blank');
    setTimeout(() => URL.revokeObjectURL(url), 1000);
}
function saveFile() {
    const blob = new Blob([editor.getValue()], {type: 'text/plain'});
    const a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = currentFileName || 'untitled.txt';
    a.click();
    URL.revokeObjectURL(a.href);
}
function toggleHelp(show) {
    document.getElementById('help').classList.toggle('hidden', !show);
}
function showHelp() { toggleHelp(true); }
function handleCommand(cmd) {
    if (cmd.startsWith('w')) {
        if (cmd.length > 2) currentFileName = cmd.slice(2).trim();
        saveFile();
    } else if (cmd === 'q') {
        if (confirm('Close the editor? Unsaved changes will be lost.')) {
            window.close();
        }
    } else if (cmd.startsWith('o ')) {
        document.getElementById('fileInput').click();
    } else if (cmd === 'help') {
        showHelp();
    } else if (cmd === 'undo') {
        undo();
    } else if (cmd === 'redo') {
        redo();
    } else if (cmd === 'cut') {
        cutSelection();
    } else if (cmd === 'copy') {
        copySelection();
    } else if (cmd === 'paste') {
        pasteFromClipboard();
    } else if (cmd.startsWith('find ')) {
        const text = cmd.slice(5);
        const cur = editor.getSearchCursor(text);
        if (cur.findNext()) {
            editor.setSelection(cur.from(), cur.to());
            editor.scrollIntoView({from: cur.from(), to: cur.to()});
        } else {
            alert('Not found');
        }
    } else if (cmd === 'selectall') {
        selectAll();
    } else if (cmd.startsWith('fontsize ')) {
        const arg = cmd.slice(9).trim();
        if (arg === '+') incFont();
        else if (arg === '-') decFont();
        else if (!isNaN(parseInt(arg))) setFontSize(parseInt(arg));
    } else if (cmd === 'numbers') {
        toggleNumbers();
    } else if (cmd === 'preview') {
        preview();
    } else if (cmd.startsWith('goto ')) {
        const line = parseInt(cmd.slice(5).trim());
        if (!isNaN(line)) editor.setCursor({line: line-1, ch: 0});
    } else if (cmd.startsWith('replace ')) {
        const parts = cmd.slice(8).split(' ');
        if (parts.length >= 2) {
            const search = new RegExp(parts[0], 'g');
            const repl = parts.slice(1).join(' ');
            editor.setValue(editor.getValue().replace(search, repl));
        }
    } else {
        alert('Unknown command: ' + cmd);
    }
}
window.addEventListener('load', init);
