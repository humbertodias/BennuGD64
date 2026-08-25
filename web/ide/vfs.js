/* In-memory project filesystem (path → Uint8Array). */

const TEXT_RE = /\.(prg|inc|h|txt|md|imp|import)$/i;

export class VirtualFS {
  constructor() {
    this.files = new Map();
  }

  normalize(path) {
    return String(path || '')
      .replace(/\\/g, '/')
      .replace(/^\/+/, '')
      .replace(/\/+/g, '/');
  }

  list() {
    return [...this.files.keys()].sort((a, b) => a.localeCompare(b));
  }

  has(path) {
    return this.files.has(this.normalize(path));
  }

  read(path) {
    return this.files.get(this.normalize(path));
  }

  write(path, data) {
    const key = this.normalize(path);
    if (!key) throw new Error('empty path');
    if (typeof data === 'string') data = new TextEncoder().encode(data);
    this.files.set(key, data instanceof Uint8Array ? data : new Uint8Array(data));
  }

  remove(path) {
    return this.files.delete(this.normalize(path));
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
}
