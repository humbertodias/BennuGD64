import { Terminal } from 'https://cdn.jsdelivr.net/npm/@xterm/xterm@5.5.0/+esm';
import { FitAddon } from 'https://cdn.jsdelivr.net/npm/@xterm/addon-fit@0.10.0/+esm';
import { VirtualFS } from './vfs.js';
import { detectDcb, formatDcb } from './dcb.js';
import { compileWithBgdc, dcbNameFor, loadBgdc } from './bgdc.js';

const SAMPLE_NAMES = [
  'hello', 'fire', 'firework', 'rain', 'starfield', 'keyboard', 'joystick', 'wpad'
];

const FALLBACK_HELLO = `import "mod_video"
import "mod_map"
import "mod_draw"
import "mod_key"
import "mod_proc"
import "mod_text"
import "mod_sys"

PROCESS Main()
BEGIN
    set_mode(320, 240, 16);
    set_fps(30, 0);
    write(0, 160, 120, ALIGN_CENTER, "Hello World from " + os_name());
    LOOP
        IF (key(_esc))
            exit();
        END
        FRAME;
    END
END
`;

const vfs = new VirtualFS();
let monacoApi;
let editor;
let currentPath = 'hello.prg';
let editorLocked = false;
let term;
let fit;

const filesEl = document.getElementById('files');
const editorTitle = document.getElementById('editor-title');
const frame = document.getElementById('canvas-frame');
const empty = document.getElementById('player-empty');
const sampleSelect = document.getElementById('sample-select');

function pill(id, text, cls) {
  const el = document.getElementById(id);
  el.textContent = text;
  el.className = 'pill' + (cls ? ' ' + cls : '');
}

function log(line, err) {
  term.writeln((err ? '\x1b[31m' : '') + String(line).replace(/\r/g, '') + (err ? '\x1b[0m' : ''));
}

function loadMonaco() {
  return new Promise((resolve, reject) => {
    const script = document.createElement('script');
    script.src = 'https://cdn.jsdelivr.net/npm/monaco-editor@0.52.2/min/vs/loader.js';
    script.onload = () => {
      window.require.config({
        paths: { vs: 'https://cdn.jsdelivr.net/npm/monaco-editor@0.52.2/min/vs' }
      });
      window.require(['vs/editor/editor.main'], (m) => resolve(m));
    };
    script.onerror = () => reject(new Error('failed to load Monaco'));
    document.head.appendChild(script);
  });
}

function registerBennu(monaco) {
  monaco.languages.register({ id: 'bennu' });
  monaco.languages.setMonarchTokensProvider('bennu', {
    ignoreCase: true,
    keywords: [
      'program', 'begin', 'end', 'process', 'function', 'frame', 'loop',
      'from', 'to', 'step', 'while', 'repeat', 'until', 'for', 'if', 'else',
      'elseif', 'switch', 'case', 'default', 'break', 'continue', 'return',
      'global', 'local', 'private', 'const', 'type', 'struct', 'pointer',
      'import', 'include', 'onexit', 'clone', 'signal'
    ],
    tokenizer: {
      root: [
        [/\/\/.*$/, 'comment'],
        [/\/\*/, 'comment', '@comment'],
        [/"([^"\\]|\\.)*"/, 'string'],
        [/'([^'\\]|\\.)*'/, 'string'],
        [/\b\d+\b/, 'number'],
        [/#\w+/, 'keyword'],
        [/\b(process|function|begin|end|frame|loop|from|to|step|while|repeat|until|if|else|elseif|switch|case|default|break|continue|return|global|local|private|const|type|struct|import|include|program|onexit|clone|signal)\b/, 'keyword'],
        [/[a-zA-Z_][\w]*/, 'identifier']
      ],
      comment: [
        [/[^/*]+/, 'comment'],
        [/\*\//, 'comment', '@pop'],
        [/[/*]/, 'comment']
      ]
    }
  });
}

function extOf(path) {
  const m = path.toLowerCase().match(/\.([^.]+)$/);
  return m ? m[1] : '';
}

function renderExplorer() {
  filesEl.replaceChildren();
  for (const path of vfs.list()) {
    const btn = document.createElement('button');
    btn.type = 'button';
    btn.className = 'file ' + extOf(path);
    if (path === currentPath) btn.classList.add('active');
    const name = document.createElement('span');
    name.textContent = path;
    const ext = document.createElement('span');
    ext.className = 'ext';
    ext.textContent = extOf(path) || 'file';
    btn.append(name, ext);
    btn.onclick = () => openFile(path);
    filesEl.appendChild(btn);
  }
}

function languageFor(path) {
  if (/\.prg$/i.test(path)) return 'bennu';
  if (/\.(json)$/i.test(path)) return 'json';
  return 'plaintext';
}

function openFile(path) {
  flushEditor();
  currentPath = path;
  editorTitle.textContent = path;
  const binary = !vfs.isText(path);
  editorLocked = binary;
  if (binary) {
    const bytes = vfs.read(path) || new Uint8Array();
    const info = detectDcb(bytes);
    const header = info.ok
      ? formatDcb(info)
      : `${path} (${bytes.length} bytes)\nNot a text file.`;
    editor.setValue(header);
    monacoApi.editor.setModelLanguage(editor.getModel(), 'plaintext');
    editor.updateOptions({ readOnly: true });
  } else {
    editor.updateOptions({ readOnly: false });
    editor.setValue(vfs.readText(path));
    monacoApi.editor.setModelLanguage(editor.getModel(), languageFor(path));
  }
  renderExplorer();
}

function flushEditor() {
  if (!editor || !currentPath || editorLocked) return;
  if (vfs.isText(currentPath)) vfs.write(currentPath, editor.getValue());
}

function sourcePath() {
  if (/\.prg$/i.test(currentPath)) return currentPath;
  const prgs = vfs.list().filter((p) => /\.prg$/i.test(p));
  return prgs[0] || currentPath;
}

function setDcbPill(info) {
  if (!info || !info.ok) {
    pill('pill-dcb', 'dcb', '');
    return;
  }
  pill('pill-dcb', `${info.kind} ${info.label}`, info.runtimeOk ? 'ok' : 'warn');
}

async function compile() {
  flushEditor();
  const src = sourcePath();
  if (!/\.prg$/i.test(src)) {
    log('Open a .prg file to compile.', true);
    return null;
  }
  log(`$ bgdc.wasm -g -o ${dcbNameFor(src)} ${src}`);
  pill('pill-bgdc', 'bgdc…', 'warn');
  try {
    const result = await compileWithBgdc(vfs.snapshot(), src, (line) => log(line));
    vfs.merge(result.files);
    const out = dcbNameFor(src);
    const bytes = vfs.read(out);
    if (!bytes) {
      pill('pill-bgdc', 'bgdc fail', 'err');
      log(`compile failed (exit ${result.exitCode}); no ${out}`, true);
      setDcbPill(null);
      renderExplorer();
      return null;
    }
    const info = detectDcb(bytes);
    log(formatDcb(info), !info.ok);
    setDcbPill(info);
    pill('pill-bgdc', result.exitCode === 0 ? 'bgdc ok' : `bgdc ${result.exitCode}`, result.exitCode === 0 ? 'ok' : 'warn');
    renderExplorer();
    if (currentPath === out) openFile(out);
    return { path: out, info, ok: info.ok && info.runtimeOk };
  } catch (err) {
    pill('pill-bgdc', 'bgdc fail', 'err');
    log(err && err.message ? err.message : String(err), true);
    return null;
  }
}

function stopGame() {
  frame.hidden = true;
  empty.hidden = false;
  frame.removeAttribute('src');
  pill('pill-bgdi', 'bgdi', '');
}

function runGame(dcbPath) {
  empty.hidden = true;
  frame.hidden = false;
  pill('pill-bgdi', 'bgdi…', 'warn');
  const files = {};
  for (const [path, data] of Object.entries(vfs.snapshot())) {
    files[path] = data;
  }
  const send = () => {
    frame.contentWindow.postMessage({ type: 'run', files, entry: dcbPath }, '*');
  };
  frame.removeAttribute('src');
  frame.onload = send;
  frame.src = 'run.html';
}

async function compileAndRun() {
  const compiled = await compile();
  if (!compiled || !compiled.ok) {
    log('Not running: DCB missing or incompatible.', true);
    return;
  }
  log(`$ bgdi ${compiled.path}`);
  runGame(compiled.path);
}

function onParentMessage(ev) {
  const msg = ev.data;
  if (!msg || typeof msg !== 'object') return;
  if (msg.type === 'bgdi-ready') pill('pill-bgdi', 'bgdi ready', 'ok');
  if (msg.type === 'bgdi-start') {
    pill('pill-bgdi', 'bgdi running', 'ok');
    log(`[bgdi] ${msg.entry}`);
  }
  if (msg.type === 'bgdi-print') log(msg.text, !!msg.err);
  if (msg.type === 'bgdi-exit') {
    pill('pill-bgdi', `bgdi exit ${msg.code}`, msg.code ? 'warn' : 'ok');
    log(`[bgdi] exit ${msg.code}`);
  }
}

async function loadSamples() {
  sampleSelect.replaceChildren();
  const placeholder = document.createElement('option');
  placeholder.value = '';
  placeholder.textContent = 'Sample…';
  sampleSelect.appendChild(placeholder);
  for (const name of SAMPLE_NAMES) {
    const opt = document.createElement('option');
    opt.value = name;
    opt.textContent = name + '.prg';
    sampleSelect.appendChild(opt);
  }

  let hello = FALLBACK_HELLO;
  try {
    const res = await fetch('../samples/hello.prg');
    if (res.ok) hello = await res.text();
  } catch (e) { /* bundled fallback */ }
  vfs.write('hello.prg', hello);
}

async function loadSample(name) {
  const path = `${name}.prg`;
  try {
    const res = await fetch(`../samples/${path}`);
    if (!res.ok) throw new Error(path + ' not in samples/');
    vfs.write(path, await res.text());
  } catch (err) {
    if (name === 'hello') vfs.write(path, FALLBACK_HELLO);
    else {
      log(err.message, true);
      return;
    }
  }
  openFile(path);
}

function bindUi() {
  document.getElementById('btn-compile').onclick = () => compile();
  document.getElementById('btn-run').onclick = () => compileAndRun();
  document.getElementById('btn-stop').onclick = () => stopGame();
  document.getElementById('btn-new').onclick = () => {
    const name = prompt('New file name', 'game.prg');
    if (!name) return;
    if (!vfs.has(name)) vfs.write(name, /\.prg$/i.test(name) ? 'PROCESS Main()\nBEGIN\n    FRAME;\nEND\n' : '');
    openFile(name);
  };
  document.getElementById('btn-delete').onclick = () => {
    if (!currentPath) return;
    if (!confirm('Delete ' + currentPath + '?')) return;
    vfs.remove(currentPath);
    const next = vfs.list()[0];
    if (next) openFile(next);
    else {
      vfs.write('hello.prg', FALLBACK_HELLO);
      openFile('hello.prg');
    }
  };
  document.getElementById('btn-upload').onclick = () => document.getElementById('pick-files').click();
  document.getElementById('pick-files').onchange = async (e) => {
    for (const file of [...e.target.files]) {
      vfs.write(file.name, new Uint8Array(await file.arrayBuffer()));
    }
    e.target.value = '';
    renderExplorer();
  };
  sampleSelect.onchange = (e) => {
    if (e.target.value) loadSample(e.target.value);
    e.target.value = '';
  };
  window.addEventListener('message', onParentMessage);
  window.addEventListener('keydown', (e) => {
    if ((e.metaKey || e.ctrlKey) && e.key === 'Enter') {
      e.preventDefault();
      compileAndRun();
    }
  });
}

async function main() {
  term = new Terminal({
    convertEol: true,
    fontSize: 12,
    fontFamily: 'ui-monospace, SFMono-Regular, Menlo, Consolas, monospace',
    theme: { background: '#0b0e14', foreground: '#e8edf7', cursor: '#5b9dff' }
  });
  fit = new FitAddon();
  term.loadAddon(fit);
  term.open(document.getElementById('term'));
  fit.fit();
  new ResizeObserver(() => fit.fit()).observe(document.getElementById('term'));

  log('BennuGD Web IDE');
  log('Virtual FS → bgdc.wasm → DCB detector → bgdi → canvas');
  log('Ctrl/Cmd+Enter compiles and runs.');

  monacoApi = await loadMonaco();
  registerBennu(monacoApi);
  editor = monacoApi.editor.create(document.getElementById('editor'), {
    value: '',
    language: 'bennu',
    theme: 'vs-dark',
    automaticLayout: true,
    minimap: { enabled: false },
    fontSize: 13,
    tabSize: 4
  });

  await loadSamples();
  openFile('hello.prg');
  bindUi();

  try {
    await loadBgdc();
    pill('pill-bgdc', 'bgdc ready', 'ok');
    log('bgdc.wasm loaded (WASI p1).');
  } catch (err) {
    pill('pill-bgdc', 'bgdc missing', 'err');
    log(err.message, true);
  }

  try {
    const probe = await fetch('../bgdi.js', { method: 'HEAD' });
    if (!probe.ok) throw new Error('bgdi.js missing');
    pill('pill-bgdi', 'bgdi ready', 'ok');
    log('bgdi (Emscripten) is available.');
  } catch (err) {
    pill('pill-bgdi', 'bgdi missing', 'err');
    log('bgdi.js not found. Serve this page from dist/web-wasm32-static.', true);
  }
}

main().catch((err) => {
  console.error(err);
  if (term) log(String(err), true);
});
