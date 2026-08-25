/* Line editor + command dispatch for the IDE xterm. */

const COMMANDS = [
  'help', 'clear', 'pwd', 'cd', 'ls', 'cat', 'open', 'touch',
  'mkdir', 'rm', 'mv', 'compile', 'run', 'stop', 'download'
];

const HELP = [
  'ls [path]              list files',
  'cd [dir]               change directory',
  'pwd                    print working directory',
  'cat <file>             print a text file',
  'open <file>            open in the editor',
  'mkdir <dir>            create a folder',
  'touch <file>           create an empty file',
  'rm [-r] <path>         delete a file (or folder with -r)',
  'mv <src> <dst>         rename, or move into a folder',
  'compile [file.prg]     run bgdc.wasm',
  'run [file.prg|.dcb]    compile if needed, then bgdi',
  'stop                   stop the player',
  'download <path>        save a file or zip a folder',
  'clear                  clear the terminal',
  'help                   this list'
];

function splitArgs(line) {
  const out = [];
  let cur = '';
  let q = '';
  for (let i = 0; i < line.length; i++) {
    const c = line[i];
    if (q) {
      if (c === q) q = '';
      else cur += c;
      continue;
    }
    if (c === '"' || c === "'") {
      q = c;
      continue;
    }
    if (/\s/.test(c)) {
      if (cur) {
        out.push(cur);
        cur = '';
      }
      continue;
    }
    cur += c;
  }
  if (q) throw new Error('unclosed quote');
  if (cur) out.push(cur);
  return out;
}

function takeFlag(args, flag) {
  const next = [];
  let hit = false;
  for (const a of args) {
    if (a === flag) hit = true;
    else next.push(a);
  }
  return { args: next, hit };
}

export class Shell {
  constructor(term, api) {
    this.term = term;
    this.api = api;
    this.cwd = '';
    this.buffer = '';
    this.cursor = 0;
    this.history = [];
    this.histIndex = -1;
    this.stash = '';
    this.busy = false;
    this.depth = 0;
    this.ready = false;
    this.term.onData((data) => this.onData(data));
  }

  start() {
    this.ready = true;
    this.drawInput();
  }

  println(line, err) {
    this.term.write('\r\x1b[K');
    this.term.writeln(
      (err ? '\x1b[31m' : '') + String(line).replace(/\r/g, '') + (err ? '\x1b[0m' : '')
    );
    if (this.ready && !this.busy) this.drawInput();
  }

  async runExclusive(fn) {
    this.depth++;
    this.busy = true;
    if (this.depth === 1) this.term.write('\r\x1b[K');
    try {
      return await fn();
    } finally {
      this.depth--;
      if (this.depth === 0) {
        this.busy = false;
        if (this.ready) this.drawInput();
      }
    }
  }

  promptVisible() {
    return '$ ' + (this.cwd || '/') + ' ';
  }

  promptText() {
    return '\x1b[32m$\x1b[0m \x1b[36m' + (this.cwd || '/') + '\x1b[0m ';
  }

  drawInput() {
    const back = this.buffer.length - this.cursor;
    this.term.write('\r\x1b[K' + this.promptText() + this.buffer);
    if (back > 0) this.term.write('\x1b[' + back + 'D');
  }

  resolve(path) {
    return this.api.vfs.resolve(this.cwd, path);
  }

  onData(data) {
    if (!this.ready || this.busy) return;
    if (data === '\x1b[A') return this.hist(-1);
    if (data === '\x1b[B') return this.hist(1);
    if (data === '\x1b[C') return this.moveCursor(1);
    if (data === '\x1b[D') return this.moveCursor(-1);
    if (data === '\x1b[H' || data === '\x1b[1~') return this.setCursor(0);
    if (data === '\x1b[F' || data === '\x1b[4~') return this.setCursor(this.buffer.length);
    if (data === '\x1b[3~') return this.del();
    if (data === '\t') return this.complete();
    for (let i = 0; i < data.length; i++) {
      const c = data[i];
      const code = c.charCodeAt(0);
      if (c === '\r' || c === '\n') {
        void this.submit();
        return;
      }
      if (c === '\x7f' || c === '\b') {
        this.backspace();
        continue;
      }
      if (c === '\x03') {
        this.term.write('^C\r\n');
        this.buffer = '';
        this.cursor = 0;
        this.histIndex = -1;
        this.drawInput();
        continue;
      }
      if (c === '\x0c') {
        this.term.clear();
        this.drawInput();
        continue;
      }
      if (c === '\x15') {
        this.buffer = this.buffer.slice(this.cursor);
        this.cursor = 0;
        this.drawInput();
        continue;
      }
      if (c === '\x17') {
        this.killWord();
        continue;
      }
      if (code < 32) continue;
      this.buffer = this.buffer.slice(0, this.cursor) + c + this.buffer.slice(this.cursor);
      this.cursor++;
      this.drawInput();
    }
  }

  setCursor(n) {
    this.cursor = Math.max(0, Math.min(this.buffer.length, n));
    this.drawInput();
  }

  moveCursor(delta) {
    this.setCursor(this.cursor + delta);
  }

  backspace() {
    if (!this.cursor) return;
    this.buffer = this.buffer.slice(0, this.cursor - 1) + this.buffer.slice(this.cursor);
    this.cursor--;
    this.drawInput();
  }

  del() {
    if (this.cursor >= this.buffer.length) return;
    this.buffer = this.buffer.slice(0, this.cursor) + this.buffer.slice(this.cursor + 1);
    this.drawInput();
  }

  killWord() {
    let i = this.cursor;
    while (i > 0 && /\s/.test(this.buffer[i - 1])) i--;
    while (i > 0 && !/\s/.test(this.buffer[i - 1])) i--;
    this.buffer = this.buffer.slice(0, i) + this.buffer.slice(this.cursor);
    this.cursor = i;
    this.drawInput();
  }

  hist(dir) {
    if (!this.history.length) return;
    if (this.histIndex < 0) this.stash = this.buffer;
    const next = this.histIndex < 0
      ? (dir < 0 ? this.history.length - 1 : -1)
      : this.histIndex + dir;
    if (next < 0 || next >= this.history.length) {
      this.histIndex = -1;
      this.buffer = this.stash;
    } else {
      this.histIndex = next;
      this.buffer = this.history[next];
    }
    this.cursor = this.buffer.length;
    this.drawInput();
  }

  complete() {
    const before = this.buffer.slice(0, this.cursor);
    const m = before.match(/^(.*?)(\S*)$/);
    const lead = m[1];
    const prefix = m[2];
    const cmd = !lead.trim();
    let names;
    if (cmd) {
      names = COMMANDS.filter((c) => c.startsWith(prefix));
    } else {
      names = this.pathMatches(prefix);
    }
    if (!names.length) return;
    if (names.length === 1) {
      const fill = names[0].slice(prefix.length);
      this.buffer = this.buffer.slice(0, this.cursor) + fill + this.buffer.slice(this.cursor);
      this.cursor += fill.length;
      this.drawInput();
      return;
    }
    const common = names.reduce((a, b) => {
      let i = 0;
      while (i < a.length && a[i] === b[i]) i++;
      return a.slice(0, i);
    });
    if (common.length > prefix.length) {
      const fill = common.slice(prefix.length);
      this.buffer = this.buffer.slice(0, this.cursor) + fill + this.buffer.slice(this.cursor);
      this.cursor += fill.length;
      this.drawInput();
      return;
    }
    this.term.write('\r\n' + names.join('  ') + '\r\n');
    this.drawInput();
  }

  pathMatches(prefix) {
    const slash = prefix.lastIndexOf('/');
    const dirPart = slash >= 0 ? prefix.slice(0, slash + 1) : '';
    const leaf = slash >= 0 ? prefix.slice(slash + 1) : prefix;
    let listing;
    try {
      listing = this.api.vfs.entries(this.resolve(dirPart || '.'));
    } catch (err) {
      return [];
    }
    if (listing.kind === 'file') return [];
    const out = [];
    for (const d of listing.dirs) {
      if (d.startsWith(leaf)) out.push(dirPart + d + '/');
    }
    for (const f of listing.files) {
      if (f.startsWith(leaf)) out.push(dirPart + f);
    }
    return out;
  }

  async submit() {
    if (this.busy) return;
    const line = this.buffer;
    this.buffer = '';
    this.cursor = 0;
    this.histIndex = -1;
    this.term.write('\r\n');
    const trimmed = line.trim();
    if (!trimmed) {
      this.drawInput();
      return;
    }
    if (this.history[this.history.length - 1] !== trimmed) this.history.push(trimmed);
    if (this.history.length > 80) this.history.shift();
    await this.runExclusive(() => this.exec(trimmed));
  }

  async exec(line) {
    let argv;
    try {
      argv = splitArgs(line);
    } catch (err) {
      this.api.log(err.message, true);
      return;
    }
    const cmd = (argv.shift() || '').toLowerCase();
    try {
      await this.dispatch(cmd, argv);
    } catch (err) {
      this.api.log(err && err.message ? err.message : String(err), true);
    }
  }

  async dispatch(cmd, args) {
    switch (cmd) {
      case 'help':
      case '?':
        HELP.forEach((line) => this.api.log(line));
        return;
      case 'clear':
      case 'cls':
        this.term.clear();
        return;
      case 'pwd':
        this.api.log(this.cwd || '/');
        return;
      case 'cd':
        this.cd(args[0]);
        return;
      case 'ls':
      case 'dir':
        this.ls(args);
        return;
      case 'cat':
      case 'type':
        if (!args[0]) throw new Error('usage: cat <file>');
        this.cat(args[0]);
        return;
      case 'open':
        if (!args[0]) throw new Error('usage: open <file>');
        this.api.open(this.resolve(args[0]));
        return;
      case 'mkdir':
        if (!args[0]) throw new Error('usage: mkdir <dir>');
        this.api.mkdir(this.resolve(args[0]));
        return;
      case 'touch': {
        if (!args[0]) throw new Error('usage: touch <file>');
        const path = this.resolve(args[0]);
        if (this.api.vfs.isDir(path) && this.api.vfs.has(path)) throw new Error(path + ' is a directory');
        if (!this.api.vfs.has(path)) this.api.write(path, '');
        else this.api.refresh();
        return;
      }
      case 'rm': {
        const parsed = takeFlag(args, '-r');
        if (!parsed.args[0]) throw new Error('usage: rm [-r] <path>');
        this.api.remove(this.resolve(parsed.args[0]), parsed.hit);
        return;
      }
      case 'mv':
        if (args.length < 2) throw new Error('usage: mv <src> <dst>');
        this.api.move(this.resolve(args[0]), this.resolve(args[1]));
        return;
      case 'compile':
      case 'bgdc':
        return this.api.compile(args[0] ? this.resolve(args[0]) : undefined);
      case 'run':
      case 'bgdi':
        return this.api.run(args[0] ? this.resolve(args[0]) : undefined);
      case 'stop':
        this.api.stop();
        return;
      case 'download':
        if (!args[0]) throw new Error('usage: download <path>');
        this.api.download(this.resolve(args[0]));
        return;
      default:
        throw new Error(cmd + ': command not found (try help)');
    }
  }

  cd(arg) {
    const dest = arg == null || arg === '' ? '' : this.resolve(arg);
    if (dest && this.api.vfs.files.has(dest)) throw new Error(dest + ' is a file');
    if (dest && !this.api.vfs.isDir(dest)) throw new Error(dest + ': no such directory');
    this.cwd = dest;
    this.api.selectDir(this.cwd);
  }

  ls(args) {
    const long = takeFlag(args, '-l');
    const path = long.args[0] ? this.resolve(long.args[0]) : this.cwd;
    const listing = this.api.vfs.entries(path);
    if (listing.kind === 'file') {
      this.api.log(listing.files[0]);
      return;
    }
    const rows = [];
    for (const d of listing.dirs) rows.push({ name: d, dir: true });
    for (const f of listing.files) rows.push({ name: f, dir: false });
    if (!rows.length) return;
    for (const row of rows) {
      const full = this.api.vfs.join(listing.kind === 'file' ? this.api.vfs.parent(path) : path, row.name);
      if (long.hit) {
        const size = row.dir ? ''.padStart(8) : String((this.api.vfs.read(full) || []).length).padStart(8);
        const mark = row.dir ? 'd' : '-';
        this.api.log(`${mark} ${size}  ${row.name}${row.dir ? '/' : ''}`);
      } else if (row.dir) {
        this.api.log('\x1b[36m' + row.name + '/\x1b[0m');
      } else {
        this.api.log(row.name);
      }
    }
  }

  cat(arg) {
    const path = this.resolve(arg);
    if (this.api.vfs.isDir(path) && !this.api.vfs.files.has(path)) {
      throw new Error(path + ' is a directory');
    }
    if (path === this.api.currentPath()) this.api.flush();
    const data = this.api.vfs.read(path);
    if (!data) throw new Error(path + ': no such file');
    if (!this.api.vfs.isText(path)) {
      const info = this.api.detectDcb(data);
      if (info && info.ok) this.api.log(this.api.formatDcb(info));
      else this.api.log(path + ' (' + data.length + ' bytes, binary)');
      return;
    }
    const text = this.api.vfs.readText(path).replace(/\s+$/, '');
    if (text) this.api.log(text);
  }
}
