let editor;
let currentFileName = '';
function init() {
    editor = CodeMirror(document.getElementById('editor'), {
        lineNumbers: true,
        mode: 'javascript',
        theme: 'default'
    });
    document.getElementById('newBtn').addEventListener('click', newFile);
    document.getElementById('openBtn').addEventListener('click', () => document.getElementById('fileInput').click());
    document.getElementById('saveBtn').addEventListener('click', saveFile);
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
