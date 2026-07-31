## Flashing the ESP32 boards (Heltec V3, Cardputer Adv) with esptool

The official web flasher at flasher.meshtastic.org does not reliably handle a
`.factory.bin` from an unofficial build like this one: its "upload your own firmware"
path only knows the exact file names of an officially published release, so it cannot
find the files in this release and can leave the device unable to boot (it will keep
printing something like `invalid header: 0x...` over serial). Until we ship an
easier tool, please flash these two boards with `esptool` directly.

### 1. Get esptool

No Python install required — grab the standalone binary for your OS from the
official Espressif releases page:

<https://github.com/espressif/esptool/releases/latest>

Pick the archive that matches your machine (for example
`esptool-vX.Y.Z-macos-arm64.tar.gz` for an Apple Silicon Mac,
`esptool-vX.Y.Z-windows-amd64.zip` for Windows), extract it, and you will have an
`esptool` (or `esptool.exe`) you can run directly — no `pip install` needed.

If you already have PlatformIO installed (for example through VS Code), it also
ships its own copy of esptool and that works just as well; you do not need to
download a separate one.

### 2. Connect the board

Plug the board in over USB. You do not need to look up which serial port it landed
on — if it is the only ESP32-family device plugged in, esptool will find it on its
own when you leave out `--port`.

### 3. Erase and flash

From a terminal, in the folder where you downloaded the `.factory.bin`:

```bash
esptool --chip esp32s3 erase_flash
esptool --chip esp32s3 write_flash 0x0 firmware-heltec_v3_zhtw-2.7.26-f35eb6023.factory.bin
```

(Swap in the `firmware-cardputer_adv_zhtw-...factory.bin` file name for the
Cardputer Adv — both boards use `--chip esp32s3`.)

`erase_flash` wipes everything already on the device, including any saved settings.
`write_flash 0x0 ...` writes the full image, which already contains the bootloader
and partition table, so nothing else needs to be flashed separately.

If esptool cannot find the port automatically (for example you have more than one
serial device attached), add `--port` with the device path, e.g.
`--port /dev/cu.usbserial-0001` on macOS or `--port COM5` on Windows.

---

## 用 esptool 燒錄 ESP32 系列（Heltec V3、Cardputer Adv）

flasher.meshtastic.org 的官方網頁版工具，目前沒辦法可靠處理這種非官方 build 的
`.factory.bin`：它的「上傳自訂韌體」那條路徑，只認得官方正式 release 裡固定的檔名，
找不到我們這份 release 裡實際的檔案，可能導致裝置開機失敗（serial console 會一直
重複印出類似 `invalid header: 0x...` 的訊息）。在我們做出更簡單的工具之前，
這兩支板子請先直接用 `esptool` 燒錄。

### 1. 取得 esptool

不需要安裝 Python——直接到 Espressif 官方 release 頁面，下載對應你系統的獨立執行檔：

<https://github.com/espressif/esptool/releases/latest>

依你的系統選擇對應檔案（例如 Apple Silicon Mac 選
`esptool-vX.Y.Z-macos-arm64.tar.gz`，Windows 選
`esptool-vX.Y.Z-windows-amd64.zip`），解壓後就會得到一個可以直接執行的
`esptool`（Windows 是 `esptool.exe`），不需要 `pip install`。

如果你已經裝過 PlatformIO（例如透過 VS Code 的擴充套件），裡面本來就內建一份
esptool，一樣可以直接用，不用另外下載。

### 2. 接上板子

用 USB 線把板子接到電腦。不需要自己去找它接在哪個序列埠（COM port）——只要這是
唯一一台接著的 ESP32 系列裝置，不指定 `--port` 的話 esptool 會自己找到它。

### 3. 清除並燒錄

在終端機裡，切到你下載 `.factory.bin` 的資料夾，執行：

```bash
esptool --chip esp32s3 erase_flash
esptool --chip esp32s3 write_flash 0x0 firmware-heltec_v3_zhtw-2.7.26-f35eb6023.factory.bin
```

（Cardputer Adv 換成 `firmware-cardputer_adv_zhtw-...factory.bin` 這個檔名即可，
兩支板子都是 `--chip esp32s3`。）

`erase_flash` 會清空裝置上原本的所有資料，包含已儲存的設定。`write_flash 0x0 ...`
寫入的是完整映像檔（已經包含 bootloader 與 partition table），不需要再另外燒別的檔案。

如果 esptool 沒辦法自動找到序列埠（例如你同時接了不只一台序列裝置），加上
`--port` 指定路徑即可，例如 macOS 的 `--port /dev/cu.usbserial-0001`，
或 Windows 的 `--port COM5`。
