// ============================================================================
//  audio_io.cpp  —  I2S mic capture + speaker playback
//
//  Uses the legacy driver/i2s.h API (stable across ESP32 Arduino core 2.x/3.x
//  for the C3). Two separate I2S ports: I2S_NUM_0 = mic, the same port is
//  reconfigured for output during speak() because the C3 exposes ONE I2S
//  peripheral — so we install/uninstall around record vs speak.
// ============================================================================
#include "audio_io.h"
#include "config.h"
#include "providers.h"
#include <driver/i2s.h>
#include <WiFiClientSecure.h>

#define I2S_PORT I2S_NUM_0

void AudioIO::begin() { /* lazy init per direction */ }

// ---- microphone (RX) ------------------------------------------------------
void AudioIO::startMic() {
  i2s_config_t cfg = {};
  cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
  cfg.sample_rate = MIC_SAMPLE_RATE;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;   // INMP441 is 24-in-32
  cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = 4;
  cfg.dma_buf_len = 256;
  i2s_pin_config_t pins = {};
  pins.bck_io_num = PIN_I2S_MIC_SCK;
  pins.ws_io_num  = PIN_I2S_MIC_WS;
  pins.data_in_num = PIN_I2S_MIC_SD;
  pins.data_out_num = I2S_PIN_NO_CHANGE;
  i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
  i2s_set_pin(I2S_PORT, &pins);
  micReady = true;
}

// ---- speaker (TX) ---------------------------------------------------------
void AudioIO::startSpk() {
  i2s_config_t cfg = {};
  cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
  cfg.sample_rate = SPK_SAMPLE_RATE;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = 8;
  cfg.dma_buf_len = 256;
  i2s_pin_config_t pins = {};
  pins.bck_io_num = PIN_I2S_SPK_BCLK;
  pins.ws_io_num  = PIN_I2S_SPK_LRC;
  pins.data_out_num = PIN_I2S_SPK_DIN;
  pins.data_in_num = I2S_PIN_NO_CHANGE;
  i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
  i2s_set_pin(I2S_PORT, &pins);
  spkReady = true;
}

size_t AudioIO::record(int16_t* buf, size_t maxSamples, uint8_t holdPin) {
  if (spkReady) { i2s_driver_uninstall(I2S_PORT); spkReady = false; }
  if (!micReady) startMic();

  size_t got = 0;
  int32_t raw[256];
  uint32_t start = millis();
  // record while button held; small grace so a quick tap still grabs audio
  while ((digitalRead(holdPin) || millis() - start < 400) && got < maxSamples) {
    size_t bytesRead = 0;
    i2s_read(I2S_PORT, raw, sizeof(raw), &bytesRead, 100 / portTICK_PERIOD_MS);
    size_t n = bytesRead / sizeof(int32_t);
    for (size_t i = 0; i < n && got < maxSamples; ++i) {
      // 32-bit frame -> top 16 bits, with a little gain
      buf[got++] = (int16_t)(raw[i] >> 14);
    }
    if (millis() - start > RECORD_MAX_SECS * 1000UL) break;
  }
  return got;
}

void AudioIO::speak(const String& text, const String& provider,
                    const String& apiKey, std::function<void(uint8_t)> ampCb) {
  if (micReady) { i2s_driver_uninstall(I2S_PORT); micReady = false; }
  if (!spkReady) startSpk();

  // TTS endpoint: OpenAI + Groq expose speech synthesis returning WAV.
  const char* host; const char* path; const char* model; const char* voice;
  if (provider == "groq") {
    host = "api.groq.com"; path = "/openai/v1/audio/speech";
    model = "playai-tts"; voice = "Fritz-PlayAI";
  } else {
    host = "api.openai.com"; path = "/v1/audio/speech";
    model = "gpt-4o-mini-tts"; voice = "alloy";
  }

  String body = String("{\"model\":\"") + model + "\",\"voice\":\"" + voice +
                "\",\"input\":\"" + text + "\",\"response_format\":\"wav\","
                "\"sample_rate\":" + String(SPK_SAMPLE_RATE) + "}";

  WiFiClientSecure client; client.setInsecure();
  if (!client.connect(host, 443)) return;
  client.printf("POST %s HTTP/1.1\r\n", path);
  client.printf("Host: %s\r\n", host);
  client.printf("Authorization: Bearer %s\r\n", apiKey.c_str());
  client.print("Content-Type: application/json\r\n");
  client.printf("Content-Length: %u\r\nConnection: close\r\n\r\n", body.length());
  client.print(body);

  // skip HTTP headers
  String line; uint32_t t0 = millis();
  while (client.connected() && millis() - t0 < 8000) {
    line = client.readStringUntil('\n');
    if (line == "\r" || line.length() == 0) break;
  }
  // skip 44-byte WAV header
  uint8_t skip[44]; size_t s = 0;
  while (s < 44 && client.connected()) { if (client.available()) skip[s++] = client.read(); }

  // stream PCM to the amp, emit amplitude for lip-sync
  int16_t chunk[256]; size_t idx = 0;
  while (client.connected() || client.available()) {
    if (client.available()) {
      uint8_t lo = client.read();
      if (!client.available() && !client.connected()) break;
      uint8_t hi = client.read();
      int16_t sample = (int16_t)((hi << 8) | lo);
      chunk[idx++] = sample;
      if (idx == 256) {
        size_t written; i2s_write(I2S_PORT, chunk, sizeof(chunk), &written,
                                  100 / portTICK_PERIOD_MS);
        // crude peak for the mouth
        int32_t peak = 0; for (int i=0;i<256;++i){int a=abs(chunk[i]); if(a>peak)peak=a;}
        if (ampCb) ampCb((uint8_t)min(255, peak >> 7));
        idx = 0;
      }
    }
  }
  if (idx) { size_t w; i2s_write(I2S_PORT, chunk, idx*2, &w, 100/portTICK_PERIOD_MS); }
  i2s_zero_dma_buffer(I2S_PORT);
  client.stop();
  if (ampCb) ampCb(0);
}
