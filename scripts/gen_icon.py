#!/usr/bin/env python3
"""Write a 512x512 RGB PNG icon (no extra deps)."""
import struct
import zlib
from pathlib import Path


def chunk(tag: bytes, data: bytes) -> bytes:
    crc = zlib.crc32(tag + data) & 0xFFFFFFFF
    return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", crc)


def main() -> None:
    w = h = 512
    rows = []
    for y in range(h):
        row = bytearray(1 + w * 3)
        row[0] = 0
        for x in range(w):
            # dark navy with a cyan "moon" circle
            dx = x - 256
            dy = y - 240
            inside = dx * dx + dy * dy < 140 * 140
            crescent = (dx - 40) * (dx - 40) + dy * dy < 120 * 120
            if inside and not crescent:
                r, g, b = 180, 230, 240
            else:
                r = 12
                g = 18 + (y // 8)
                b = 32 + (x // 16)
            i = 1 + x * 3
            row[i : i + 3] = bytes((r & 255, g & 255, b & 255))
        rows.append(bytes(row))
    raw = b"".join(rows)
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 9))
    png += chunk(b"IEND", b"")
    out = Path(__file__).resolve().parents[1] / "sce_sys" / "icon0.png"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(png)
    print("wrote", out, "bytes", out.stat().st_size)


if __name__ == "__main__":
    main()
