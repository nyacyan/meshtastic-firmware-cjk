# Traditional Chinese firmware (unofficial)

This is an unofficial derivative of [meshtastic/firmware](https://github.com/meshtastic/firmware),
based on release `v2.7.26.54e0d8d`. It adds Traditional Chinese text rendering and a
Bopomofo (zhuyin) input method to five boards, and nothing else: every stock
environment is left exactly as upstream ships it.

It is **not affiliated with or endorsed by the Meshtastic project**. Meshtastic® is a
registered trademark of Meshtastic LLC. Please report problems with these builds here
rather than to the upstream project, and reproduce them on a stock build before
reporting anything upstream.

| Environment | Board | Input |
| --- | --- | --- |
| `gat562_family_zhtw` | GAT562 Family (nRF52840) | Bopomofo on the on-screen keyboard |
| `seeed_wio_tracker_L1_zhtw` | Seeed Wio Tracker L1 / L1 Lite / L1 Pro | Bopomofo on the on-screen keyboard |
| `heltec-v3_zhtw` | Heltec LoRa32 V3 | display only |
| `m5stack-cardputer-adv_zhtw` | M5Stack Cardputer Adv | Bopomofo on the physical keyboard |
| `nrf52_promicro_diy_tcxo_zhtw` | NRF52 Pro-micro DIY | display only |
| `nrf52_promicro_diy_tcxo_ime_zhtw` | NRF52 Pro-micro DIY | Bopomofo, needs navigation hardware fitted |
| `nrf52_promicro_diy_tcxo_kb_zhtw` | NRF52 Pro-micro DIY | Bopomofo on an I2C keyboard (M5Stack CardKB) |

```
pio run -e gat562_family_zhtw
```

Both Pro-micro DIY targets expect the builder to have wired a 128x64 I2C SSD1306 to the
header, and both link with the serial log muted, which is what makes the glyph tables
fit. Bring a new node up on the stock `nrf52_promicro_diy_tcxo` env, where the log still
works, and flash a `_zhtw` one once the hardware is known good.

The `_ime_` variant additionally needs a 4-way switch or trackball on the four free pins
(P1.01, P1.02, P1.06, P1.07); it navigates with those and commits the highlighted key on
a 500 ms hold of the user button, there being no fifth pin for a centre press. Its IME
lives inside the on-screen keyboard and is toggled between Chinese and English on that
keyboard's own ESC key, which is labelled `TW ESC` or `EN ESC` to show the current mode.

The `_kb_` variant is for a physical I2C keyboard instead — an M5Stack CardKB or similar,
found by I2C probe with no configuration. It is a separate env rather than a flag on the
one above because the two use different IME code paths, and the on-screen one drops
physical key characters on the floor. Typing any printable key on the message screen opens
free-text compose, and Chinese/English toggles on `fn`+`c`. That combination is this fork's
own: upstream toggles on Ctrl+Space, which only the Cardputer's built-in keyboard can
generate, and a CardKB has no Ctrl key.

Both IME variants cost the same eight feature modules — motion and magnetometer support,
and INA2xx power telemetry, are the ones that bite — so if you are not going to compose on
the node itself, prefer the display-only env, which keeps them.

The glyph tables and the dictionary are generated data, checked in so the firmware
builds without extra tooling. No font file is redistributed; only rasterised bitmaps
are embedded. Terms for every font and dictionary in the chain are in
[`licenses/`](licenses/), alongside the upstream GPL-3.0 licence that covers the
firmware as a whole.

**These builds come with no warranty of any kind, express or implied. Flashing
firmware can leave a device unusable, and a mesh radio running unverified firmware may
behave in ways you do not expect. Use them at your own risk.**

---

## 繁體中文韌體（非官方）

這是 [meshtastic/firmware](https://github.com/meshtastic/firmware) `v2.7.26.54e0d8d`
的非官方衍生版本，替四款機種加上繁體中文顯示與注音輸入法，其餘一律不動：所有原廠
env 都與上游完全相同。

本專案與 Meshtastic 官方專案無隸屬關係，亦未經其背書。Meshtastic® 為 Meshtastic LLC
的註冊商標。這些韌體如有問題請回報到本專案，不要送去官方；要向官方回報之前，請先在
原廠韌體上重現。

字型點陣表與注音字典都是產生出來的資料，直接收進版本庫，所以編譯時不需要額外工具。
本專案不轉散布任何字型檔，嵌入的只有點陣資料；所有字型與字典的授權條款放在
[`licenses/`](licenses/)，韌體整體則沿用上游的 GPL-3.0。

**本韌體不提供任何形式的保證。刷機有機會讓裝置無法開機，跑著未經驗證韌體的無線電
節點也可能出現非預期行為。請自行承擔使用風險。**

以下為上游 Meshtastic 專案的原始 README，其中的徽章與連結皆指向上游專案。

---

<div align="center" markdown="1">

<img src=".github/meshtastic_logo.png" alt="Meshtastic Logo" width="80"/>
<h1>Meshtastic Firmware</h1>

![GitHub release downloads](https://img.shields.io/github/downloads/meshtastic/firmware/total)
[![CI](https://img.shields.io/github/actions/workflow/status/meshtastic/firmware/main_matrix.yml?branch=master&label=actions&logo=github&color=yellow)](https://github.com/meshtastic/firmware/actions/workflows/ci.yml)
[![CLA assistant](https://cla-assistant.io/readme/badge/meshtastic/firmware)](https://cla-assistant.io/meshtastic/firmware)
[![Fiscal Contributors](https://opencollective.com/meshtastic/tiers/badge.svg?label=Fiscal%20Contributors&color=deeppink)](https://opencollective.com/meshtastic/)
[![Vercel](https://img.shields.io/static/v1?label=Powered%20by&message=Vercel&style=flat&logo=vercel&color=000000)](https://vercel.com?utm_source=meshtastic&utm_campaign=oss)

<a href="https://trendshift.io/repositories/5524" target="_blank"><img src="https://trendshift.io/api/badge/repositories/5524" alt="meshtastic%2Ffirmware | Trendshift" style="width: 250px; height: 55px;" width="250" height="55"/></a>

</div>

</div>

<div align="center">
	<a href="https://meshtastic.org">Website</a>
	-
	<a href="https://meshtastic.org/docs/">Documentation</a>
</div>

## Overview

This repository contains the official device firmware for Meshtastic, an open-source LoRa mesh networking project designed for long-range, low-power communication without relying on internet or cellular infrastructure. The firmware supports various hardware platforms, including ESP32, nRF52, RP2040/RP2350, and Linux-based devices.

Meshtastic enables text messaging, location sharing, and telemetry over a decentralized mesh network, making it ideal for outdoor adventures, emergency preparedness, and remote operations.

### Get Started

- 🔧 **[Building Instructions](https://meshtastic.org/docs/development/firmware/build)** – Learn how to compile the firmware from source.
- ⚡ **[Flashing Instructions](https://meshtastic.org/docs/getting-started/flashing-firmware/)** – Install or update the firmware on your device.

Join our community and help improve Meshtastic! 🚀

## Stats

![Alt](https://repobeats.axiom.co/api/embed/8025e56c482ec63541593cc5bd322c19d5c0bdcf.svg "Repobeats analytics image")
