// ============================================================================
//  Dasai Mochi Bot  —  config.h
//  Central place for pins, constants, and tunables.
//  Board: Waveshare ESP32-C3-Zero (ESP32-C3FH4, RISC-V single core, 4MB flash)
// ============================================================================
#pragma once

// ---------------------------------------------------------------------------
//  FIRMWARE INFO
// ---------------------------------------------------------------------------
#define FW_NAME     "Dasai Mochi Bot"
#define FW_VERSION  "1.0.0"

// ---------------------------------------------------------------------------
//  PIN MAP  (ESP32-C3-Zero)
//
//  Notes on the ESP32-C3-Zero:
//    * Exposed GPIOs: 0,1,2,3,4,5,6,7,8,9,10,18,19,20,21
//    * GPIO12..GPIO17 are NOT broken out (used by the stacked 4MB flash).
//    * GPIO9  = BOOT button  (hold during USB connect to enter flash mode).
//    * GPIO10 = onboard WS2812 RGB LED.
//    * GPIO20 = UART0 RX, GPIO21 = UART0 TX (used for serial logging).
//
//  The ESP32-C3 has NO dedicated I2C/I2S pins — any GPIO can be mapped,
//  which is why the pins below are freely chosen and easy to rewire.
// ---------------------------------------------------------------------------

// --- OLED (SSD1306 0.96", I2C) ---
#define PIN_OLED_SDA      4
#define PIN_OLED_SCL      5
#define OLED_I2C_ADDR     0x3C       // most 0.96" SSD1306 modules; some are 0x3D
#define OLED_WIDTH        128
#define OLED_HEIGHT       64

// --- INMP441 microphone (I2S input) ---
#define PIN_I2S_MIC_SCK   0          // bit clock  (BCLK)
#define PIN_I2S_MIC_WS    1          // word select (LRCLK)
#define PIN_I2S_MIC_SD    2          // serial data out of mic
// INMP441 L/R pin -> tie to GND for LEFT channel (recommended)

// --- MAX98357A I2S amplifier -> 2W speaker (I2S output) ---
#define PIN_I2S_SPK_BCLK  6
#define PIN_I2S_SPK_LRC   7
#define PIN_I2S_SPK_DIN   8
// MAX98357A GAIN pin: leave floating = 9dB. SD pin: tie HIGH to enable.

// --- TTP touch buttons (active-HIGH digital) ---
#define PIN_TOUCH_MAIN    3          // talk / wake  (push-to-talk)
#define PIN_TOUCH_NEXT    18         // next face / menu
#define PIN_TOUCH_MODE    19         // toggle: assistant <-> idle-faces mode

// --- Status LED (onboard WS2812) ---
#define PIN_RGB_LED       10
#define RGB_LED_COUNT     1

// ---------------------------------------------------------------------------
//  AUDIO
// ---------------------------------------------------------------------------
#define MIC_SAMPLE_RATE   16000      // 16 kHz mono is the STT sweet spot
#define SPK_SAMPLE_RATE   16000
#define RECORD_MAX_SECS   8          // hard cap on a single utterance

// ---------------------------------------------------------------------------
//  CAPTIVE PORTAL / PROVISIONING
// ---------------------------------------------------------------------------
#define AP_SSID           "DasaiMochi-Setup"
#define AP_PASSWORD       ""         // open AP for easy first-time setup
#define CONFIG_PORTAL_IP  192,168,4,1
#define DNS_PORT          53
#define HTTP_PORT         80

// How long to wait for a saved WiFi to connect before falling back to AP.
#define WIFI_CONNECT_TIMEOUT_MS  15000

// ---------------------------------------------------------------------------
//  IDLE FACE ANIMATION
// ---------------------------------------------------------------------------
#define IDLE_FRAME_MS     120        // base tick for blink/idle animation
#define IDLE_TIMEOUT_MS   8000        // no interaction -> drift into idle faces

// ---------------------------------------------------------------------------
//  NVS (saved settings) KEYS
// ---------------------------------------------------------------------------
#define NVS_NAMESPACE     "mochi"
#define NVS_KEY_SSID      "wifi_ssid"
#define NVS_KEY_PASS      "wifi_pass"
#define NVS_KEY_PROVIDER  "llm_prov"
#define NVS_KEY_APIKEY    "llm_key"
#define NVS_KEY_MODEL     "llm_model"
#define NVS_KEY_NAME      "bot_name"
