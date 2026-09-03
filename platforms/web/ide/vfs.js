/* In-memory project filesystem (path → Uint8Array), with directory nodes. */

const TEXT_RE = /\.(prg|inc|h|txt|md|imp|import)$/i;

export class VirtualFS {
  constructor() {
    this.files = new Map();
    this.dirs = new Set();
  }

  normalize(path) {
    return String(path || '')
      .replace(/\\/g, '/')
      .replace(/^\/+/, '')
      .replace(/\/+$/, '')
      .replace(/\/+/g, '/');
  }

  parent(path) {
    const key = this.normalize(path);
    const slash = key.lastIndexOf('/');
    return slash > 0 ? key.slice(0, slash) : '';
  }

  basename(path) {
    const key = this.normalize(path);
    const slash = key.lastIndexOf('/');
    return slash >= 0 ? key.slice(slash + 1) : key;
  }

  join(dir, name) {
    const base = this.normalize(dir);
    const leaf = this.normalize(name);
    if (!leaf) return base;
    return base ? base + '/' + leaf : leaf;
  }

  resolve(from, path) {
    const rel = path == null ? '' : String(path);
    const abs = rel.startsWith('/') || rel.startsWith('\\');
    const combined = abs
      ? rel
      : (rel ? (this.normalize(from) ? this.normalize(from) + '/' + rel : rel) : this.normalize(from));
    const parts = [];
    for (const p of String(combined).replace(/\\/g, '/').split('/')) {
      if (!p || p === '.') continue;
      if (p === '..') {
        parts.pop();
        continue;
      }
      parts.push(p);
    }
    return parts.join('/');
  }

  entries(dir = '') {
    const key = this.normalize(dir);
    if (key && this.files.has(key)) {
      return { kind: 'file', dirs: [], files: [this.basename(key)] };
    }
    if (key && !this.isDir(key)) throw new Error((key || '/') + ' not found');
    const prefix = key ? key + '/' : '';
    const dirs = new Set();
    const files = [];
    for (const d of this.dirs) {
      if (!key) {
        if (!d.includes('/')) dirs.add(d);
      } else if (d.startsWith(prefix)) {
        const rest = d.slice(prefix.length);
        if (rest && !rest.includes('/')) dirs.add(rest);
      }
    }
    for (const f of this.files.keys()) {
      if (!key) {
        const slash = f.indexOf('/');
        if (slash < 0) files.push(f);
        else dirs.add(f.slice(0, slash));
      } else if (f.startsWith(prefix)) {
        const rest = f.slice(prefix.length);
        const slash = rest.indexOf('/');
        if (slash < 0) files.push(rest);
        else dirs.add(rest.slice(0, slash));
      }
    }
    return { kind: 'dir', dirs: [...dirs].sort(), files: files.sort() };
  }

  list() {
    return [...this.files.keys()].sort((a, b) => a.localeCompare(b));
  }

  has(path) {
    const key = this.normalize(path);
    return this.files.has(key) || this.dirs.has(key);
  }

  isDir(path) {
    const key = this.normalize(path);
    if (!key) return true;
    if (this.files.has(key)) return false;
    if (this.dirs.has(key)) return true;
    const prefix = key + '/';
    for (const f of this.files.keys()) {
      if (f.startsWith(prefix)) return true;
    }
    for (const d of this.dirs) {
      if (d === key || d.startsWith(prefix)) return true;
    }
    return false;
  }

  mkdir(path) {
    const key = this.normalize(path);
    if (!key) return;
    if (this.files.has(key)) throw new Error(key + ' is a file');
    if (key.split('/').includes('..')) throw new Error('invalid path');
    let acc = '';
    for (const part of key.split('/').filter(Boolean)) {
      acc = acc ? acc + '/' + part : part;
      if (this.files.has(acc)) throw new Error(acc + ' is a file');
      this.dirs.add(acc);
    }
  }

  read(path) {
    return this.files.get(this.normalize(path));
  }

  write(path, data) {
    const key = this.normalize(path);
    if (!key) throw new Error('empty path');
    if (key.split('/').includes('..')) throw new Error('invalid path');
    if (this.dirs.has(key)) throw new Error(key + ' is a directory');
    const parent = this.parent(key);
    if (parent) this.mkdir(parent);
    if (typeof data === 'string') data = new TextEncoder().encode(data);
    this.files.set(key, data instanceof Uint8Array ? data : new Uint8Array(data));
  }

  remove(path) {
    const key = this.normalize(path);
    if (!key) return false;
    const prefix = key + '/';
    let hit = this.files.delete(key);
    hit = this.dirs.delete(key) || hit;
    for (const f of [...this.files.keys()]) {
      if (f.startsWith(prefix)) {
        this.files.delete(f);
        hit = true;
      }
    }
    for (const d of [...this.dirs]) {
      if (d.startsWith(prefix)) {
        this.dirs.delete(d);
        hit = true;
      }
    }
    return hit;
  }

  rename(from, to) {
    const src = this.normalize(from);
    const dest = this.normalize(to);
    if (!src) throw new Error('empty path');
    if (!dest) throw new Error('empty path');
    if (src.split('/').includes('..') || dest.split('/').includes('..')) {
      throw new Error('invalid path');
    }
    if (dest === src) return dest;
    if (dest.startsWith(src + '/')) throw new Error('cannot move into itself');
    if (this.has(dest)) throw new Error(dest + ' already exists');

    if (this.files.has(src)) {
      this.write(dest, this.files.get(src));
      this.files.delete(src);
      return dest;
    }
    if (!this.isDir(src)) throw new Error(src + ' not found');

    this.mkdir(dest);
    const prefix = src + '/';
    const remap = (old) => dest + old.slice(src.length);
    for (const d of [...this.dirs]) {
      if (d.startsWith(prefix)) this.dirs.add(remap(d));
    }
    for (const [f, data] of [...this.files]) {
      if (f.startsWith(prefix)) this.write(remap(f), data);
    }
    this.remove(src);
    return dest;
  }

  move(from, destDir) {
    const src = this.normalize(from);
    const destParent = this.normalize(destDir);
    if (!src) throw new Error('empty path');
    if (src.split('/').includes('..') || destParent.split('/').includes('..')) {
      throw new Error('invalid path');
    }
    if (destParent === src || (destParent && destParent.startsWith(src + '/'))) {
      throw new Error('cannot move into itself');
    }
    const dest = destParent ? destParent + '/' + this.basename(src) : this.basename(src);
    return this.rename(src, dest);
  }

  readText(path) {
    const bytes = this.read(path);
    if (!bytes) return '';
    return new TextDecoder('utf-8', { fatal: false }).decode(bytes);
  }

  isText(path) {
    return TEXT_RE.test(this.normalize(path));
  }

  snapshot() {
    const out = {};
    for (const [path, data] of this.files) out[path] = data;
    return out;
  }

  merge(files) {
    for (const [path, data] of Object.entries(files || {})) {
      if (data) this.write(path, data);
    }
  }

  tree() {
    const dirSet = new Set(this.dirs);
    for (const path of this.files.keys()) {
      let acc = '';
      const parts = path.split('/');
      for (let i = 0; i < parts.length - 1; i++) {
        acc = acc ? acc + '/' + parts[i] : parts[i];
        dirSet.add(acc);
      }
    }

    const root = { name: '', path: '', kind: 'dir', children: [] };
    const nodes = new Map([['', root]]);
    const ensureDir = (path) => {
      if (nodes.has(path)) return nodes.get(path);
      const node = { name: this.basename(path), path, kind: 'dir', children: [] };
      nodes.set(path, node);
      ensureDir(this.parent(path)).children.push(node);
      return node;
    };
    for (const d of dirSet) ensureDir(d);
    for (const path of this.files.keys()) {
      ensureDir(this.parent(path)).children.push({
        name: this.basename(path),
        path,
        kind: 'file'
      });
    }

    const sortNode = (node) => {
      if (!node.children) return;
      node.children.sort((a, b) => {
        if (a.kind !== b.kind) return a.kind === 'dir' ? -1 : 1;
        return a.name.localeCompare(b.name);
      });
      node.children.forEach(sortNode);
    };
    sortNode(root);
    return root;
  }

  subtree(path) {
    const key = this.normalize(path);
    const prefix = key ? key + '/' : '';
    const files = {};
    const dirs = [];
    if (key && this.isDir(key)) dirs.push(key);
    for (const d of this.dirs) {
      if (!key || d === key || d.startsWith(prefix)) dirs.push(d);
    }
    for (const [f, data] of this.files) {
      if (!key || f === key || f.startsWith(prefix)) files[f] = data;
    }
    return { files, dirs: [...new Set(dirs)] };
  }
}
