/* DCB/DCL header detector — same layout as core/include/dcb.h */

const DCB_MAGIC = new Uint8Array([0x64, 0x63, 0x62, 0x0d, 0x0a, 0x1f, 0x00, 0x00]);
const DCL_MAGIC = new Uint8Array([0x64, 0x63, 0x6c, 0x0d, 0x0a, 0x1f, 0x00, 0x00]);

function magicEq(bytes, magic) {
  if (!bytes || bytes.length < magic.length) return false;
  for (let i = 0; i < magic.length; i++) {
    if (bytes[i] !== magic[i]) return false;
  }
  return true;
}

function readU32LE(bytes, offset) {
  return bytes[offset]
    | (bytes[offset + 1] << 8)
    | (bytes[offset + 2] << 16)
    | (bytes[offset + 3] << 24);
}

export function detectDcb(bytes) {
  if (!(bytes instanceof Uint8Array)) bytes = new Uint8Array(bytes || []);
  if (bytes.length < 12) {
    return { ok: false, error: 'file too short for a DCB header' };
  }

  let kind = null;
  if (magicEq(bytes, DCB_MAGIC)) kind = 'DCB';
  else if (magicEq(bytes, DCL_MAGIC)) kind = 'DCL';
  else {
    return { ok: false, error: 'not a valid DCB/DCL magic' };
  }

  const version = readU32LE(bytes, 8) >>> 0;
  const major = (version >> 8) & 0xff;
  const minor = version & 0xff;
  return {
    ok: true,
    kind,
    version,
    major,
    minor,
    label: `${major}.${minor.toString(16).padStart(2, '0')}`,
    hex: '0x' + version.toString(16).toUpperCase().padStart(4, '0'),
    runtimeOk: version >= 0x0700,
    libOk: version >= 0x0710
  };
}

export function formatDcb(info) {
  if (!info.ok) return `DCB detector: ${info.error}`;
  return [
    `DCB detector: ${info.kind}`,
    `  version ${info.label}  (${info.hex})`,
    `  runtime accepts >= 0x0700: ${info.runtimeOk ? 'yes' : 'no'}`,
    `  library requires >= 0x0710: ${info.libOk ? 'yes' : 'no'}`
  ].join('\n');
}
