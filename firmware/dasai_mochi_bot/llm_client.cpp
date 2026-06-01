// ============================================================================
//  llm_client.cpp
// ============================================================================
#include "llm_client.h"
#include "providers.h"
#include "config.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// We skip cert pinning to keep first-run setup painless. For a hardened
// build, replace setInsecure() with a root CA bundle.
static void makeClient(WiFiClientSecure& c) { c.setInsecure(); }

void LlmClient::begin() {}

// --- system prompt: keep replies short + spoken ----------------------------
static String systemPrompt(const String& botName) {
  return "You are " + botName + ", a friendly pocket robot. "
         "Answer in 1-2 short spoken sentences, warm and a little playful. "
         "You can tell the time, the weather, the news, and answer questions. "
         "Never use markdown, lists, or emoji in your reply.";
}

String LlmClient::buildChatBody(const String& userText, const String& model,
                                const String& botName, bool /*compat*/) {
  JsonDocument doc;
  doc["model"] = model;
  doc["max_tokens"] = 160;
  doc["temperature"] = 0.7;
  JsonArray msgs = doc["messages"].to<JsonArray>();
  JsonObject sys = msgs.add<JsonObject>();
  sys["role"] = "system";  sys["content"] = systemPrompt(botName);
  JsonObject usr = msgs.add<JsonObject>();
  usr["role"] = "user";    usr["content"] = userText;
  String out; serializeJson(doc, out); return out;
}

String LlmClient::chat(const String& userText, const String& provider,
                       const String& apiKey, const String& model,
                       const String& botName) {
  const ProviderInfo* p = providerById(provider);

  WiFiClientSecure client; makeClient(client);
  HTTPClient http;

  String url, body;
  if (p->openAiCompatible) {
    url  = String("https://") + p->host + p->path;
    body = buildChatBody(userText, model, botName, true);
  } else {  // ---- Gemini special case ----
    url = String("https://") + p->host + p->path + "/" + model +
          ":generateContent?key=" + apiKey;
    JsonDocument doc;
    JsonArray contents = doc["contents"].to<JsonArray>();
    JsonObject c = contents.add<JsonObject>();
    c["role"] = "user";
    c["parts"][0]["text"] = systemPrompt(botName) + "\n\nUser: " + userText;
    serializeJson(doc, body);
  }

  if (!http.begin(client, url)) return "";
  http.addHeader("Content-Type", "application/json");
  if (p->openAiCompatible)
    http.addHeader("Authorization", String("Bearer ") + apiKey);
  if (provider == "openrouter")             // OpenRouter likes a referer
    http.addHeader("HTTP-Referer", "https://github.com");

  int code = http.POST(body);
  String reply;
  if (code == 200) {
    JsonDocument res;
    if (!deserializeJson(res, http.getStream())) {
      if (p->openAiCompatible)
        reply = res["choices"][0]["message"]["content"].as<String>();
      else
        reply = res["candidates"][0]["content"]["parts"][0]["text"].as<String>();
    }
  } else {
    Serial.printf("chat HTTP %d\n", code);
  }
  http.end();
  reply.trim();
  return reply;
}

// --- minimal WAV header so the STT endpoint accepts our PCM ----------------
static void writeWavHeader(uint8_t* h, uint32_t dataLen, uint32_t rate) {
  uint32_t chunk = 36 + dataLen, byteRate = rate * 2;
  memcpy(h, "RIFF", 4);                 memcpy(h + 8, "WAVE", 4);
  memcpy(h + 12, "fmt ", 4);            memcpy(h + 36, "data", 4);
  h[4]=chunk; h[5]=chunk>>8; h[6]=chunk>>16; h[7]=chunk>>24;
  h[16]=16; h[20]=1; h[22]=1;            // PCM, mono
  h[24]=rate; h[25]=rate>>8; h[26]=rate>>16; h[27]=rate>>24;
  h[28]=byteRate; h[29]=byteRate>>8; h[30]=byteRate>>16; h[31]=byteRate>>24;
  h[32]=2; h[34]=16;                     // block align, bits
  h[40]=dataLen; h[41]=dataLen>>8; h[42]=dataLen>>16; h[43]=dataLen>>24;
}

String LlmClient::speechToText(const int16_t* pcm, size_t samples,
                               const String& provider, const String& apiKey) {
  // Whisper-style STT is available on OpenAI and Groq. For other providers
  // we route STT through Groq if the user pasted a Groq key, else OpenAI.
  const char* host; const char* path; const char* model;
  if (provider == "groq") {
    host = "api.groq.com"; path = "/openai/v1/audio/transcriptions";
    model = "whisper-large-v3-turbo";
  } else {
    host = "api.openai.com"; path = "/v1/audio/transcriptions";
    model = "whisper-1";
  }

  uint32_t dataLen = samples * 2;
  uint8_t header[44]; writeWavHeader(header, dataLen, MIC_SAMPLE_RATE);

  // multipart/form-data assembled in chunks to avoid a giant String.
  String boundary = "----mochi" + String(millis());
  String pre =
    "--" + boundary + "\r\nContent-Disposition: form-data; name=\"model\"\r\n\r\n"
    + model + "\r\n--" + boundary +
    "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"a.wav\"\r\n"
    "Content-Type: audio/wav\r\n\r\n";
  String post = "\r\n--" + boundary + "--\r\n";

  WiFiClientSecure client; makeClient(client);
  if (!client.connect(host, 443)) return "";
  uint32_t total = pre.length() + 44 + dataLen + post.length();

  client.printf("POST %s HTTP/1.1\r\n", path);
  client.printf("Host: %s\r\n", host);
  client.printf("Authorization: Bearer %s\r\n", apiKey.c_str());
  client.printf("Content-Type: multipart/form-data; boundary=%s\r\n", boundary.c_str());
  client.printf("Content-Length: %u\r\nConnection: close\r\n\r\n", total);

  client.print(pre);
  client.write(header, 44);
  client.write((const uint8_t*)pcm, dataLen);
  client.print(post);

  // read response, find body, parse JSON {"text": "..."}
  String line, body; bool inBody = false; uint32_t t0 = millis();
  while (client.connected() && millis() - t0 < 15000) {
    while (client.available()) {
      char ch = client.read();
      if (!inBody) { line += ch; if (line.endsWith("\r\n\r\n")) inBody = true; }
      else body += ch;
    }
  }
  client.stop();
  JsonDocument res; String text;
  if (!deserializeJson(res, body)) text = res["text"].as<String>();
  text.trim();
  return text;
}
