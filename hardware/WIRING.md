# 🔌 Wiring guide

Everything runs at **3.3V** from the ESP32-C3-Zero's `3V3` pin. The 2W speaker is the
only thing that should NOT touch a GPIO directly — it goes through the MAX98357A amp.

## Power
```
LiPo (3.7V) ──► TP4056 charger ──► slide switch ──► ESP32-C3-Zero 5V/VBUS pad
USB-C also charges the LiPo through the TP4056.
```
> Use a TP4056 module **with protection** (DW01 + FS8205). Tie all grounds together.

## OLED — SSD1306 0.96" (I2C)
| OLED | ESP32-C3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO4 |
| SCL | GPIO5 |

If the screen stays black, your module may be at address `0x3D` — change `OLED_I2C_ADDR` in `config.h`.

## Microphone — INMP441 (I2S input)
| INMP441 | ESP32-C3 |
|---|---|
| VDD | 3V3 |
| GND | GND |
| SCK | GPIO0 |
| WS  | GPIO1 |
| SD  | GPIO2 |
| L/R | **GND** (selects LEFT channel — required) |

## Speaker amp — MAX98357A (I2S output)
| MAX98357A | ESP32-C3 |
|---|---|
| VIN | 3V3 (or 5V for louder) |
| GND | GND |
| BCLK | GPIO6 |
| LRC  | GPIO7 |
| DIN  | GPIO8 |
| SD   | 3V3 (tie HIGH to enable) |
| GAIN | leave floating = 9 dB |
| Speaker + / − | to the 2W speaker |

## Touch buttons — TTP223 ×3 (active HIGH)
| Button | ESP32-C3 | Job |
|---|---|---|
| TALK | GPIO3 | push-to-talk (hold to record) |
| NEXT | GPIO18 | cycle silly faces |
| MODE | GPIO19 | toggle assistant ⇄ face-toy |

Each TTP223: `VCC→3V3`, `GND→GND`, `I/O→` the GPIO above.

## ⚠️ Reserved pins — do not use
| Pin | Why |
|---|---|
| GPIO9 | BOOT button (flash mode) |
| GPIO10 | onboard WS2812 RGB LED |
| GPIO12–17 | onboard 4MB flash (not broken out) |
| GPIO20 / 21 | UART0 RX/TX (serial logging) |

## Quick test order
1. Flash, power on → OLED should show a face within ~2s.
2. No WiFi yet → bot makes `DasaiMochi-Setup` hotspot (RGB LED purple).
3. Configure → connects to WiFi (LED blinks blue → off).
4. Hold TALK and speak → LISTEN face + green LED.
5. Release → THINK (amber) → TALK (cyan) as it replies.
