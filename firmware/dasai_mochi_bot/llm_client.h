// ============================================================================
//  llm_client.h  —  Talks to the user's chosen AI provider over HTTPS.
//
//  Responsibilities:
//     chat()          : text in -> assistant text out (OpenAI-compatible JSON)
//     speechToText()  : PCM audio -> text (Groq/OpenAI Whisper endpoint)
//     (TTS lives in audio_io because it streams straight to the speaker)
//
//  Design choice for a tiny C3: we use a system prompt that asks for SHORT,
//  spoken-style replies (1-2 sentences) so responses fit in RAM and sound
//  natural coming out of a little speaker.
// ============================================================================
#pragma once
#include <Arduino.h>

class LlmClient {
 public:
  void begin();

  // Returns assistant reply text, or "" on failure.
  String chat(const String& userText, const String& provider,
              const String& apiKey, const String& model, const String& botName);

  // Transcribe PCM (16-bit mono @ MIC_SAMPLE_RATE) to text.
  // Uses the Whisper-style endpoint on OpenAI/Groq. For providers without
  // STT (e.g. Gemini path differs), falls back gracefully.
  String speechToText(const int16_t* pcm, size_t samples,
                      const String& provider, const String& apiKey);

 private:
  String buildChatBody(const String& userText, const String& model,
                       const String& botName, bool openAiCompatible);
};
