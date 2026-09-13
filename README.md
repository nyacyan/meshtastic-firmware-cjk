# Meshtastic Firmware with CJK Display Support (Unofficial)

This is an unofficial derivative of [meshtastic/firmware](https://github.com/meshtastic/firmware),
based on release `v2.7.26.54e0d8d`. It adds CJK (Traditional & Simplified Chinese via GB2312)
glyph rendering and optional Bopomofo input method, plus LilyGO T-Beam GPS enhancements:

It is **not affiliated with or endorsed by the Meshtastic project**. Meshtastic® is a
registered trademark of Meshtastic LLC. Please report problems with these builds here
rather than to the upstream project, and reproduce them on a stock build before
reporting anything upstream.

| Environment | Board | Input / Display |
| --- | --- | --- |
| `tbeam_zhtw` | LilyGO T-Beam (ESP32) | display only (GB2312 CJK + GPS A-GPS & Fixes) |
| `gat562_family_zhtw` | GAT562 Family (nRF52840) | Bopomofo on the on-screen keyboard |
| `seeed_wio_tracker_L1_zhtw` | Seeed Wio Tracker L1 / L1 Lite / L1 Pro | Bopomofo on the on-screen keyboard |
| `seeed_wio_tracker_L1_eink_zhtw` | Seeed Wio Tracker L1 E-Ink | Bopomofo on the on-screen keyboard |
| `heltec-v3_zhtw` | Heltec LoRa32 V3 | display only |
| `heltec-wireless-paper_zhtw` | Heltec Wireless Paper (V1.1 and later) | display only |
| `heltec-mesh-node-t114_zhtw` | Heltec Mesh Node T114 | display only |
| `heltec-v4_zhtw` | Heltec V4 (OLED) | display only |
| `heltec-v4-r8-oled_zhtw` | Heltec V4 R8 (OLED) | display only |
| `m5stack-cardputer-adv_zhtw` | M5Stack Cardputer Adv | Bopomofo on the physical keyboard |
| `nrf52_promicro_diy_tcxo_zhtw` | NRF52 Pro-micro DIY | display only |
| `nrf52_promicro_diy_tcxo_ime_zhtw` | NRF52 Pro-micro DIY | Bopomofo, needs navigation hardware fitted |
| `nrf52_promicro_diy_tcxo_kb_zhtw` | NRF52 Pro-micro DIY | Bopomofo on an I2C keyboard (M5Stack CardKB) |

```
pio run -e tbeam_zhtw
```

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

## Meshtastic 中文 CJK 顯示支援與 T-Beam 增強固件（非官方）

這是基於 [meshtastic/firmware](https://github.com/meshtastic/firmware) `v2.7.26.54e0d8d` 的非官方衍生版本。主要特性如下：

1. **中文 CJK 字符點陣支援（非界面漢化）**：
   - 整合 GB2312 簡體/繁體常用中文字庫點陣（`traditional_chinese_utf8_10x10.h`），使節點在 OLED 或墨水屏上能正確渲染和顯示中文群聊消息、私聊消息及節點名稱。
   - 支援注音/Bopomofo 螢幕與實體鍵盤輸入（針對部分支援鍵盤的機種）。

2. **LilyGO T-Beam (ESP32 + u-blox NEO-6M / M8N) GPS 深度修復與 A-GPS 注入**：
   - **PR #11697 丟星休眠修復**：修復 GPS 執行緒每 200ms 輪詢時，因 NMEA 1Hz 速率導致 80% 幾率誤判「丟星」並進入休眠的 Bug。
   - **搜星失敗休眠懲罰上限優化**：將原本搜星逾時後最高達 1 小時（3600 秒）的懲罰性休眠強行封頂為 2 分鐘（120 秒），防止弱訊號或室內丟星後節點長時間失聯。
   - **動態 A-GPS 輔助注入 (`UBX-AID-INI`)**：符合 u-blox 6/7/8 規格，支援開機、藍牙手機連線校時 (`set_time_only`) 及手機分享位置時動態注入衛星時間/座標輔助數據，徹底消除冷啟動頻率盲掃。
   - **全速接收模式與 30s 常開搜星**：開啟 NEO-6M 最大效能模式並輸出 `$GPGSV` 衛星載噪比報文；`GPS_UPDATE_ALWAYS_ON_THRESHOLD_MS` 提升至 30s，使官方 App 設定 30 秒時即可進入常開全速搜星。

本專案與 Meshtastic 官方專案無隸屬關係，亦未經其背書。Meshtastic® 為 Meshtastic LLC 的註冊商標。

**本韌體不提供任何形式的保證。刷機有機會讓裝置無法開機，跑著未經驗證韌體的無線電節點也可能出現非預期行為。請自行承擔使用風險。**

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
