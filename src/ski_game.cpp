// Ski-Game inspiration from On-Going Winter Olympics of 2026, Game 3
// Graphics-Heavy
// Uses most of the Ram of Arduino Uno, hence played as a separate file for the Game-Boy inspired project.

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI 11
#define OLED_CLK  13
#define OLED_DC   7
#define OLED_CS   6
#define OLED_RESET 8

// #define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// Included for the high score saving to work
#include <EEPROM.h>

const int trigPin    = 9;
const int echoPin    = 3;
const int speakerPin = 4;

const float F           = 60.0;
const float CAM_H       = 8.0;
const int   HORIZON     = 18;
const int   SCR_CX      = 64;
const float ROAD_HALF   = 22.0;
const float SENSOR_NEAR = 5.0;
const float SENSOR_FAR  = 40.0;
const float MIN_SEP     = 28.0;
const float BASE_GAP    = 8.0;
const float MIN_GAP     = 4.5;
const float BASE_SPEED  = 1.2;
const float SPEED_CAP   = 3.0;
const int   NUM_GATES   = 3;
const int   EEPROM_ADDR = 0;
const float COLLISION_Z = 15.0;

// --- PROCEDURAL CURVE STATE ---
// Instead of a fixed sine, a random-walk target is nudged each frame
// and the actual curve smoothly follows it.
float curveTarget  = 0.0;   // where the curve wants to be
float curveVal     = 0.0;   // actual smoothed curve (used everywhere)
const float CURVE_MAX      = 7.0;   // max lateral world units
const float CURVE_NUDGE    = 0.04;  // random walk step per frame
const float CURVE_SMOOTH   = 0.015; // how fast curveVal follows curveTarget

struct Gate { float x, z, gap; bool scored, leftSolid; };
Gate gates[NUM_GATES];

float speed       = BASE_SPEED;
float globalZ     = 0;
float trackOffset = 0.0;
float smoothDist  = 22.0;
int   score       = 0;
int   hiScore     = 0;
int   lives       = 3;
float lastGateX   = 0;

int  gameState     = 0;
int  countdown     = 3;
int  countdownTimer= 0;
int  frameCount    = 0;  // used for speed line alternation

// --- MOUNTAIN PEAKS (PROGMEM bitmap) ---
// 128 wide, 10 tall, stored as 10 rows of 16 bytes = 128 bits per row
// Hand-crafted jagged silhouette — peaks at roughly x=20,45,70,95,110
const uint8_t PROGMEM mountainBmp[] = {
  // Row 0 — all clear
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  // Row 1 — peak tips: x≈52, x≈80, x≈108
  0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x04,0x00,
  // Row 2 — peaks widen
  0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0x00,0x38,0x00,0x00,0x0E,0x00,
  // Row 3
  0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x1F,0x00,
  // Row 4 — secondary peak at x≈62 starts
  0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x02,0x00,0xFE,0x00,0x00,0x3F,0x80,
  // Row 5
  0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x80,0x00,0x07,0x01,0xFF,0x00,0x00,0xFF,0xC0,
  // Row 6 — left region now starts rising from x≈42
  0x00,0x00,0x00,0x00,0x03,0x81,0xFF,0xC0,0x00,0x0F,0x83,0xFF,0x80,0x01,0xFF,0xE0,
  // Row 7
  0x00,0x00,0x00,0x00,0x07,0xC3,0xFF,0xE0,0x00,0x3F,0xC7,0xFF,0xC0,0x07,0xFF,0xF0,
  // Row 8
  0x00,0x00,0x00,0x00,0x1F,0xFF,0xFF,0xF8,0x00,0xFF,0xFF,0xFF,0xF0,0x1F,0xFF,0xF8,
  // Row 9 — base fills right two thirds
  0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

// --- PARALLAX WORLD POSITIONS ---
const float FAR_WX[12]  = {-90,-75,-55,-35,-15,5,20,38,52,65,78,88};
const float FAR_PH[12]  = {0,2.1,4.7,1.3,6.2,3.8,5.5,0.9,2.6,4.1,1.8,3.3};
const float MID_WX[8]   = {-30,-24,-32,-26,26,30,24,32};
const float MID_PH[8]   = {0,3.5,7.1,1.9,5.3,2.8,8.4,4.6};
const float NEAR_WX[10] = {-16,-10,-6,-2,2,6,10,14,-14,-8};
const float NEAR_PH[10] = {0,1.1,2.3,4.7,3.2,5.8,0.7,2.9,6.1,1.5};

// ------------------------------------------------
float getDistance() {
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long d = pulseIn(echoPin, HIGH, 25000);
  return (d == 0) ? 100.0 : d * 0.034 / 2.0;
}

// Simple Mii-style bassline
// const int wiiNotes[] = {330, 0, 415, 0, 494, 0, 440, 0, 330, 0, 247, 0}; 
// const int wiiTempos[] = {150, 50, 150, 50, 150, 50, 150, 50, 150, 50, 150, 50};
// int wiiIdx = 0;
// unsigned long lastNoteTime = 0;

// --- WII MUSIC DATA ---
// const int wiiMelody[] = {
//   392, 523, 659, 784, 0, 523, 0, 392, 0, 330, 0,  // Intro
//   440, 494, 523, 494, 440, 392, 0,                // Phrase
//   330, 0, 262, 0                                  // End
// };

// const int wiiRhythm[] = {
//   150, 150, 150, 300, 100, 150, 50, 150, 50, 300, 100,
//   150, 150, 150, 150, 150, 300, 100,
//   250, 100, 250, 100
// };

// int wiiIdx = 0;
// unsigned long wiiPrevTime = 0;


// void playWiiMusic() {
//   if (millis() - wiiPrevTime >= wiiRhythm[wiiIdx]) {
//     wiiPrevTime = millis();
    
//     int note = wiiMelody[wiiIdx];
//     if (note > 0) {
//       tone(speakerPin, note, wiiRhythm[wiiIdx]);
//     } else {
//       noTone(speakerPin); // The "0" in melody creates the silent gaps
//     }

//     wiiIdx = (wiiIdx + 1) % 23; // Loop the 23-element array
//   }
// }

// void playWiiMusic() {
//   // Play note for 80% of the rhythm time for that "bouncy" effect
//   int noteDuration = wiiRhythm[wiiIdx] * 0.8; 

//   if (millis() - wiiPrevTime >= wiiRhythm[wiiIdx]) {
//     wiiPrevTime = millis();
    
//     int note = wiiMelody[wiiIdx];
//     if (note > 0) {
//       tone(speakerPin, note, noteDuration);
//     } else {
//       noTone(speakerPin);
//     }

//     wiiIdx = (wiiIdx + 1) % 21; // Loop the 21 notes
//   }
// }

// --- WII MUSIC DATA ---
// Simplified Mii Channel style melody for Arduino UNO buzzer

const int wiiMelody[] = {
  659, 784, 988, 784,
  659, 523, 659, 784,
  587, 659, 784, 988,
  784, 659, 523, 494,
  523, 587, 659, 523,
  440, 392
};

const int wiiRhythm[] = {
  140, 140, 140, 220,
  140, 140, 140, 220,
  140, 140, 140, 220,
  140, 140, 140, 220,
  140, 140, 140, 220,
  220, 260
};

const int WII_NOTE_COUNT = sizeof(wiiMelody) / sizeof(wiiMelody[0]);

int wiiIdx = 0;
unsigned long wiiPrevTime = 0;

void playWiiMusic() {
  if (millis() - wiiPrevTime >= (unsigned long)wiiRhythm[wiiIdx]) {
    wiiPrevTime = millis();

    int note = wiiMelody[wiiIdx];
    int noteDuration = (int)(wiiRhythm[wiiIdx] * 0.8);

    if (note > 0) {
      tone(speakerPin, note, noteDuration);
    } else {
      noTone(speakerPin);
    }

    wiiIdx++;
    if (wiiIdx >= WII_NOTE_COUNT) wiiIdx = 0;
  }
}

// curveOffset: at near Z the full curveVal applies,
// at far Z it tapers to zero — perspective-correct lateral bend
float curveOffset(float Z) {
  return curveVal * (1.0f / max(1.0f, Z * 0.18f));
}

bool project(float wx, float Z, int &sx, int &sy) {
  if (Z < 0.5) return false;
  float curve = curveOffset(Z);
  sx = SCR_CX + (int)(((wx + trackOffset + curve) / Z) * F);
  sy = HORIZON + (int)((CAM_H / Z) * F);
  return (sy >= HORIZON && sy < SCREEN_HEIGHT);
}

float calcGapAtScore(int s) { return max(MIN_GAP, BASE_GAP - s * 0.12f); }

float furthestZ() {
  float fz = 0;
  for (int i = 0; i < NUM_GATES; i++) fz = max(fz, gates[i].z);
  return fz;
}

float safeX(float bias) {
  for (int attempt = 0; attempt < 20; attempt++) {
    float x = bias + random(-12, 12);
    x = constrain(x, -15, 15);
    bool ok = true;
    for (int i = 0; i < NUM_GATES; i++)
      if (abs(gates[i].x - x) < 9) { ok = false; break; }
    if (ok) return x;
  }
  return constrain(bias, -15, 15);
}

void spawnGate(int i, float zStart) {
  float bias = (lastGateX > 0) ? -7 : 7;
  float x    = safeX(bias);
  lastGateX  = x;
  gates[i]   = { x, zStart, calcGapAtScore(score), false, x < 0 };
}

bool nearGateY(int testY) {
  for (int i = 0; i < NUM_GATES; i++) {
    int gsx, gsy;
    if (project(gates[i].x, gates[i].z, gsx, gsy))
      if (abs(testY - gsy) <= 4) return true;
  }
  return false;
}

void drawGate(int i) {
  float gh  = gates[i].gap;
  float Z   = gates[i].z;

  int lsx, lsy, rsx, rsy;
  if (!project(gates[i].x - gh, Z, lsx, lsy)) return;
  if (!project(gates[i].x + gh, Z, rsx, rsy)) return;

  // --- depth-scaled dimensions ---
  // poleH: already correct
  int poleH = max(2, (int)((CAM_H / Z) * F * 2.5));

  // poleW: 1px at Z>30, 2px at Z>12, 3px at Z<6
  int poleW = (Z < 6.0f) ? 3 : (Z < 12.0f) ? 2 : 1;

  // flagH: scales with depth, same ratio as flagW
  int fw    = max(3, min(7, 3 + (int)(3.5f / Z)));   // wider range than before
  int fh    = max(2, min(5, 2 + (int)(2.5f / Z)));   // height now also scales

  // clearW: tight at far, generous at near — preserves snow texture at distance
  // At Z>25: no clear (let snow show through — gives atmosphere)
  // At Z 10-25: narrow clear
  // At Z<10: full clear
  int clearW = (Z > 25.0f) ? 0 : (Z > 10.0f) ? 1 : max(2, 1 + (int)(3.0f / Z));

  // clear behind poles
  if (clearW > 0) {
    for (int py = lsy - fh - 1; py < lsy + poleH + 2; py++) {
      for (int cx = lsx - clearW; cx <= lsx + clearW; cx++)
        display.drawPixel(cx, py, BLACK);
      for (int cx = rsx - clearW; cx <= rsx + clearW; cx++)
        display.drawPixel(cx, py, BLACK);
    }
  }

  // --- poles: draw with depth-scaled thickness ---
  for (int pw = 0; pw < poleW; pw++) {
    display.drawFastVLine(lsx + pw - poleW/2, lsy, poleH, WHITE);
    display.drawFastVLine(rsx + pw - poleW/2, rsy, poleH, WHITE);
  }

  // --- flags: scaled height AND width ---
  if (gates[i].leftSolid) {
    display.fillRect(lsx - 1,       lsy - fh, fw, fh, WHITE);
    // hollow right flag: draw outline only
    display.drawRect(rsx - fw + 1,  rsy - fh, fw, fh, WHITE);
  } else {
    display.drawRect(lsx - 1,       lsy - fh, fw, fh, WHITE);
    display.fillRect(rsx - fw + 1,  rsy - fh, fw, fh, WHITE);
  }

  // --- crossbar: filled rect when near, single line when far ---
  int crossY = min(lsy, rsy) - 1;
  if (Z < 8.0f) {
    // thick filled crossbar up close
    display.fillRect(lsx + fw - 1, crossY - 1, rsx - lsx - fw*2 + 2, 2, WHITE);
  } else {
    display.drawLine(lsx + fw - 1, crossY, rsx - fw + 1, crossY, WHITE);
  }

  // --- base line + shadow for depth grounding ---
  int baseY = lsy + poleH;
  if (baseY < SCREEN_HEIGHT) {
    // main base
    display.drawLine(lsx, baseY, rsx, baseY, WHITE);
    // shadow: offset 1px down and 1px right — only visible when not too far
    if (Z < 20.0f && baseY + 1 < SCREEN_HEIGHT) {
      // sparse shadow: every other pixel so it reads as shade not solid line
      for (int sx = lsx + 1; sx <= rsx + 1; sx += 2)
        display.drawPixel(sx, baseY + 1, WHITE);
    }
  }
}

void drawSkier() {
  int lean = (int)(trackOffset * 0.15);

  // SHADOW: flattened ellipse on the snow, 3px below skier base
  // Shifts with lean to stay under the skis
  int shadowY = SCREEN_HEIGHT - 1;
  int shadowCX = SCR_CX + lean / 2;
  for (int dx = -5; dx <= 5; dx++) {
    // ellipse: only draw if within vertical radius 1
    if (dx * dx <= 25) {
      display.drawPixel(shadowCX + dx, shadowY,     WHITE);
      if (abs(dx) <= 3) display.drawPixel(shadowCX + dx, shadowY - 1, WHITE);
    }
  }

  // body
  display.fillRect(SCR_CX - 2, SCREEN_HEIGHT - 14, 4, 5, WHITE);
  // head
  display.fillRect(SCR_CX - 1, SCREEN_HEIGHT - 17, 3, 3, WHITE);
  // skis
  display.drawLine(SCR_CX - 6, SCREEN_HEIGHT - 1, SCR_CX - 1 + lean, SCREEN_HEIGHT - 5, WHITE);
  display.drawLine(SCR_CX + 6, SCREEN_HEIGHT - 1, SCR_CX + 1 + lean, SCREEN_HEIGHT - 5, WHITE);
  // poles
  display.drawLine(SCR_CX - 3, SCREEN_HEIGHT - 12, SCR_CX - 6, SCREEN_HEIGHT - 7, WHITE);
  display.drawLine(SCR_CX + 3, SCREEN_HEIGHT - 12, SCR_CX + 6, SCREEN_HEIGHT - 7, WHITE);
}

// SPEED LINES: radial streaks from vanishing point, only above score threshold
// Lines alternate each frame for a strobing motion feel without flicker
void drawSpeedLines() {
  if (speed < 2.0) return;  // only appear when genuinely fast

  // intensity: 0.0 at speed=2.0, 1.0 at SPEED_CAP
  float intensity = (speed - 2.0f) / (SPEED_CAP - 2.0f);
  // length scales with intensity: 4–18px
  int lineLen = 4 + (int)(intensity * 14.0f);

  // 6 radial directions spread around the vanishing point
  // alternate odd/even sets each frame so they strobe rather than persist
  // direction pairs: (dx,dy) normalised — pointing outward from horizon centre
  const int8_t dirs[6][2] = {
    {-3, 2}, {-2, 3}, {-1, 4},
    { 3, 2}, { 2, 3}, { 1, 4}
  };
  int startDir = (frameCount % 2 == 0) ? 0 : 3;  // alternate left/right set
  for (int d = startDir; d < startDir + 3; d++) {
    float nx = dirs[d][0], ny = dirs[d][1];
    float mag = sqrt(nx*nx + ny*ny);
    nx /= mag; ny /= mag;
    // start near vanishing point, offset slightly so they don't overlap skier
    float startDist = 6.0 + intensity * 4.0;
    int x0 = SCR_CX + (int)(nx * startDist);
    int y0 = HORIZON + (int)(ny * startDist);
    int x1 = SCR_CX + (int)(nx * (startDist + lineLen));
    int y1 = HORIZON + (int)(ny * (startDist + lineLen));
    // only draw if entirely on screen and below horizon
    if (y0 >= HORIZON && y1 < SCREEN_HEIGHT && x0 >= 0 && x1 < SCREEN_WIDTH
     && x0 < SCREEN_WIDTH && x1 >= 0)
      display.drawLine(x0, y0, x1, y1, WHITE);
  }
}

// FAR LAYER: horizon marks (unchanged)
void drawFarLayer() {
  for (int i = 0; i < 12; i++) {
    int sx, sy;
    if (!project(FAR_WX[i], 75, sx, sy)) continue;
    int h = 1 + (int)(fabs(sin(FAR_PH[i])) * 2);
    for (int dy = 0; dy < h; dy++)
      display.drawPixel(sx, HORIZON - 2 - dy, WHITE);
  }
}

// MID LAYER: trees (unchanged)
void drawMidLayer() {
  for (int i = 0; i < 8; i++) {
    float Z = 18.0 + fmod(MID_PH[i] * 4.3, 24.0);
    int sx, sy;
    if (!project(MID_WX[i], Z, sx, sy)) continue;
    int sz = max(2, (int)((CAM_H / Z) * F * 0.55));
    display.drawFastVLine(sx, sy - sz, sz, WHITE);
    for (int row = 0; row < sz - 1; row++) {
      int hw = (int)((float)row / (sz - 1) * sz * 0.7);
      display.drawLine(sx - hw, sy - sz * 2 + row,
                       sx + hw, sy - sz * 2 + row, WHITE);
    }
  }
}

// NEAR LAYER: snow spray (unchanged)
void drawNearLayer() {
  for (int i = 0; i < 10; i++) {
    for (int k = 0; k < 3; k++) {
      float Z = 3.5 + fmod(globalZ * 0.7 + NEAR_PH[i] * 3.0 + k * 2.5, 8.0);
      if (Z < 1.0) continue;
      int sx, sy;
      if (!project(NEAR_WX[i], Z, sx, sy)) continue;
      display.drawPixel(sx, sy, WHITE);
      if (k == 0) display.drawPixel(sx + 1, sy, WHITE);
    }
  }
}

void drawScene() {
  display.clearDisplay();

  // 1. BACKGROUND: Draw mountains first
  display.drawBitmap(0, 0, mountainBmp, 128, 10, WHITE);

  // 2. UI MASKS: "Black out" the areas where text will go
  // Top Left Mask (Covers Score and High Score)
  display.fillRect(0, 0, 40, 17, BLACK); 
  // Top Right Mask (Covers the 3 Lives dots)
  display.fillRect(108, 0, 20, 8, BLACK);

  // 3. ENVIRONMENT: Sky horizon bands
  for (int x = 0; x < SCREEN_WIDTH; x += 3) display.drawPixel(x, HORIZON-3, WHITE);
  for (int x = 0; x < SCREEN_WIDTH; x += 2) display.drawPixel(x, HORIZON-2, WHITE);
  for (int x = 0; x < SCREEN_WIDTH; x += 1) display.drawPixel(x, HORIZON-1, WHITE);

  // 4. SNOW SURFACE & TRACK
  for (int y = HORIZON; y < SCREEN_HEIGHT - 16; y++) {
    int dy = y - HORIZON;
    if (dy < 1) continue;
    float Z    = (CAM_H * F) / dy;
    int period = (int)constrain(Z / 4.0f, 4, 12);
    float curve = curveOffset(Z);
    int lEdge  = SCR_CX + (int)((-ROAD_HALF + trackOffset + curve) / Z * F);
    int rEdge  = SCR_CX + (int)(( ROAD_HALF + trackOffset + curve) / Z * F);
    for (int x = lEdge; x <= rEdge; x++)
      if ((x + y) % period == 0) display.drawPixel(x, y, WHITE);
  }

  // Draw track edges
  {
    int prevLX = -1, prevLY = -1, prevRX = -1, prevRY = -1;
    for (int step = 0; step <= 20; step++) {
      float t = (float)step / 20.0;
      float Z = 2.0 + 78.0 * t * t;
      int lsx, lsy, rsx, rsy;
      bool lv = project(-ROAD_HALF, Z, lsx, lsy);
      bool rv = project( ROAD_HALF, Z, rsx, rsy);
      if (lv && prevLX >= 0) display.drawLine(prevLX, prevLY, lsx, lsy, WHITE);
      if (rv && prevRX >= 0) display.drawLine(prevRX, prevRY, rsx, rsy, WHITE);
      prevLX = lv ? lsx : -1;  prevLY = lv ? lsy : -1;
      prevRX = rv ? rsx : -1;  prevRY = rv ? rsy : -1;
    }
  }

  // Centre dashes
  for (int i = 1; i < 7; i++) {
    float z = i * 6 + fmod(globalZ, 6);
    int cx, cy, ax, ay, bx, by;
    if (!project(0, z, cx, cy) || nearGateY(cy)) continue;
    float dh = min(4.0f, ROAD_HALF * 0.3f);
    if (project(-dh, z, ax, ay) && project(dh, z, bx, by))
      display.drawLine(ax, ay, bx, by, WHITE);
  }

  // 5. OBJECTS: Parallax layers, Speed lines, and Gates
  drawFarLayer();
  drawMidLayer();
  drawNearLayer();
  drawSpeedLines();

  int order[NUM_GATES] = {0, 1, 2};
  for (int a = 0; a < NUM_GATES-1; a++)
    for (int b = a+1; b < NUM_GATES; b++)
      if (gates[order[a]].z < gates[order[b]].z) {
        int tmp = order[a]; order[a] = order[b]; order[b] = tmp;
      }
  for (int i = 0; i < NUM_GATES; i++) drawGate(order[i]);

  // 6. PLAYER
  drawSkier();

  // 7. UI TEXT (Drawn last on top of the black masks)
  display.setTextSize(1); display.setTextColor(WHITE);
  display.setCursor(0, 0);  display.print(F("SCORE:")); display.print(score);
  display.setCursor(0, 9);  display.print(F("HIGH:")); display.print(hiScore);
  for (int i = 0; i < lives; i++)
    display.fillRect(SCREEN_WIDTH - 4 - i*6, 3, 3, 3, WHITE);
}

void drawReady() {
  display.clearDisplay();
  display.drawBitmap(0, 0, mountainBmp, 128, 10, WHITE);
  display.setTextSize(1); display.setTextColor(WHITE);
  display.setCursor(22, 12); display.print(F("SKI SLALOM"));
  display.setTextSize(2);
  display.setCursor(countdown > 0 ? 58 : 50, 30);
  if (countdown > 0) display.print(countdown); else display.print(F("GO"));
  display.setTextSize(1);
  if (hiScore > 0) { display.setCursor(40, 54); display.print(F("HIGH:")); display.print(hiScore); }
  display.display();
}

void drawDead() {
  noTone(speakerPin);
  display.clearDisplay(); 
  display.setTextSize(1); display.setTextColor(WHITE);
  // display.display();
  display.setCursor(38, 10); display.print(F("MISSED!"));
  display.setCursor(30, 25); display.print(F("LIVES: ")); display.print(lives);
  display.setCursor(30, 38); display.print(F("SCORE: ")); display.print(score);
  display.setCursor(16, 54); display.print(F("Wave to restart"));
  // display.clearDisplay(); 
  display.setTextSize(1); display.setTextColor(WHITE);
  display.display();
}

void drawGameOver() {
  noTone(speakerPin);
  wiiIdx = 0; // Reset music to the beginning for next time
  // display.clearDisplay();
  display.drawBitmap(0, 0, mountainBmp, 128, 10, WHITE);
  display.setTextSize(1); display.setTextColor(WHITE);
  display.setCursor(28, 12); display.print(F("GAME OVER"));
  display.setCursor(28, 26); display.print(F("SCORE: ")); display.print(score);
  display.setCursor(28, 38); display.print(F("BEST:  ")); display.print(hiScore);
  display.setCursor(16, 54); display.print(F("Wave to restart"));
  display.display();
}

void startRound() {
  wiiIdx = 0;
  // lastNoteTime = millis();
  speed = BASE_SPEED; lastGateX = 0;
  for (int i = 0; i < NUM_GATES; i++)
    gates[i] = { (float)random(-15,15), 40.0f + i*MIN_SEP,
                 calcGapAtScore(score), false, (bool)(i%2==0) };
  gameState = 0; countdown = 3; countdownTimer = 0;
}

void setup() {
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  pinMode(speakerPin, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC)){
    for(;;);
  }

  //display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay(); 
  display.display();
  randomSeed(analogRead(A1));
  EEPROM.get(EEPROM_ADDR, hiScore);
  if (hiScore < 0 || hiScore > 9999) hiScore = 0;
  lives = 3; score = 0;
  curveVal = 0.0; curveTarget = 0.0;
  startRound();
}

void loop() {
  frameCount++;

  float raw = getDistance();
  if (raw > 2 && raw < 60)
    smoothDist = smoothDist * 0.85 + raw * 0.15;

  // PROCEDURAL CURVE: random walk with smooth follow
  // Nudge target by a small random step, keep within ±CURVE_MAX
  curveTarget += ((float)random(-100, 101) / 100.0f) * CURVE_NUDGE;
  curveTarget  = constrain(curveTarget, -CURVE_MAX, CURVE_MAX);
  // Smooth curveVal toward target — feels like genuine track bending
  curveVal += (curveTarget - curveVal) * CURVE_SMOOTH;

  // Curve-aware sensor mapping: sensor range always spans full ±ROAD_HALF
  // regardless of which way the curve is currently bending
  float t = constrain(
    (smoothDist - SENSOR_NEAR) / (SENSOR_FAR - SENSOR_NEAR), 0.0f, 1.0f);
  // curveVal at Z≈0 (player position) equals curveVal * (1/max(1,0)) = curveVal
  // so we subtract it from both ends of the reachable range
  float curveAtPlayer = curveVal;
  trackOffset = (ROAD_HALF - curveAtPlayer) - t * 2.0f * (ROAD_HALF - fabsf(curveAtPlayer));

  if (gameState == 0) {
    noTone(speakerPin);
    wiiIdx = 0;
    countdownTimer++;
    if (countdownTimer > 40) { countdownTimer = 0; countdown--; }
    if (countdown < 0) gameState = 1;
    drawReady(); delay(20); return;
  }
  // ... inside the case for the Ski Game ...
  if (gameState == 2) {
    playWiiMusic();
    drawDead(); 
    delay(20);
    if (++countdownTimer > 80) { 
      if (lives > 0) startRound(); 
    else gameState = 3; 
    }
    return;
  }
  if (gameState == 3) {
    drawGameOver();
    if (raw > 2 && raw < 10) { score = 0; lives = 3; startRound(); }
    delay(20); return;
    noTone(speakerPin);
    wiiIdx= 0;
  }

  // PLAYING
  speed = min(SPEED_CAP, BASE_SPEED + score * 0.06f);
  globalZ += speed;

  playWiiMusic();

  int order[NUM_GATES] = {0, 1, 2};
  for (int a = 0; a < NUM_GATES-1; a++)
    for (int b = a+1; b < NUM_GATES; b++)
      if (gates[order[a]].z < gates[order[b]].z) {
        int tmp = order[a]; order[a] = order[b]; order[b] = tmp;
      }

  for (int i = 0; i < NUM_GATES; i++) {
    float oldZ = gates[i].z;
    gates[i].z -= speed;
    if (!gates[i].scored && oldZ >= COLLISION_Z && gates[i].z < COLLISION_Z) {
      gates[i].scored = true;
      float skierWorldX = -trackOffset;
      if (fabsf(skierWorldX - gates[i].x) <= (gates[i].gap + 1.0f)) {
        score++;
        if (score > hiScore) { hiScore = score; 
        EEPROM.put(EEPROM_ADDR, hiScore); 
        }
        tone(speakerPin, 900, 50);
      } else {
        lives--;
        tone(speakerPin, 200, 300);
        countdownTimer = 0;
        gameState = (lives <= 0) ? 3 : 2;
      }
    }
    if (gates[i].z < 1.0)
      spawnGate(i, max(80.0f, furthestZ() + MIN_SEP + random(0, 12)));
  }

  drawScene();
  display.display();
  delay(20);
}
