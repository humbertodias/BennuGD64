import { Terminal } from 'https://cdn.jsdelivr.net/npm/@xterm/xterm@5.5.0/+esm';
import { FitAddon } from 'https://cdn.jsdelivr.net/npm/@xterm/addon-fit@0.10.0/+esm';
import { VirtualFS } from './vfs.js';
import { detectDcb, formatDcb } from './dcb.js';
import { compileWithBgdc, dcbNameFor, loadBgdc } from './bgdc.js';
import { zipStore } from './zip.js';
import { Shell } from './shell.js';

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
let selectedPath = 'hello.prg';
let editorLocked = false;
let term;
let fit;
let shell;
const collapsed = new Set();

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
  if (shell) {
    shell.println(line, err);
    return;
  }
  if (!term) return;
  term.writeln((err ? '\x1b[31m' : '') + String(line).replace(/\r/g, '') + (err ? '\x1b[0m' : ''));
}

function exclusive(fn) {
  return shell ? shell.runExclusive(fn) : fn();
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

function fileBasename(path) {
  return vfs.basename(path);
}

function targetDir() {
  const sel = selectedPath || currentPath;
  if (sel && vfs.isDir(sel)) return vfs.normalize(sel);
  if (sel) return vfs.parent(sel);
  return '';
}

function saveBlob(name, data) {
  const blob = new Blob([data], { type: 'application/octet-stream' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = name;
  a.click();
  URL.revokeObjectURL(url);
}

function downloadPath(path) {
  if (path === currentPath || (vfs.isDir(path) && currentPath.startsWith(path + '/'))) {
    flushEditor();
  }
  if (!vfs.isDir(path)) {
    const data = vfs.read(path);
    if (!data) return;
    saveBlob(fileBasename(path), data);
    return;
  }
  const { files, dirs } = vfs.subtree(path);
  const parent = vfs.parent(path);
  const strip = parent ? parent + '/' : '';
  const rel = (full) => (strip && full.startsWith(strip) ? full.slice(strip.length) : full);
  const entries = [];
  const seen = new Set();
  for (const dir of dirs.sort()) {
    const name = rel(dir).replace(/\/?$/, '/') ;
    if (!name || seen.has(name)) continue;
    seen.add(name);
    entries.push({ name, data: new Uint8Array() });
  }
  for (const [file, data] of Object.entries(files)) {
    const name = rel(file);
    if (!name || seen.has(name)) continue;
    seen.add(name);
    entries.push({ name, data });
  }
  if (!entries.length) {
    entries.push({ name: fileBasename(path) + '/', data: new Uint8Array() });
  }
  saveBlob(fileBasename(path) + '.zip', zipStore(entries));
}

function expandTo(path) {
  let dir = vfs.parent(path);
  while (dir) {
    collapsed.delete(dir);
    dir = vfs.parent(dir);
  }
}

function renderExplorer() {
  filesEl.replaceChildren();
  const highlight = selectedPath || currentPath;
  const walk = (node, depth) => {
    for (const child of node.children || []) {
      filesEl.appendChild(rowFor(child, depth, highlight));
      if (child.kind === 'dir' && !collapsed.has(child.path)) walk(child, depth + 1);
    }
  };
  walk(vfs.tree(), 0);
}

function rowFor(node, depth, highlight) {
  const row = document.createElement('div');
  row.className = 'file ' + (node.kind === 'dir' ? 'dir' : extOf(node.path));
  row.dataset.path = node.path;
  row.dataset.kind = node.kind;
  row.draggable = true;
  if (node.path === highlight) row.classList.add('active');
  row.style.paddingLeft = 4 + depth * 12 + 'px';
  row.addEventListener('dragstart', (e) => {
    if (e.target.closest('.file-dl, .file-twist')) {
      e.preventDefault();
      return;
    }
    e.dataTransfer.setData('application/x-bennugd-path', node.path);
    e.dataTransfer.setData('text/plain', node.path);
    e.dataTransfer.effectAllowed = 'move';
  });

  const twist = document.createElement('button');
  twist.type = 'button';
  twist.className = 'file-twist';
  if (node.kind === 'dir') {
    twist.textContent = collapsed.has(node.path) ? '▸' : '▾';
    twist.title = collapsed.has(node.path) ? 'Expand' : 'Collapse';
    twist.onclick = (e) => {
      e.stopPropagation();
      if (collapsed.has(node.path)) collapsed.delete(node.path);
      else collapsed.add(node.path);
      renderExplorer();
    };
  } else {
    twist.textContent = '';
    twist.tabIndex = -1;
    twist.disabled = true;
  }

  const btn = document.createElement('button');
  btn.type = 'button';
  btn.className = 'file-open';
  btn.textContent = node.name;
  btn.title = node.path;
  btn.onclick = () => {
    selectedPath = node.path;
    if (node.kind === 'dir') renderExplorer();
    else openFile(node.path);
  };

  row.append(twist, btn);
  const ext = document.createElement('span');
  ext.className = 'ext';
  ext.textContent = node.kind === 'dir' ? 'dir' : (extOf(node.path) || 'file');
  const dl = document.createElement('button');
  dl.type = 'button';
  dl.className = 'file-dl';
  dl.title = 'Download ' + node.path + (node.kind === 'dir' ? ' as zip' : '');
  dl.setAttribute('aria-label', 'Download ' + node.path);
  dl.textContent = '↓';
  dl.onclick = () => downloadPath(node.path);
  dl.draggable = false;
  row.append(ext, dl);
  return row;
}

function skipDropName(name) {
  const base = String(name || '').split('/').pop();
  return !base || base === '.DS_Store' || base === 'Thumbs.db' || base.startsWith('._');
}

function readEntries(reader) {
  return new Promise((resolve, reject) => {
    const all = [];
    const next = () => {
      reader.readEntries((batch) => {
        if (!batch.length) return resolve(all);
        all.push(...batch);
        next();
      }, reject);
    };
    next();
  });
}

async function walkEntry(entry, prefix) {
  const path = prefix ? prefix + entry.name : entry.name;
  if (skipDropName(path)) return [];
  if (entry.isFile) {
    const file = await new Promise((res, rej) => entry.file(res, rej));
    return [{ path, file }];
  }
  const kids = await readEntries(entry.createReader());
  if (!kids.length) return [{ path, dir: true }];
  const out = [];
  for (const kid of kids) out.push(...await walkEntry(kid, path + '/'));
  return out;
}

async function itemsFromDataTransfer(dt) {
  const items = [...(dt.items || [])];
  if (items.some((it) => it.webkitGetAsEntry)) {
    const collected = [];
    for (const it of items) {
      const entry = it.webkitGetAsEntry && it.webkitGetAsEntry();
      if (entry) collected.push(...await walkEntry(entry, ''));
    }
    if (collected.length) return collected;
  }
  return [...(dt.files || [])].map((file) => ({
    path: file.webkitRelativePath || file.name,
    file
  }));
}

function isInternalDrag(dt) {
  return [...(dt.types || [])].includes('application/x-bennugd-path');
}

function dropDestFromEvent(e) {
  const row = e.target.closest && e.target.closest('#files .file');
  if (!row) return '';
  const path = row.dataset.path || '';
  if (row.dataset.kind === 'dir') return path;
  return vfs.parent(path);
}

function clearDropMarks() {
  filesEl.classList.remove('drop');
  filesEl.querySelectorAll('.file.drop').forEach((el) => el.classList.remove('drop'));
}

function markDropDest(destPath) {
  clearDropMarks();
  if (!destPath) {
    filesEl.classList.add('drop');
    return;
  }
  const row = [...filesEl.querySelectorAll('.file[data-path]')].find((el) => el.dataset.path === destPath);
  if (row) row.classList.add('drop');
  else filesEl.classList.add('drop');
}

function remapOpenPath(from, to) {
  const prefix = from + '/';
  const fix = (path) => {
    if (!path) return path;
    if (path === from) return to;
    if (path.startsWith(prefix)) return to + path.slice(from.length);
    return path;
  };
  currentPath = fix(currentPath);
  selectedPath = fix(selectedPath);
  if (editorTitle && currentPath) editorTitle.textContent = currentPath;
}

async function importDropped(pairs, destDir) {
  let last = '';
  for (const item of pairs) {
    if (skipDropName(item.path)) continue;
    const dest = destDir ? vfs.join(destDir, item.path) : vfs.normalize(item.path);
    try {
      if (item.dir) vfs.mkdir(dest);
      else vfs.write(dest, new Uint8Array(await item.file.arrayBuffer()));
      expandTo(dest);
      last = dest;
    } catch (err) {
      log((dest || item.path) + ': ' + err.message, true);
    }
  }
  if (last) selectedPath = last;
  renderExplorer();
}

function bindExplorerDrop() {
  const pane = document.getElementById('explorer-pane');

  pane.addEventListener('dragover', (e) => {
    if (!e.dataTransfer) return;
    e.preventDefault();
    e.dataTransfer.dropEffect = isInternalDrag(e.dataTransfer) ? 'move' : 'copy';
    markDropDest(dropDestFromEvent(e));
  });
  pane.addEventListener('dragleave', (e) => {
    if (!pane.contains(e.relatedTarget)) clearDropMarks();
  });
  pane.addEventListener('drop', async (e) => {
    e.preventDefault();
    clearDropMarks();
    const dest = dropDestFromEvent(e);
    const internal = e.dataTransfer.getData('application/x-bennugd-path')
      || (isInternalDrag(e.dataTransfer) ? e.dataTransfer.getData('text/plain') : '');
    if (internal) {
      try {
        if (currentPath === internal || currentPath.startsWith(internal + '/')) flushEditor();
        const to = vfs.move(internal, dest);
        remapOpenPath(internal, to);
        expandTo(to);
        selectedPath = to;
        renderExplorer();
        log('moved ' + internal + ' → ' + to);
      } catch (err) {
        log(err.message, true);
      }
      return;
    }
    try {
      const pairs = await itemsFromDataTransfer(e.dataTransfer);
      if (!pairs.length) return log('No files dropped.', true);
      await importDropped(pairs, dest);
      log('imported ' + pairs.length + ' item(s)' + (dest ? ' into ' + dest : ''));
    } catch (err) {
      log(String(err), true);
    }
  });
}

function languageFor(path) {
  if (/\.prg$/i.test(path)) return 'bennu';
  if (/\.(json)$/i.test(path)) return 'json';
  return 'plaintext';
}

function openFile(path) {
  if (vfs.isDir(path)) {
    selectedPath = path;
    renderExplorer();
    return;
  }
  if (path !== currentPath) flushEditor();
  currentPath = path;
  selectedPath = path;
  expandTo(path);
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

async function compileInner(srcPath) {
  flushEditor();
  const src = srcPath || sourcePath();
  if (!/\.prg$/i.test(src)) {
    log('Open a .prg file to compile.', true);
    return null;
  }
  if (!vfs.has(src)) {
    log(src + ' not found', true);
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

function compile(srcPath) {
  return exclusive(() => compileInner(srcPath));
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

async function runTarget(path) {
  if (path && /\.dcb$/i.test(path)) {
    flushEditor();
    const bytes = vfs.read(path);
    if (!bytes) {
      log(path + ' not found', true);
      return;
    }
    const info = detectDcb(bytes);
    setDcbPill(info);
    log(formatDcb(info), !info.ok);
    if (!info.ok || !info.runtimeOk) {
      log('Not running: DCB missing or incompatible.', true);
      return;
    }
    log(`$ bgdi ${path}`);
    runGame(path);
    return;
  }
  const compiled = await compileInner(path);
  if (!compiled || !compiled.ok) {
    log('Not running: DCB missing or incompatible.', true);
    return;
  }
  log(`$ bgdi ${compiled.path}`);
  runGame(compiled.path);
}

function compileAndRun() {
  return exclusive(() => runTarget());
}

function runFromShell(path) {
  return exclusive(() => runTarget(path));
}

function mkdirPath(path) {
  vfs.mkdir(path);
  selectedPath = vfs.normalize(path);
  expandTo(selectedPath);
  renderExplorer();
}

function writePath(path, data) {
  vfs.write(path, data);
  expandTo(path);
  renderExplorer();
}

function removePath(path, recursive) {
  if (!path) throw new Error('empty path');
  if (!vfs.has(path)) throw new Error(path + ': no such file or directory');
  const folder = vfs.isDir(path) && !vfs.files.has(path);
  if (folder && !recursive) throw new Error(path + ' is a directory (use rm -r)');
  const lostOpen = currentPath === path || (folder && currentPath.startsWith(path + '/'));
  if (lostOpen) flushEditor();
  vfs.remove(path);
  if (shell && (shell.cwd === path || (folder && shell.cwd.startsWith(path + '/')))) {
    shell.cwd = vfs.parent(path);
  }
  selectedPath = '';
  if (lostOpen || !vfs.has(currentPath)) {
    const next = vfs.list()[0];
    if (next) openFile(next);
    else {
      vfs.write('hello.prg', FALLBACK_HELLO);
      openFile('hello.prg');
    }
  } else {
    renderExplorer();
  }
}

function movePath(src, dest) {
  if (currentPath === src || currentPath.startsWith(src + '/')) flushEditor();
  const to = (vfs.isDir(dest) && dest !== src) ? vfs.move(src, dest) : vfs.rename(src, dest);
  remapOpenPath(src, to);
  if (shell) {
    if (shell.cwd === src) shell.cwd = to;
    else if (shell.cwd.startsWith(src + '/')) shell.cwd = to + shell.cwd.slice(src.length);
  }
  expandTo(to);
  selectedPath = to;
  renderExplorer();
  log('moved ' + src + ' → ' + to);
}

function selectDir(path) {
  selectedPath = path || '';
  if (path) expandTo(path);
  renderExplorer();
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

function registerIdeCommands(monaco, ed) {
  ed.addAction({
    id: 'bennu.compile',
    label: 'Bennu: Compile',
    keybindings: [monaco.KeyMod.CtrlCmd | monaco.KeyMod.Shift | monaco.KeyCode.KeyB],
    contextMenuGroupId: 'bennu',
    contextMenuOrder: 1,
    run: () => compile()
  });
  ed.addAction({
    id: 'bennu.run',
    label: 'Bennu: Run',
    keybindings: [monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter],
    contextMenuGroupId: 'bennu',
    contextMenuOrder: 2,
    run: () => compileAndRun()
  });
}

const EXPLORER_WIDTH_KEY = 'bennugd-ide-explorer-width';

function applyExplorerWidth(px) {
  const work = document.getElementById('work');
  const min = 140;
  const max = Math.max(min, Math.min(480, Math.floor(work.clientWidth * 0.5)));
  const width = Math.max(min, Math.min(max, Math.round(px)));
  work.style.setProperty('--explorer-width', width + 'px');
  return width;
}

function bindExplorerResize() {
  const split = document.getElementById('explorer-split');
  const work = document.getElementById('work');
  const saved = parseInt(localStorage.getItem(EXPLORER_WIDTH_KEY), 10);
  if (saved) applyExplorerWidth(saved);

  split.addEventListener('pointerdown', (e) => {
    e.preventDefault();
    split.classList.add('dragging');
    document.body.classList.add('resizing', 'resizing-col');
    split.setPointerCapture(e.pointerId);
    const onMove = (ev) => {
      const width = applyExplorerWidth(ev.clientX - work.getBoundingClientRect().left);
      localStorage.setItem(EXPLORER_WIDTH_KEY, String(width));
    };
    const onUp = () => {
      split.classList.remove('dragging');
      document.body.classList.remove('resizing', 'resizing-col');
      split.removeEventListener('pointermove', onMove);
      split.removeEventListener('pointerup', onUp);
    };
    split.addEventListener('pointermove', onMove);
    split.addEventListener('pointerup', onUp);
  });
}

const TERM_HEIGHT_KEY = 'bennugd-ide-term-height';

function applyTermHeight(px) {
  const min = 80;
  const top = document.getElementById('top');
  const reserved = (top ? top.getBoundingClientRect().height : 48) + 6 + 140;
  const max = Math.max(min, Math.floor(window.innerHeight - reserved));
  const height = Math.max(min, Math.min(max, Math.round(px)));
  document.body.style.setProperty('--term-height', height + 'px');
  if (fit) fit.fit();
  return height;
}

function bindTermResize() {
  const split = document.getElementById('term-split');
  const saved = parseInt(localStorage.getItem(TERM_HEIGHT_KEY), 10);
  if (saved) applyTermHeight(saved);

  split.addEventListener('pointerdown', (e) => {
    e.preventDefault();
    split.classList.add('dragging');
    document.body.classList.add('resizing', 'resizing-row');
    try { split.setPointerCapture(e.pointerId); } catch (err) { /* synthetic events */ }
    const onMove = (ev) => {
      const height = applyTermHeight(window.innerHeight - ev.clientY);
      localStorage.setItem(TERM_HEIGHT_KEY, String(height));
    };
    const onUp = () => {
      split.classList.remove('dragging');
      document.body.classList.remove('resizing', 'resizing-row');
      window.removeEventListener('pointermove', onMove);
      window.removeEventListener('pointerup', onUp);
    };
    window.addEventListener('pointermove', onMove);
    window.addEventListener('pointerup', onUp);
    onMove(e);
  });

  window.addEventListener('resize', () => {
    const current = parseInt(getComputedStyle(document.body).getPropertyValue('--term-height'), 10)
      || parseInt(localStorage.getItem(TERM_HEIGHT_KEY), 10)
      || 180;
    applyTermHeight(current);
  });
}

function bindUi() {
  document.getElementById('btn-compile').onclick = () => compile();
  document.getElementById('btn-run').onclick = () => compileAndRun();
  document.getElementById('btn-stop').onclick = () => stopGame();
  document.getElementById('btn-new').onclick = () => {
    let name;
    try {
      name = prompt('New file path', vfs.join(targetDir(), 'game.prg'));
    } catch (err) {
      log(err.message, true);
      return;
    }
    if (!name) return;
    const path = vfs.normalize(name);
    if (!path) return;
    if (vfs.isDir(path)) return log(path + ' is a directory', true);
    if (!vfs.has(path)) {
      try {
        vfs.write(path, /\.prg$/i.test(path) ? 'PROCESS Main()\nBEGIN\n    FRAME;\nEND\n' : '');
      } catch (err) {
        log(err.message, true);
        return;
      }
    }
    expandTo(path);
    openFile(path);
  };
  document.getElementById('btn-mkdir').onclick = () => {
    let name;
    try {
      name = prompt('New folder path', vfs.join(targetDir(), 'lib'));
    } catch (err) {
      log(err.message, true);
      return;
    }
    if (!name) return;
    try {
      vfs.mkdir(name);
    } catch (err) {
      log(err.message, true);
      return;
    }
    selectedPath = vfs.normalize(name);
    expandTo(selectedPath);
    renderExplorer();
  };
  document.getElementById('btn-delete').onclick = () => {
    const victim = selectedPath || currentPath;
    if (!victim) return;
    const folder = vfs.isDir(victim);
    if (!confirm(folder ? 'Delete folder ' + victim + ' and its files?' : 'Delete ' + victim + '?')) return;
    try {
      removePath(victim, folder);
    } catch (err) {
      log(err.message, true);
    }
  };
  document.getElementById('btn-upload').onclick = (e) => {
    document.getElementById(e.shiftKey ? 'pick-folder' : 'pick-files').click();
  };
  document.getElementById('btn-upload').title = 'Upload files. Shift+click to upload a folder.';
  const ingestUploads = async (list) => {
    for (const file of [...list]) {
      const key = vfs.normalize(file.webkitRelativePath || file.name);
      try {
        vfs.write(key, new Uint8Array(await file.arrayBuffer()));
        expandTo(key);
      } catch (err) {
        log((key || file.name) + ': ' + err.message, true);
      }
    }
    renderExplorer();
  };
  document.getElementById('pick-files').onchange = async (e) => {
    await ingestUploads(e.target.files);
    e.target.value = '';
  };
  document.getElementById('pick-folder').onchange = async (e) => {
    await ingestUploads(e.target.files);
    e.target.value = '';
  };
  sampleSelect.onchange = (e) => {
    if (e.target.value) loadSample(e.target.value);
    e.target.value = '';
  };
  window.addEventListener('message', onParentMessage);
  bindExplorerResize();
  bindTermResize();
  bindExplorerDrop();
}

async function main() {
  term = new Terminal({
    convertEol: true,
    cursorBlink: true,
    fontSize: 12,
    fontFamily: 'ui-monospace, SFMono-Regular, Menlo, Consolas, monospace',
    theme: { background: '#0b0e14', foreground: '#e8edf7', cursor: '#5b9dff' }
  });
  fit = new FitAddon();
  term.loadAddon(fit);
  term.open(document.getElementById('term'));
  fit.fit();
  new ResizeObserver(() => fit.fit()).observe(document.getElementById('term'));
  document.getElementById('term-wrap').addEventListener('mousedown', () => term.focus());

  shell = new Shell(term, {
    vfs,
    log,
    compile,
    run: runFromShell,
    stop: stopGame,
    open: openFile,
    download: downloadPath,
    mkdir: mkdirPath,
    write: writePath,
    remove: removePath,
    move: movePath,
    selectDir,
    flush: flushEditor,
    refresh: renderExplorer,
    currentPath: () => currentPath,
    detectDcb,
    formatDcb
  });

  log('BennuGD Web IDE');
  log('Virtual FS → bgdc.wasm → DCB detector → bgdi → canvas');
  log('Ctrl/Cmd+Shift+B compiles. Ctrl/Cmd+Enter runs. F1 opens the command palette.');
  log('Click the terminal and type help for commands.');

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
  registerIdeCommands(monacoApi, editor);

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

  shell.start();
}

main().catch((err) => {
  console.error(err);
  if (term) log(String(err), true);
});
