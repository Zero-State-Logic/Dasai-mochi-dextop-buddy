// ============================================================================
//  faces.h / faces.cpp  —  Silly face engine for the SSD1306 OLED
//
//  Draws expressive vector faces (eyes + mouth) so we are NOT shipping a pile
//  of bitmaps. Each face is parameterised, so blinking, looking around, and
//  mood changes are cheap on a tiny C3.
//
//  States the rest of the firmware drives:
//     FACE_IDLE      - random blinks, occasional look-around (entertainment)
//     FACE_LISTEN    - wide attentive eyes (recording your voice)
//     FACE_THINK     - eyes up, dotted mouth (waiting on the LLM)
//     FACE_TALK      - mouth animates while TTS plays
//     FACE_HAPPY / FACE_SAD / FACE_SURPRISE / FACE_SLEEP  - moods & silliness
// ============================================================================
#pragma once
#include <Adafruit_SSD1306.h>

enum FaceState {
  FACE_IDLE = 0,
  FACE_LISTEN,
  FACE_THINK,
  FACE_TALK,
  FACE_HAPPY,
  FACE_SAD,
  FACE_SURPRISE,
  FACE_SLEEP,
  FACE_COUNT
};

class FaceEngine {
 public:
  explicit FaceEngine(Adafruit_SSD1306& display) : d(display) {}

  void begin();
  void setState(FaceState s);
  FaceState state() const { return st; }

  // Call as often as you like in loop(); it self-throttles on millis().
  void update();

  // While FACE_TALK, feed a 0..255 amplitude so the mouth opens with volume.
  void setMouthOpen(uint8_t amount) { mouthAmp = amount; }

  // Cycle to the next silly idle face (bound to the NEXT touch button).
  void nextSillyFace();

 private:
  Adafruit_SSD1306& d;
  FaceState st = FACE_IDLE;
  uint32_t  lastFrame = 0;
  uint32_t  nextBlink = 0;
  uint32_t  nextLook  = 0;
  int8_t    gazeX = 0, gazeY = 0;   // pupil offset
  bool      blinking = false;
  uint8_t   blinkPhase = 0;
  uint8_t   mouthAmp = 0;
  uint8_t   sillyIndex = 0;
  uint16_t  talkPhase = 0;

  void drawEyes(int cx1, int cx2, int cy, int w, int h, int8_t gx, int8_t gy);
  void drawIdle();
  void drawListen();
  void drawThink();
  void drawTalk();
  void drawMood(FaceState s);
};
