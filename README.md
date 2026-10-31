# Moonlight PS4

Unofficial [Sunshine](https://github.com/LizardByte/Sunshine) / NVIDIA GameStream client for a **jailbroken PS4** (GoldHEN / HEN). Built with the [OpenOrbis](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain) toolchain.

**Current version: 0.1.0**

Arabic: عميل بث لـ PS4 المعدل يستقبل صورة Sunshine من الكمبيوتر عبر بروتوكول Moonlight (HTTP + RTSP + RTP H.264).

## Status

| Feature | State |
| --- | --- |
| On-console IP editor + HUD | Done |
| Sunshine `serverinfo` over HTTP 47989 | Done |
| Applist / launch / RTSP PLAY | Done (HTTPS pairing still limited) |
| RTP H.264 receive + NAL reassembly | Done |
| Hardware video (`libSceVideodec2`) | Next |
| Full PIN pairing (AES/SHA/RSA + client cert) | Next — needs Mbed TLS |
| DualShock 4 → Sunshine control AES | Next |

Installable artifact from GitHub Actions: `IV0000-MLHT00001_00-MOONLIGHT0000000.pkg`

## Requirements

- PS4 with GoldHEN-compatible firmware
- PC running [Sunshine](https://github.com/LizardByte/Sunshine)
- Same LAN (UDP 47998 video, TCP 47989 HTTP, TCP 48010 RTSP)

## Install on PS4

1. Download the `.pkg` from the latest GitHub Actions run (**Actions → Build → PS4 PKG**).
2. Copy it to the console (USB / FTP) and install with Package Installer. `CATEGORY=gd`.
3. Optional config over FTP: `/data/moonlight/moonlight.ini` (see `assets/moonlight.ini`).
4. Start Sunshine on the PC, then open **Moonlight** on the PS4.
5. Set the host IP with D-Pad, press **X**. **OPTIONS** quits.

Debug UDP log (optional): `debug_host=` in the ini, then on the PC: `nc -ulk 9999`.

## Build

### Host tests (Windows / Linux / CI)

```bash
make -f Makefile.host test cli
./build-host/moonlight-host --serverinfo 192.168.1.10
```

### PS4 package

Linux x86_64 with OpenOrbis (`OO_PS4_TOOLCHAIN` set):

```bash
make
# → IV0000-MLHT00001_00-MOONLIGHT0000000.pkg
```

GitHub Actions runs both jobs on every push.

## Layout

```
include/ml/          public headers
src/core/            GameStream HTTP/XML, RTSP, RTP, HUD app
src/platform/orbis/  PS4 video-out, pad, net
src/platform/host/   desktop CLI for CI
tests/               protocol unit tests
.github/workflows/   OpenOrbis PKG + host tests
```

## License

MIT. Moonlight / GameStream protocol is used for interoperability with Sunshine. This project is not affiliated with NVIDIA, Sony, or LizardByte.
