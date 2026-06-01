// ============================================================================
//  providers.h  —  Multi-provider LLM abstraction
//
//  The user picks ONE provider in the captive portal and pastes their own key.
//  We keep the differences (host, path, auth header, JSON body shape) in one
//  small table so adding a provider later is a couple of lines.
//
//  Supported out of the box (all are OpenAI-Chat-Completions compatible,
//  which keeps the request/response parsing identical):
//     - OpenAI        (GPT models)
//     - Groq          (Llama 3.x, very fast, generous free tier)
//     - OpenRouter    (proxy to many models incl. Llama/Claude/Gemini)
//     - Together AI   (open models)
//
//  Gemini uses a different shape, so it is handled as a special case in
//  llm_client.cpp rather than this table.
// ============================================================================
#pragma once
#include <Arduino.h>

enum LlmProvider {
  PROV_OPENAI = 0,
  PROV_GROQ,
  PROV_OPENROUTER,
  PROV_TOGETHER,
  PROV_GEMINI,
  PROV_COUNT
};

struct ProviderInfo {
  const char* id;          // short id stored in NVS
  const char* label;       // shown in the web UI
  const char* host;        // TLS host
  const char* path;        // chat completions path
  const char* defaultModel;
  bool        openAiCompatible;
};

// NOTE: kept in PROGMEM-friendly static table.
static const ProviderInfo PROVIDERS[] = {
  { "openai",     "OpenAI (GPT)",        "api.openai.com",       "/v1/chat/completions",
    "gpt-4o-mini",                                  true  },
  { "groq",       "Groq (Llama 3.x)",    "api.groq.com",         "/openai/v1/chat/completions",
    "llama-3.1-8b-instant",                         true  },
  { "openrouter", "OpenRouter",          "openrouter.ai",        "/api/v1/chat/completions",
    "meta-llama/llama-3.1-8b-instruct",             true  },
  { "together",   "Together AI",         "api.together.xyz",     "/v1/chat/completions",
    "meta-llama/Llama-3.3-70B-Instruct-Turbo-Free", true  },
  { "gemini",     "Google Gemini",       "generativelanguage.googleapis.com",
    "/v1beta/models",  "gemini-1.5-flash",          false },
};

inline const ProviderInfo* providerById(const String& id) {
  for (int i = 0; i < PROV_COUNT; ++i) {
    if (id == PROVIDERS[i].id) return &PROVIDERS[i];
  }
  return &PROVIDERS[PROV_OPENAI];  // safe default
}
