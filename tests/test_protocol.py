"""Mirror of tests/test_core.c so the protocol can be checked without a C toolchain."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))


def xml_status_ok(xml: str) -> bool:
    m = re.search(r'status_code="(\d+)"', xml)
    if m:
        return m.group(1) == "200"
    m = re.search(r"<status_code>(\d+)</status_code>", xml)
    return bool(m and m.group(1) == "200")


def xml_get(xml: str, tag: str) -> str:
    m = re.search(rf"<{tag}[^>]*>(.*?)</{tag}>", xml, re.S)
    if not m:
        raise AssertionError(tag)
    return m.group(1)


def rtp_parse(pkt: bytes):
    assert (pkt[0] >> 6) == 2
    return {"pt": pkt[1] & 0x7F, "seq": (pkt[2] << 8) | pkt[3], "hdr": 12}


def depay(payload: bytes, nal: bytearray) -> tuple[bytearray, bool]:
    nal_type = payload[0] & 0x1F
    if 1 <= nal_type <= 23:
        out = bytearray(b"\x00\x00\x00\x01") + payload
        return out, True
    if nal_type == 28:
        fu = payload[1]
        start, end, typ = (fu >> 7) & 1, (fu >> 6) & 1, fu & 0x1F
        nri = payload[0] & 0x60
        if start:
            nal[:] = b"\x00\x00\x00\x01" + bytes([nri | typ]) + payload[2:]
        else:
            nal.extend(payload[2:])
        return nal, bool(end)
    raise AssertionError("unsupported")


def test_xml() -> None:
    xml = (
        '<?xml version="1.0"?><root status_code="200"><hostname>PC</hostname>'
        "<PairStatus>1</PairStatus><HttpsPort>47984</HttpsPort></root>"
    )
    assert xml_status_ok(xml)
    assert xml_get(xml, "hostname") == "PC"
    assert xml_get(xml, "PairStatus") == "1"
    assert xml_get(xml, "HttpsPort") == "47984"


def test_rtp() -> None:
    pkt = bytes([0x80, 96, 0x00, 0x01, 0, 0, 0, 0, 0, 0, 0, 1, 0x65, 1, 2, 3, 4, 5, 6, 7])
    h = rtp_parse(pkt)
    assert h["pt"] == 96 and h["seq"] == 1
    nal, done = depay(pkt[12:], bytearray())
    assert done and len(nal) == 12 and nal[4] == 0x65


def test_fu() -> None:
    nal = bytearray()
    nal, done = depay(bytes([0x7C, 0x85, 0xAA, 0xBB]), nal)
    assert not done
    nal, done = depay(bytes([0x7C, 0x05, 0xCC]), nal)
    assert not done
    nal, done = depay(bytes([0x7C, 0x45, 0xDD]), nal)
    assert done and nal[4] == 0x65 and len(nal) == 9


def test_ini() -> None:
    text = (ROOT / "assets" / "moonlight.ini").read_text(encoding="utf-8")
    host = None
    for line in text.splitlines():
        line = line.strip()
        if line.startswith("host="):
            host = line.split("=", 1)[1]
    assert host and host.count(".") == 3


def main() -> int:
    test_xml()
    test_rtp()
    test_fu()
    test_ini()
    print("ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
