/* Monaco language, snippets, and IntelliSense for BennuGD. */

import { CATALOG } from './bennu-catalog.js';

const KEYWORDS = [
  'program', 'process', 'function', 'begin', 'end', 'frame', 'loop',
  'from', 'to', 'step', 'while', 'repeat', 'until', 'for', 'if', 'else',
  'elseif', 'elif', 'elsif', 'switch', 'case', 'default', 'break', 'continue',
  'return', 'global', 'local', 'public', 'private', 'const', 'type', 'struct',
  'pointer', 'import', 'include', 'onexit', 'onerror', 'clone', 'signal',
  'declare', 'debug', 'yield', 'sizeof', 'offset', 'int', 'float', 'string',
  'byte', 'word', 'dword', 'short', 'char', 'signed', 'unsigned',
  'and', 'or', 'xor', 'not', 'mod'
];

const CORE_CONSTS = [
  'TRUE', 'FALSE', 'NULL',
  'STATUS_DEAD', 'STATUS_KILLED', 'STATUS_RUNNING', 'STATUS_SLEEPING',
  'STATUS_FROZEN', 'STATUS_WAITING',
  'OS_WIN32', 'OS_LINUX', 'OS_MACOS', 'OS_WII', 'OS_ANDROID', 'OS_EMSCRIPTEN',
  'OS_SWITCH', 'OS_PSP', 'OS_PANDORA', 'MIN_INT', 'MAX_INT'
];

const CORE_LOCALS = ['id', 'father', 'son', 'smallbro', 'bigbro', 'priority'];
const RENDER_LOCALS = [
  'x', 'y', 'z', 'file', 'graph', 'size', 'angle', 'flags', 'alpha',
  'region', 'ctype', 'cnumber', 'resolution', 'size_x', 'size_y', 'blendop', 'palette'
];
const CORE_GLOBALS = ['argc', 'argv', 'os_id', 'fps', 'frame_time', 'timer', 'ascii', 'scan_code', 'shift_status'];
const MOUSE_FIELDS = [
  'x', 'y', 'z', 'file', 'graph', 'angle', 'size', 'flags', 'region',
  'left', 'middle', 'right', 'wheelup', 'wheeldown'
];
const RESERVED_FIELDS = ['process_type', 'frame_percent', 'status', 'saved_status', 'saved_priority'];

const NAMED_PARAMS = {
  set_mode: ['width', 'height', 'depth', 'flags'],
  set_fps: ['fps', 'frameskip'],
  write: ['fontId', 'x', 'y', 'align', 'text'],
  write_int: ['fontId', 'x', 'y', 'align', 'valuePtr'],
  key: ['scancode'],
  rand: ['min', 'max'],
  load_map: ['filename'],
  load_fpg: ['filename'],
  load_png: ['filename'],
  load_fnt: ['filename'],
  put: ['file', 'graph', 'x', 'y'],
  xput: ['file', 'graph', 'x', 'y', 'angle', 'size', 'flags', 'region'],
  signal: ['processId', 'sig'],
  collision: ['processId'],
  get_id: ['processType'],
  exists: ['processId'],
  draw_line: ['x0', 'y0', 'x1', 'y1'],
  draw_rect: ['x', 'y', 'width', 'height'],
  draw_box: ['x', 'y', 'width', 'height'],
  draw_circle: ['x', 'y', 'radius'],
  draw_fcircle: ['x', 'y', 'radius'],
  put_pixel: ['x', 'y', 'color'],
  say: ['text'],
  getenv: ['name'],
  fopen: ['filename', 'mode'],
  fade: ['r', 'g', 'b', 'speed'],
  play_wav: ['id', 'loops'],
  play_song: ['id', 'loops'],
  set_title: ['title'],
  get_angle: ['processId'],
  get_dist: ['processId'],
  advance: ['distance'],
  xadvance: ['angle', 'distance'],
  strlen: ['text'],
  substr: ['text', 'start', 'length'],
  find: ['text', 'needle'],
  delete_text: ['textId'],
  map_load: ['filename'],
  fpg_load: ['filename']
};

const SIG_TYPE = { I: 'int', S: 'string', F: 'float', P: 'pointer', B: 'byte', W: 'word' };

const IMPORTABLE = Object.keys(CATALOG).filter((n) => n.startsWith('mod_')).sort();

function parseSig(sig) {
  const parts = [];
  for (let i = 0; i < sig.length; i++) {
    if (sig.startsWith('V++', i)) {
      parts.push('var');
      i += 2;
      continue;
    }
    parts.push(SIG_TYPE[sig[i]] || sig[i].toLowerCase());
  }
  return parts;
}

function paramNames(fn, sig) {
  const types = parseSig(sig);
  const named = NAMED_PARAMS[fn.toLowerCase()];
  return types.map((t, i) => (named && named[i]) ? named[i] : (types.length === 1 ? t : t + (i + 1)));
}

function labelFor(fn, sig) {
  const params = paramNames(fn, sig);
  return fn + '(' + params.join(', ') + ')';
}

function snippetFor(fn, sig) {
  const params = paramNames(fn, sig);
  if (!params.length) return fn + '()';
  return fn + '(' + params.map((p, i) => '${' + (i + 1) + ':' + p + '}').join(', ') + ')';
}

function preferredSig(sigs) {
  if (!sigs || !sigs.length) return '';
  return [...sigs].sort((a, b) => {
    const da = Math.abs(parseSig(a).length - 3);
    const db = Math.abs(parseSig(b).length - 3);
    return da - db || a.length - b.length;
  })[0];
}

function resolveModules(names) {
  const out = new Set();
  const visit = (name) => {
    const key = String(name || '').replace(/\\/g, '/').split('/').pop().replace(/\.(dll|so|dylib)$/i, '');
    if (!key || out.has(key) || !CATALOG[key]) return;
    out.add(key);
    (CATALOG[key].deps || []).forEach(visit);
  };
  names.forEach(visit);
  return out;
}

function parseSource(text) {
  const stripped = text.replace(/\/\*[\s\S]*?\*\//g, '').replace(/\/\/.*$/gm, '');
  const imports = [...stripped.matchAll(/^\s*(?:#\s*)?(?:import|include)\s+"([^"]+)"/gmi)].map((m) => m[1]);
  const procs = [...stripped.matchAll(/\b(?:process|function)\s+([A-Za-z_][\w]*)/gi)].map((m) => m[1]);
  const vars = [...stripped.matchAll(/\b(?:int|float|string|byte|word|dword|short|char)\s+([A-Za-z_][\w]*)/gi)].map((m) => m[1]);
  return { imports, procs, vars };
}

function wordBefore(model, position) {
  const line = model.getLineContent(position.lineNumber);
  const until = line.slice(0, position.column - 1);
  const m = until.match(/[A-Za-z_][\w]*$/);
  return { line, until, word: m ? m[0] : '' };
}

function collectApi(modNames) {
  const funcs = new Map();
  const consts = [];
  for (const name of resolveModules(modNames)) {
    const mod = CATALOG[name];
    if (!mod) continue;
    for (const [fn, sigs] of Object.entries(mod.funcs || {})) {
      const key = fn.toLowerCase();
      if (!funcs.has(key)) funcs.set(key, { name: fn, sigs, module: name });
    }
    for (const c of mod.consts || []) consts.push({ name: c, module: name });
  }
  return { funcs, consts };
}

export function registerBennu(monaco, hooks = {}) {
  monaco.languages.register({ id: 'bennu' });
  monaco.languages.setLanguageConfiguration('bennu', {
    comments: { lineComment: '//', blockComment: ['/*', '*/'] },
    brackets: [['(', ')'], ['[', ']']],
    autoClosingPairs: [
      { open: '(', close: ')' },
      { open: '[', close: ']' },
      { open: '"', close: '"' },
      { open: "'", close: "'" }
    ],
    surroundingPairs: [
      { open: '(', close: ')' },
      { open: '[', close: ']' },
      { open: '"', close: '"' },
      { open: "'", close: "'" }
    ],
    wordPattern: /(-?\d*\.\d\w*)|([^\`\~\!\@\#\%\^\&\*\(\)\-\=\+\[\{\]\}\\\|\;\:\'\"\,\.\<\>\/\?\s]+)/g
  });
  monaco.languages.setMonarchTokensProvider('bennu', {
    ignoreCase: true,
    keywords: KEYWORDS,
    tokenizer: {
      root: [
        [/\/\/.*$/, 'comment'],
        [/\/\*/, 'comment', '@comment'],
        [/"([^"\\]|\\.)*"/, 'string'],
        [/'([^'\\]|\\.)*'/, 'string'],
        [/\b\d+\b/, 'number'],
        [/#\w+/, 'keyword'],
        [/[a-zA-Z_][\w]*/, {
          cases: {
            '@keywords': 'keyword',
            '@default': 'identifier'
          }
        }]
      ],
      comment: [
        [/[^/*]+/, 'comment'],
        [/\*\//, 'comment', '@pop'],
        [/[/*]/, 'comment']
      ]
    }
  });

  const K = monaco.languages.CompletionItemKind;
  const InsertSnippet = monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet;

  const snippets = [
    { label: 'process', insert: 'PROCESS ${1:Name}()\nBEGIN\n    $0\n    FRAME;\nEND', doc: 'Process definition' },
    { label: 'function', insert: 'FUNCTION ${1:Name}()\nBEGIN\n    $0\nEND', doc: 'Function definition' },
    { label: 'loop', insert: 'LOOP\n    $0\n    FRAME;\nEND', doc: 'Main loop' },
    { label: 'if', insert: 'IF (${1:cond})\n    $0\nEND', doc: 'If block' },
    { label: 'else', insert: 'ELSE\n    $0', doc: 'Else branch' },
    { label: 'elseif', insert: 'ELSEIF (${1:cond})\n    $0', doc: 'Elseif branch' },
    { label: 'while', insert: 'WHILE (${1:cond})\n    $0\nEND', doc: 'While loop' },
    { label: 'repeat', insert: 'REPEAT\n    $0\nUNTIL (${1:cond});', doc: 'Repeat/until loop' },
    { label: 'from', insert: 'FROM ${1:i} = ${2:0} TO ${3:10}\n    $0\nEND', doc: 'Counted loop' },
    { label: 'switch', insert: 'SWITCH (${1:value})\n    CASE ${2:0}:\n        $0\n    DEFAULT:\nEND', doc: 'Switch block' },
    { label: 'import', insert: 'import "${1:mod_video}"', doc: 'Import a module' },
    { label: 'include', insert: 'include "${1:file.inc}"', doc: 'Include a source file' },
    { label: 'private', insert: 'PRIVATE\n    ${1:int n};\n', doc: 'Private variables' },
    { label: 'global', insert: 'GLOBAL\n    ${1:int n};\n', doc: 'Global variables' }
  ];

  monaco.languages.registerCompletionItemProvider('bennu', {
    triggerCharacters: ['.', '"', '(', '_'],
    provideCompletionItems(model, position) {
      const { line, until, word } = wordBefore(model, position);
      const range = {
        startLineNumber: position.lineNumber,
        startColumn: position.column - word.length,
        endLineNumber: position.lineNumber,
        endColumn: position.column
      };
      const prefix = word.toLowerCase();
      const match = (text) => !prefix || String(text).toLowerCase().startsWith(prefix);
      const items = [];

      const importOpen = until.match(/^\s*(?:#\s*)?(?:import|include)\s+"([^"]*)$/i);
      if (importOpen) {
        const typed = importOpen[1].toLowerCase();
        const isInclude = /include/i.test(until);
        const names = isInclude
          ? (hooks.listPaths ? hooks.listPaths() : []).filter((p) => /\.(prg|inc|h)$/i.test(p))
          : IMPORTABLE;
        for (const name of names) {
          if (typed && !name.toLowerCase().startsWith(typed) && !name.toLowerCase().includes(typed)) continue;
          items.push({
            label: name,
            kind: isInclude ? K.File : K.Module,
            insertText: name,
            range: {
              startLineNumber: position.lineNumber,
              startColumn: position.column - typed.length,
              endLineNumber: position.lineNumber,
              endColumn: position.column
            },
            detail: isInclude ? 'project file' : 'module',
            sortText: '0' + name
          });
        }
        return { suggestions: items };
      }

      const member = until.match(/([A-Za-z_][\w]*)\.\w*$/);
      if (member) {
        const obj = member[1].toLowerCase();
        const fields = obj === 'mouse' ? MOUSE_FIELDS
          : obj === 'reserved' ? RESERVED_FIELDS
            : [];
        for (const f of fields) {
          if (!match(f)) continue;
          items.push({
            label: f,
            kind: K.Field,
            insertText: f,
            range,
            detail: obj + '.' + f,
            sortText: '0' + f
          });
        }
        return { suggestions: items };
      }

      const parsed = parseSource(model.getValue());
      if (hooks.listPaths && hooks.readText) {
        for (const p of hooks.listPaths()) {
          if (!/\.(prg|inc)$/i.test(p)) continue;
          try {
            const extra = parseSource(hooks.readText(p));
            parsed.procs.push(...extra.procs);
          } catch (err) { /* ignore unread files */ }
        }
      }
      const mods = resolveModules(parsed.imports);
      const api = collectApi(parsed.imports);
      const extraVars = [];
      if (mods.has('libmouse')) extraVars.push('mouse');
      if (mods.has('mod_timers')) extraVars.push('timer');
      if (mods.has('librender')) extraVars.push('fps', 'frame_time');
      if (mods.has('libkey')) extraVars.push('ascii', 'scan_code', 'shift_status');

      const inKeyCall = /\bkey\s*\(\s*[\w]*$/i.test(until);

      for (const sn of snippets) {
        if (!match(sn.label)) continue;
        items.push({
          label: sn.label,
          kind: K.Snippet,
          insertText: sn.insert,
          insertTextRules: InsertSnippet,
          documentation: sn.doc,
          detail: 'snippet',
          range,
          sortText: '1' + sn.label
        });
      }
      for (const kw of KEYWORDS) {
        if (!match(kw)) continue;
        items.push({
          label: kw,
          kind: K.Keyword,
          insertText: kw,
          range,
          sortText: '2' + kw
        });
      }
      const locals = [...new Set([...CORE_LOCALS, ...RENDER_LOCALS, ...CORE_GLOBALS, ...extraVars, ...parsed.vars, ...parsed.procs])];
      for (const name of locals) {
        if (!match(name)) continue;
        items.push({
          label: name,
          kind: parsed.procs.some((p) => p.toLowerCase() === name.toLowerCase()) ? K.Function : K.Variable,
          insertText: name,
          range,
          detail: 'local',
          sortText: '3' + name
        });
      }
      for (const fn of api.funcs.values()) {
        const label = fn.name.toLowerCase();
        if (!match(label) && !match(fn.name)) continue;
        const sig = preferredSig(fn.sigs);
        items.push({
          label,
          kind: K.Function,
          insertText: snippetFor(label, sig),
          insertTextRules: InsertSnippet,
          detail: fn.module + ' · ' + fn.sigs.map((s) => labelFor(label, s)).join(' | '),
          documentation: fn.sigs.map((s) => labelFor(label, s)).join('\n'),
          range,
          sortText: '4' + label
        });
      }
      for (const c of [...CORE_CONSTS.map((name) => ({ name, module: 'core' })), ...api.consts]) {
        if (!match(c.name) && !match(c.name.replace(/^_/, ''))) continue;
        items.push({
          label: c.name,
          kind: K.EnumMember,
          insertText: c.name,
          detail: c.module,
          range,
          sortText: (inKeyCall && c.name.startsWith('_') ? '0' : '5') + c.name
        });
      }
      return { suggestions: items };
    }
  });

  monaco.languages.registerHoverProvider('bennu', {
    provideHover(model, position) {
      const word = model.getWordAtPosition(position);
      if (!word) return null;
      const parsed = parseSource(model.getValue());
      const api = collectApi(parsed.imports);
      const key = word.word.toLowerCase();
      const fn = api.funcs.get(key);
      if (fn) {
        const sigs = fn.sigs.map((s) => labelFor(fn.name.toLowerCase(), s));
        return {
          range: new monaco.Range(position.lineNumber, word.startColumn, position.lineNumber, word.endColumn),
          contents: [
            { value: '**' + fn.name.toLowerCase() + '** — `' + fn.module + '`' },
            { value: '```bennu\n' + sigs.join('\n') + '\n```' }
          ]
        };
      }
      const cst = api.consts.find((c) => c.name.toLowerCase() === key) ||
        (CORE_CONSTS.includes(word.word.toUpperCase()) ? { name: word.word.toUpperCase(), module: 'core' } : null);
      if (cst) {
        return {
          contents: [{ value: '**' + cst.name + '** — `' + cst.module + '` constant' }]
        };
      }
      if (KEYWORDS.includes(key)) {
        return { contents: [{ value: '**' + key + '** — language keyword' }] };
      }
      return null;
    }
  });

  monaco.languages.registerSignatureHelpProvider('bennu', {
    signatureHelpTriggerCharacters: ['(', ','],
    provideSignatureHelp(model, position) {
      const textUntil = model.getValueInRange({
        startLineNumber: 1,
        startColumn: 1,
        endLineNumber: position.lineNumber,
        endColumn: position.column
      });
      const call = textUntil.match(/([A-Za-z_][\w]*)\s*\(([^()]*)$/);
      if (!call) return null;
      const parsed = parseSource(model.getValue());
      const api = collectApi(parsed.imports);
      const fn = api.funcs.get(call[1].toLowerCase());
      if (!fn) return null;
      const argIndex = call[2] ? call[2].split(',').length - 1 : 0;
      const signatures = fn.sigs.map((sig) => {
        const params = paramNames(fn.name.toLowerCase(), sig);
        return {
          label: labelFor(fn.name.toLowerCase(), sig),
          parameters: params.map((p) => ({ label: p }))
        };
      });
      let activeSignature = 0;
      signatures.forEach((s, i) => {
        if (s.parameters.length > argIndex && signatures[activeSignature].parameters.length < argIndex + 1) {
          activeSignature = i;
        }
      });
      return {
        dispose() {},
        value: {
          signatures,
          activeSignature,
          activeParameter: Math.max(0, argIndex)
        }
      };
    }
  });
}
