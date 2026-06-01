// ============================================================================
//  faces.cpp  —  implementation of the silly face engine
// ============================================================================
#include "faces.h"
#include "config.h"

void FaceEngine::begin() {
  randomSeed(esp_random());
  nextBlink = millis() + random(1500, 4000);
  nextLook  = millis() + random(2000, 5000);
}

void FaceEngine::setState(FaceState s) {
  if (s != st) { st = s; talkPhase = 0; }
}

void FaceEngine::nextSillyFace() {
  // Rotate through the mood faces for pure entertainment value.
  static const FaceState silly[] = {
    FACE_HAPPY, FACE_SURPRISE, FACE_SAD, FACE_SLEEP, FACE_IDLE
  };
  sillyIndex = (sillyIndex + 1) % (sizeof(silly) / sizeof(silly[0]));
  setState(silly[sillyIndex]);
}

void FaceEngine::update() {
  uint32_t now = millis();
  if (now - lastFrame < IDLE_FRAME_MS) return;
  lastFrame = now;

  // Blink + gaze logic only matters for the "living" faces.
  if (st == FACE_IDLE || st == FACE_HAPPY) {
    if (!blinking && now > nextBlink) { blinking = true; blinkPhase = 0; }
    if (blinking) {
      blinkPhase++;
      if (blinkPhase > 4) { blinking = false; nextBlink = now + random(1500, 4500); }
    }
    if (now > nextLook) {
      gazeX = random(-3, 4);
      gazeY = random(-2, 3);
      nextLook = now + random(1500, 4000);
    }
  } else {
    gazeX = gazeY = 0;
  }

  d.clearDisplay();
  switch (st) {
    case FACE_IDLE:     drawIdle();   break;
    case FACE_LISTEN:   drawListen(); break;
    case FACE_THINK:    drawThink();  break;
    case FACE_TALK:     drawTalk();   break;
    default:            drawMood(st); break;
  }
  d.display();
}

// Two rounded eyes with a movable pupil; h shrinks to 1px when blinking.
void FaceEngine::drawEyes(int cx1, int cx2, int cy, int w, int h,
                          int8_t gx, int8_t gy) {
  int hh = blinking ? max(1, h - blinkPhase * (h / 4)) : h;
  d.fillRoundRect(cx1 - w / 2, cy - hh / 2, w, hh, 4, SSD1306_WHITE);
  d.fillRoundRect(cx2 - w / 2, cy - hh / 2, w, hh, 4, SSD1306_WHITE);
  if (hh > 6) {  // draw pupils only when eyes are open enough
    d.fillRect(cx1 - 3 + gx, cy - 3 + gy, 6, 6, SSD1306_BLACK);
    d.fillRect(cx2 - 3 + gx, cy - 3 + gy, 6, 6, SSD1306_BLACK);
  }
}

void FaceEngine::drawIdle() {
  drawEyes(44, 84, 28, 26, 30, gazeX, gazeY);
  // gentle neutral mouth
  d.drawFastHLine(54, 50, 20, SSD1306_WHITE);
}

void FaceEngine::drawListen() {
  // wide, alert eyes + small "o" mouth = clearly paying attention
  drawEyes(44, 84, 26, 30, 34, 0, 0);
  d.fillCircle(64, 52, 4, SSD1306_WHITE);
  d.fillCircle(64, 52, 2, SSD1306_BLACK);
}

void FaceEngine::drawThink() {
  // eyes glance up, animated "..." mouth
  drawEyes(44, 84, 24, 24, 26, 0, -3);
  uint8_t dots = (millis() / 350) % 4;
  for (uint8_t i = 0; i < dots; ++i)
    d.fillCircle(52 + i * 10, 52, 2, SSD1306_WHITE);
}

void FaceEngine::drawTalk() {
  drawEyes(44, 84, 26, 26, 28, 0, 0);
  // mouth height tracks audio amplitude for lip-sync feel
  int mh = 3 + (mouthAmp * 14) / 255;
  d.fillRoundRect(54, 50 - mh / 2, 20, mh, 3, SSD1306_WHITE);
}

void FaceEngine::drawMood(FaceState s) {
  switch (s) {
    case FACE_HAPPY:
      // ^ ^ eyes + big smile
      d.drawLine(34, 30, 44, 22, SSD1306_WHITE);
      d.drawLine(44, 22, 54, 30, SSD1306_WHITE);
      d.drawLine(74, 30, 84, 22, SSD1306_WHITE);
      d.drawLine(84, 22, 94, 30, SSD1306_WHITE);
      for (int x = 48; x <= 80; ++x)
        d.drawPixel(x, 48 + (int)(8 * sin((x - 48) * PI / 32)), SSD1306_WHITE);
      break;
    case FACE_SAD:
      drawEyes(44, 84, 28, 22, 22, 0, 2);
      for (int x = 48; x <= 80; ++x)
        d.drawPixel(x, 54 - (int)(6 * sin((x - 48) * PI / 32)), SSD1306_WHITE);
      break;
    case FACE_SURPRISE:
      d.drawCircle(44, 28, 13, SSD1306_WHITE);
      d.drawCircle(84, 28, 13, SSD1306_WHITE);
      d.fillCircle(64, 52, 6, SSD1306_WHITE);
      break;
    case FACE_SLEEP: {
      // closed eyes + drifting "z z z"
      d.drawFastHLine(32, 30, 24, SSD1306_WHITE);
      d.drawFastHLine(72, 30, 24, SSD1306_WHITE);
      d.setTextSize(1); d.setTextColor(SSD1306_WHITE);
      uint8_t p = (millis() / 500) % 3;
      d.setCursor(96, 18 - p * 4); d.print("z");
      break;
    }
    default: break;
  }
}
