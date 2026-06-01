# Firmware binary goes here

The web flasher (`/docs/index.html` → ESP Web Tools) installs **one merged binary**:

```
dasai_mochi_bot.merged.bin   ← flashed at offset 0x0
```

This file is **not committed yet** — you build it once from the Arduino sketch,
then drop it in this folder. After that, anyone can flash from the GitHub Pages
site with no toolchain.

## How to build the merged binary

### Option A — Arduino IDE + esptool (quickest)
1. Open `firmware/dasai_mochi_bot/dasai_mochi_bot.ino` in Arduino IDE.
2. Boards Manager → install **esp32 by Espressif**.
3. Select board: **ESP32C3 Dev Module**. Set:
   - USB CDC On Boot: **Enabled**
   - Flash Size: **4MB**
   - Partition Scheme: **Default 4MB with spiffs**
4. Install libraries: Adafruit GFX, Adafruit SSD1306, Adafruit NeoPixel, ArduinoJson (v7).
5. **Sketch → Export Compiled Binary.** This produces, in the build folder:
   - `dasai_mochi_bot.ino.bootloader.bin`
   - `dasai_mochi_bot.ino.partitions.bin`
   - `dasai_mochi_bot.ino.bin`
   - (`boot_app0.bin` ships with the core)
6. Merge them into one file with esptool:

```bash
esptool.py --chip esp32c3 merge_bin -o dasai_mochi_bot.merged.bin \
  --flash_mode dio --flash_freq 80m --flash_size 4MB \
  0x0    dasai_mochi_bot.ino.bootloader.bin \
  0x8000 dasai_mochi_bot.ino.partitions.bin \
  0xe000 boot_app0.bin \
  0x10000 dasai_mochi_bot.ino.bin
```

7. Copy `dasai_mochi_bot.merged.bin` into this folder and commit.

### Option B — PlatformIO (reproducible / CI-friendly)
A `platformio.ini` is included at the repo root. Run:

```bash
pio run                       # builds
pio run -t mergebin           # if configured, or use the esptool step above
```

> **Why merged?** ESP Web Tools can flash multiple parts, but a single
> merged binary at `0x0` is the most foolproof for first-time users and
> avoids offset mistakes that cause boot loops.
