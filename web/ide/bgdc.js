/* Run WASI bgdc.wasm against a snapshot of the virtual FS. */

import {
  WASI,
  WASIProcExit,
  File,
  OpenFile,
  ConsoleStdout,
  PreopenDirectory,
  Directory
} from 'https://cdn.jsdelivr.net/npm/@bjorn3/browser_wasi_shim@0.4.2/+esm';

let compiled;

function wasmUrl() {
  return new URL('../bgdc.wasm', window.location.href).href;
}

export async function loadBgdc() {
  if (compiled) return compiled;
  const url = wasmUrl();
  const res = await fetch(url);
  if (!res.ok) {
    throw new Error(
      `bgdc.wasm not found (${res.status}). Build the WASI compiler and place bgdc.wasm next to the web player.`
    );
  }
  const buf = await res.arrayBuffer();
  compiled = await WebAssembly.compile(buf);
  return compiled;
}

function inodeFromFlat(files) {
  const root = new Map();

  const dirMap = (node) => {
    if (node instanceof Directory) return node.contents;
    return node;
  };

  for (const [path, data] of Object.entries(files)) {
    const parts = path.split('/').filter(Boolean);
    if (!parts.length) continue;
    let cursor = root;
    for (let i = 0; i < parts.length - 1; i++) {
      const name = parts[i];
      let child = cursor.get(name);
      if (!(child instanceof Directory)) {
        child = new Directory([]);
        cursor.set(name, child);
      }
      cursor = dirMap(child);
    }
    cursor.set(parts[parts.length - 1], new File(new Uint8Array(data)));
  }

  return [...root.entries()];
}

function flattenInodes(contents, prefix, out) {
  if (!contents) return;
  const entries = contents instanceof Map ? contents : new Map(contents);
  for (const [name, node] of entries) {
    const path = prefix ? `${prefix}/${name}` : name;
    if (node instanceof Directory || (node && node.contents)) {
      flattenInodes(node.contents, path, out);
    } else if (node && node.data) {
      out[path] = new Uint8Array(node.data);
    }
  }
}

export async function compileWithBgdc(files, sourcePath, onLine) {
  const wasm = await loadBgdc();
  const logs = [];
  const say = (line) => {
    logs.push(line);
    if (onLine) onLine(line);
  };

  const root = new PreopenDirectory('.', inodeFromFlat(files));
  const args = ['bgdc', '-g', '-o', dcbNameFor(sourcePath), sourcePath];
  const env = ['LANG=en', 'LC_ALL=C'];
  const fds = [
    new OpenFile(new File([])),
    ConsoleStdout.lineBuffered((msg) => say(msg)),
    ConsoleStdout.lineBuffered((msg) => say(msg)),
    root
  ];

  const wasi = new WASI(args, env, fds);
  const instance = await WebAssembly.instantiate(wasm, {
    wasi_snapshot_preview1: wasi.wasiImport
  });

  let exitCode = 0;
  try {
    const ret = wasi.start(instance);
    if (typeof ret === 'number') exitCode = ret;
  } catch (err) {
    if (err instanceof WASIProcExit) exitCode = err.code;
    else throw err;
  }

  const written = {};
  flattenInodes(root.dir ? root.dir.contents : root.contents, '', written);
  return { exitCode, logs, files: written };
}

export function dcbNameFor(sourcePath) {
  const norm = sourcePath.replace(/\\/g, '/');
  const slash = norm.lastIndexOf('/');
  const dir = slash >= 0 ? norm.slice(0, slash + 1) : '';
  const base = (slash >= 0 ? norm.slice(slash + 1) : norm) || 'out.prg';
  return dir + base.replace(/\.[^.]+$/, '') + '.dcb';
}
