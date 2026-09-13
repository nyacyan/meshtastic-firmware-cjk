# Meshtastic Firmware with CJK Display Support (Unofficial)

This is an unofficial derivative of [meshtastic/firmware](https://github.com/meshtastic/firmware)
based on release `v2.7.26.54e0d8d`, building upon the pioneering work of [Kent-Liu/meshtastic-firmware-zhtw](https://github.com/Kent-Liu/meshtastic-firmware-zhtw).
It adds CJK (Traditional & Simplified Chinese via GB2312) glyph rendering and optional Bopomofo input method,
plus LilyGO T-Beam GPS enhancements:

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

## Meshtastic 中文 CJK 显示支持与 T-Beam 增强固件（非官方）

这是基于 [meshtastic/firmware](https://github.com/meshtastic/firmware) `v2.7.26.54e0d8d` 的非官方衍生版本。主要特性如下：

1. **中文 CJK 字符点阵支持（非界面汉化）**：
   - 扩充并整合 GB2312 简体/繁体常用中文字库点阵（`traditional_chinese_utf8_10x10.h`），使节点在 OLED 或电子墨水屏上能清晰、正确地渲染显示中文群聊消息、私聊消息及节点名称。
   - 继承并保留了针对支持机型的注音（Bopomofo）屏幕与实体键盘输入法支持。

2. **LilyGO T-Beam (ESP32 + u-blox NEO-6M / M8N) GPS 深度修复与 A-GPS 注入**：
   - **PR #11697 丢星休眠修复**：修复 GPS 线程每 200ms 轮询时，因 NMEA 1Hz 速率导致 80% 几率误判“丢星”并触发休眠的官方 Bug。
   - **搜星失败休眠惩罚上限优化**：将原本搜星超时后最高达 1 小时（3600 秒）的惩罚性休眠强行封顶为 2 分钟（120 秒），防止弱信号或室内丢星后节点长时间失联。
   - **动态 A-GPS 辅助注入 (`UBX-AID-INI`)**：符合 u-blox 6/7/8 官方规范，支持开机、蓝牙手机连线校时（`set_time_only`）及手机共享位置时动态向芯片注入卫星时间/坐标辅助数据，彻底根除冷启动频率盲扫。
   - **全速接收模式与 30s 常开搜星**：开启 NEO-6M 最大性能模式并输出 `$GPGSV` 卫星载噪比报文；`GPS_UPDATE_ALWAYS_ON_THRESHOLD_MS` 提升至 30s，使官方 App 设置 30 秒时即可直接进入 100% 常开全速搜星。

### 致谢 (Acknowledgements)

衷心感谢 [Kent-Liu/meshtastic-firmware-zhtw](https://github.com/Kent-Liu/meshtastic-firmware-zhtw) 开源项目及其原作者 Kent Liu。该项目开创了针对 Meshtastic 设备的精巧点阵字库渲染管道与注音输入法体系，为本项目扩展 GB2312 简体中文字库提供了极为宝贵的工程基础与灵感！

---

本项目与 Meshtastic 官方项目无隶属关系，亦未经其背书。Meshtastic® 为 Meshtastic LLC 的注册商标。

**本固件不提供任何形式的保证。刷机有几率让设备无法开机，运行未经验证固件的无线电节点也可能出现非预期行为。请自行承担使用风险。**

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
