// ============================================================================
//  audio_io.h  —  I2S microphone (INMP441) + I2S amp (MAX98357A) handling
//
//  record() : capture mic PCM while the TALK button is held (push-to-talk).
//  speak()  : send text to a TTS endpoint, stream the returned audio to the
//             MAX98357A, and call back with a live amplitude so the face can
//             lip-sync. Uses OpenAI/Groq TTS (returns PCM/WAV we can stream).
// ============================================================================
#pragma once
#include <Arduino.h>
#include <functional>

class AudioIO {
 public:
  void begin();

  // Record 16-bit mono @ MIC_SAMPLE_RATE into buf until `holdPin` released
  // (or maxSamples reached). Returns number of samples captured.
  size_t record(int16_t* buf, size_t maxSamples, uint8_t holdPin);

  // Synthesize `text` and play it. ampCb receives 0..255 loudness per frame
  // so the OLED mouth can move in time.
  void speak(const String& text, const String& provider, const String& apiKey,
             std::function<void(uint8_t)> ampCb);

 private:
  bool micReady = false, spkReady = false;
  void startMic();
  void startSpk();
};
