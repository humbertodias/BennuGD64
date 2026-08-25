/* Store-only ZIP (no compression) for explorer folder downloads. */

const CRC_TABLE = new Uint32Array(256);
for (let n = 0; n < 256; n++) {
  let c = n;
  for (let k = 0; k < 8; k++) c = (c & 1) ? (0xedb88320 ^ (c >>> 1)) : (c >>> 1);
  CRC_TABLE[n] = c >>> 0;
}

function crc32(data) {
  let c = 0xffffffff;
  for (let i = 0; i < data.length; i++) c = CRC_TABLE[(c ^ data[i]) & 0xff] ^ (c >>> 8);
  return (c ^ 0xffffffff) >>> 0;
}

function u16(n) {
  return new Uint8Array([n & 0xff, (n >>> 8) & 0xff]);
}

function u32(n) {
  return new Uint8Array([n & 0xff, (n >>> 8) & 0xff, (n >>> 16) & 0xff, (n >>> 24) & 0xff]);
}

function concat(parts) {
  let total = 0;
  for (const p of parts) total += p.length;
  const out = new Uint8Array(total);
  let off = 0;
  for (const p of parts) {
    out.set(p, off);
    off += p.length;
  }
  return out;
}

export function zipStore(entries) {
  const locals = [];
  const centrals = [];
  let offset = 0;
  const enc = new TextEncoder();

  for (const { name, data } of entries) {
    const raw = data ? new Uint8Array(data) : new Uint8Array();
    const filename = enc.encode(name);
    const crc = crc32(raw);
    const header = concat([
      u32(0x04034b50),
      u16(20),
      u16(1 << 11),
      u16(0),
      u16(0),
      u16(0),
      u32(crc),
      u32(raw.length),
      u32(raw.length),
      u16(filename.length),
      u16(0),
      filename
    ]);
    locals.push(header, raw);
    centrals.push(concat([
      u32(0x02014b50),
      u16(20),
      u16(20),
      u16(1 << 11),
      u16(0),
      u16(0),
      u16(0),
      u32(crc),
      u32(raw.length),
      u32(raw.length),
      u16(filename.length),
      u16(0),
      u16(0),
      u16(0),
      u16(0),
      u32(0),
      u32(offset),
      filename
    ]));
    offset += header.length + raw.length;
  }

  const central = concat(centrals);
  const end = concat([
    u32(0x06054b50),
    u16(0),
    u16(0),
    u16(centrals.length),
    u16(centrals.length),
    u32(central.length),
    u32(offset),
    u16(0)
  ]);
  return concat([...locals, central, end]);
}
