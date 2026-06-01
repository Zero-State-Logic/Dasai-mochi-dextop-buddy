// ============================================================================
//  portal_page.h  —  The setup page the ESP32 serves from its own WiFi AP.
//
//  Kept as one PROGMEM string so the firmware stays self-contained.
//  Posts to /save with: ssid, pass, provider, model, apikey, botname.
//  Styled to match the GitHub Pages site (warm cream + ink, Mochi vibe).
// ============================================================================
#pragma once
#include <Arduino.h>

static const char PORTAL_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="en"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Dasai Mochi · Setup</title>
<style>
  :root{--ink:#2b2622;--cream:#f4ecdf;--peach:#f3c9a8;--rose:#e89a8b;--moss:#7c8c5a;}
  *{box-sizing:border-box;font-family:ui-rounded,"Segoe UI",system-ui,sans-serif}
  body{margin:0;background:var(--cream);color:var(--ink);
       display:flex;justify-content:center;min-height:100vh}
  .card{width:100%;max-width:430px;padding:26px 22px}
  .mochi{width:96px;height:80px;margin:8px auto 4px;display:block}
  h1{font-size:1.35rem;text-align:center;margin:.2rem 0}
  p.sub{text-align:center;color:#8a7d6e;margin:.1rem 0 1.4rem;font-size:.9rem}
  label{display:block;font-weight:600;font-size:.82rem;margin:14px 0 5px}
  input,select{width:100%;padding:12px 13px;border:2px solid #ddcdb5;border-radius:13px;
    background:#fffaf2;font-size:1rem;color:var(--ink)}
  input:focus,select:focus{outline:none;border-color:var(--rose)}
  .hint{font-size:.74rem;color:#9a8d7e;margin-top:4px}
  button{width:100%;margin-top:22px;padding:14px;border:none;border-radius:14px;
    background:var(--rose);color:#fff;font-size:1.05rem;font-weight:700;cursor:pointer}
  button:active{transform:translateY(1px)}
  .row{display:flex;gap:10px}.row>*{flex:1}
  .key-wrap{position:relative}
  .toggle{position:absolute;right:10px;top:11px;background:none;border:none;
    width:auto;margin:0;padding:4px;color:#9a8d7e;font-size:.8rem;cursor:pointer}
  footer{text-align:center;font-size:.7rem;color:#b3a797;margin-top:18px}
</style></head><body><div class="card">
  <svg class="mochi" viewBox="0 0 120 100">
    <ellipse cx="60" cy="58" rx="48" ry="38" fill="#fff" stroke="#e6d8c2" stroke-width="2"/>
    <circle cx="44" cy="54" r="6" fill="#2b2622"/><circle cx="76" cy="54" r="6" fill="#2b2622"/>
    <circle cx="42" cy="52" r="2" fill="#fff"/><circle cx="74" cy="52" r="2" fill="#fff"/>
    <ellipse cx="34" cy="64" rx="6" ry="4" fill="#f3c9a8" opacity=".8"/>
    <ellipse cx="86" cy="64" rx="6" ry="4" fill="#f3c9a8" opacity=".8"/>
    <path d="M52 64 q8 7 16 0" stroke="#2b2622" stroke-width="2.5" fill="none" stroke-linecap="round"/>
  </svg>
  <h1>Let's wake up Mochi</h1>
  <p class="sub">Connect your bot to WiFi and give it a brain.</p>

  <form action="/save" method="POST">
    <label>WiFi network</label>
    <input name="ssid" placeholder="Your WiFi name" required>

    <label>WiFi password</label>
    <div class="key-wrap">
      <input id="wpw" name="pass" type="password" placeholder="WiFi password">
      <button type="button" class="toggle" onclick="t('wpw',this)">show</button>
    </div>

    <label>AI provider</label>
    <select name="provider" id="prov">
      <option value="groq">Groq — Llama 3.x (fast, free tier)</option>
      <option value="openai">OpenAI — GPT</option>
      <option value="openrouter">OpenRouter — many models</option>
      <option value="together">Together AI — open models</option>
      <option value="gemini">Google Gemini</option>
    </select>
    <div class="hint" id="provhint"></div>

    <label>Model <span style="font-weight:400;color:#9a8d7e">(optional — blank = default)</span></label>
    <input name="model" id="model" placeholder="leave blank for default">

    <label>API key</label>
    <div class="key-wrap">
      <input id="key" name="apikey" type="password" placeholder="paste your key" required>
      <button type="button" class="toggle" onclick="t('key',this)">show</button>
    </div>
    <div class="hint">Your key is stored only on the device, never sent anywhere except your chosen provider.</div>

    <label>Bot name <span style="font-weight:400;color:#9a8d7e">(optional)</span></label>
    <input name="botname" placeholder="Mochi" maxlength="16">

    <button type="submit">Save &amp; Start 🍡</button>
  </form>
  <footer>Dasai Mochi Bot · setup portal</footer>
</div>
<script>
  const defaults={groq:"llama-3.1-8b-instant",openai:"gpt-4o-mini",
    openrouter:"meta-llama/llama-3.1-8b-instruct",
    together:"meta-llama/Llama-3.3-70B-Instruct-Turbo-Free",gemini:"gemini-1.5-flash"};
  const links={groq:"console.groq.com/keys",openai:"platform.openai.com/api-keys",
    openrouter:"openrouter.ai/keys",together:"api.together.xyz/settings/api-keys",
    gemini:"aistudio.google.com/apikey"};
  const prov=document.getElementById('prov'),hint=document.getElementById('provhint'),
        model=document.getElementById('model');
  function refresh(){const p=prov.value;model.placeholder=defaults[p];
    hint.textContent="Get a key at "+links[p];}
  prov.onchange=refresh;refresh();
  function t(id,b){const el=document.getElementById(id);
    const s=el.type==='password';el.type=s?'text':'password';b.textContent=s?'hide':'show';}
</script></body></html>
)HTML";

static const char SAVED_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Saved</title><style>
body{margin:0;background:#f4ecdf;color:#2b2622;font-family:ui-rounded,system-ui,sans-serif;
  display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:100vh;text-align:center;padding:20px}
h1{font-size:1.4rem}p{color:#8a7d6e;max-width:320px}
</style></head><body>
<h1>🍡 Mochi is waking up!</h1>
<p>Settings saved. The bot is restarting and will connect to your WiFi. You can close this page.</p>
</body></html>
)HTML";
