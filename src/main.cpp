//This file includes the following games & modes
//      case 0: runCalibration(); break; // New Calibration State for the Flex-Sensor to have User-wise bespoke values
//      case 1: showMenu(); break;       // choose using Potentiometer
//      case 2: runFlappyGame(); break;         // Game 1
//      case 3: runToneMatchGame(); break;      // Game 2

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// --- ANIMATION DATA ---
#define FRAME_DELAY (42)
#define FRAME_WIDTH (64)
#define FRAME_HEIGHT (64)

//--- FOR A HIGH SCORE PREVIOUS VALUE STORAGE
// --- EEPROM BEST, it adds functionality and uses ~0 additional Flash or RAM storage for the data itself
#include <EEPROM.h>
// high score record for each of the games.
int highScores[5] = {0, 0, 0, 0, 0}; // 0-2: Flappy (N, H, Hr), 3-4: Tone (M, PT)
bool newHighScoreReached = false;
int currentScoreIndex = 0; // Tracks which mode we are currently playing

unsigned long gameStartTime = 0;
bool blinkDone = false;

int currentFlex = 0;

// const byte PROGMEM frames[2][512] = {
//   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x30, 0x0C, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x61, 0x86, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x63, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x86, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0C, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x38, 0x1C, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
//   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x1E, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x70, 0x0E, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0xE3, 0xC7, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0xC7, 0xE3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7, 0xE3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE3, 0xC7, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x70, 0x0E, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x78, 0x1E, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
// };

// Split the frames into two separate 1D arrays
const unsigned char PROGMEM frame_0[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x30, 0x0C, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x61, 0x86, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x63, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x86, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0C, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x38, 0x1C, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  // Paste your first 512 bytes here
const unsigned char PROGMEM frame_1[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x1E, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x70, 0x0E, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0xE3, 0xC7, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0xC7, 0xE3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7, 0xE3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE3, 0xC7, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x70, 0x0E, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x78, 0x1E, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  // Paste your second 512 bytes here

// Create a pointer array to handle them
const unsigned char* const frames[] PROGMEM = { frame_0, frame_1 };

const unsigned char PROGMEM heart_bmp[] = { 0b01100110, 0b11111111, 0b11111111, 0b11111111, 0b01111110, 0b00111100, 0b00011000 };
const unsigned char PROGMEM bird_wing_down[] = { 0x1e, 0x00, 0x3f, 0x00, 0x73, 0x80, 0x7b, 0x80, 0x7f, 0x80, 0x3f, 0x80, 0x1f, 0x00, 0x0e, 0x00 };
const unsigned char PROGMEM bird_wing_up[] = { 0x78, 0x00, 0xff, 0x00, 0x7f, 0x80, 0x7b, 0x80, 0x7b, 0x80, 0x73, 0x80, 0x3f, 0x00, 0x1e, 0x00 };

const int speakerPin = 4, trigPin = 9, echoPin = 3, potPin = A0, resetBtnPin = 2;
#define OLED_MOSI 11
#define OLED_CLK 13
#define OLED_DC 7
#define OLED_CS 6
#define OLED_RESET 8
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);


//flappySpeed = 3-> 2, reduced
int gameState = -1, selectedOption = 0, currentFrame = 0, lastPotValue = 0, flappySpeed = 3;
unsigned long lastFrameTime = 0, lastBtnPress = 0;
bool gamePaused = false, gameOver = false, shooterOn = false, shotsFired = false, barrierActive = true;

float handY = 32.0;
//bulletSpeed = 4 -> 3
int birdX = 25, gateX = 127, gateWidth = 15, gapY = 32, gapSize = 25, score = 0, lives = 3, bulletLength = 3, bulletX = 26, bulletY = 32, bulletSpeed = 3;

float noteFrequencies[12] = { 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88 };
enum ToneState { SUB_MENU,
                 PLAY_TARGET,
                 PLAYER_MATCH,
                 SUCCESS,
                 ASK_CONTINUE };
ToneState toneStep = SUB_MENU;
int subGameChoice = 0;  // 0: Original, 1: PT
int targetNote, playerNote, matchCount = 0;
unsigned long toneStateStartTime = 0;
float smoothedDistance = 20.0;

//float aflexrestval[170];
int sumrest = 0;
//int aflexbentval[170];
long sumbent = 0;
int hld = 0;  //holds the top 10% averages of thr bent val
float thld;
int avgrest;
int avgbent;


struct ToneTile {
  int x, width;
  bool active;
};
ToneTile currentTile = { 0, 30, false };
float tileY = -10;
int ptScore = 0;
bool ptGameOver = false;

int flappyDifficulty = 0;  // 0 = Normal, 1 = Hard
int blinkCount = 0;
unsigned long lastBlinkTime = 0;
bool gateVisible = true;

const int calBtnPin = 5;
const int flexPin = A1;

int gamegameState = -1;  // -1 = Intro, 0 = Calibration, 1 = Menu
int flexRestVal = 0;     // To store Max unflexed
int flexBentVal = 1023;  // To store Min flexed
unsigned long calTimer = 0;

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 25000);
  return (duration == 0) ? 999 : duration * 0.034 / 2;
}

// class SoundManager {
// public:
//   static void playBackground(int gs) {
//     static unsigned long lastMusicTime = 0;
//     static bool toggle = false;
//     if (gs == 1 && !gameOver) {
//       if (millis() - lastMusicTime > 500) {
//         toggle = !toggle;
//         tone(speakerPin, toggle ? 200 : 250, 100);
//         lastMusicTime = millis();
//       }
//     } else {
//       noTone(speakerPin);
//     }
//   }
// };

// // Super Mario Main Theme
// int marioIntroMelody[] = {
//   660, 660, 0, 660, 0, 510, 660, 0, 770, 0, 380, 0,
//   510, 380, 320, 440, 480, 450, 440, 380, 660, 770, 880, 690, 770,
//   660, 510, 570, 480, 510, 380, 320, 440, 480, 450, 440, 380,
//   660, 770, 880, 690, 770, 660, 510, 570, 480
// };

// int marioIntroRhythm[] = {
//   100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 200,
//   150, 150, 150, 150, 150, 100, 150, 150, 120, 120, 120, 120, 120,
//   150, 150, 150, 150, 150, 150, 150, 150, 150, 100, 150, 150,
//   120, 120, 120, 120, 120, 150, 150, 150, 150
// };

// Mario Theme - Flappy Bird Intro & Background
// int marioThemeMelody[] = {
//   660, 660, 0, 660, 0, 510, 660, 0, 770, 0, 380, 0,
//   510, 380, 320, 440, 480, 450, 440, 380, 660, 770, 880, 690, 770, 
//   660, 510, 570, 480, 510, 380, 320, 440, 480, 450, 440, 380,
//   660, 770, 880, 690, 770, 660, 510, 570, 480, 0,
//   770, 690, 620, 490, 660, 415, 440, 510, 440, 510, 570
// };

// int marioThemeRhythm[] = {
//   100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 200,
//   150, 150, 150, 150, 150, 100, 150, 150, 120, 120, 120, 120, 120,
//   150, 150, 150, 150, 150, 150, 150, 150, 150, 100, 150, 150,
//   120, 120, 120, 120, 120, 150, 150, 150, 150, 200,
//   150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150
// };

// Super Mario Theme - 10 Second Loop (0:02 - 0:12)
// ADDED: PROGMEM to save RAM for the screen buffer
// const int marioThemeMelody[] PROGMEM = {
  // 660, 660, 0, 660, 0, 510, 660, 0, 770, 0, 380, 0,
  // 510, 380, 320, 440, 480, 450, 440, 380, 660, 770, 880, 690, 770, 
  // 660, 510, 570, 480
// };

// const int marioThemeRhythm[] PROGMEM = {
//   100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 200,
//   150, 150, 150, 150, 150, 100, 150, 150, 120, 120, 120, 120, 120,
//   150, 150, 150, 150
// };

// Mario World Game Over - Flappy Bird Outro
// int marioGameOverMelody[] = {
//   392, 330, 262, 294, 247, 262, 0, 196, 131
// };

// int marioGameOverRhythm[] = {
//   150, 150, 150, 150, 150, 300, 100, 150, 400
// };

// class SoundManager {
// public:
//   // --- BACKGROUND THEMES (Switch for Game -> Nested for Mode) ---
//   static void playTheme(int gs, int mode = 0) {
//     static unsigned long lastMusicTime = 0;
//     static int noteIndex = 0;

//     if (gameOver || gamePaused) { 
//       noTone(speakerPin); 
//       digitalWrite(speakerPin, LOW);
//       noteIndex = 0;
//       return; }

//     switch (gs) {
//       case 2: // FLAPPY BIRD
//         if (mode == 0) { // Normal
//           // ADDED: Place Level 1 Array/Logic here
//         } else if (mode == 1) { // Hard
//           // ADDED: Place Level 2 Array/Logic here
//         } else { // Harder
//           // ADDED: Place Level 3 Array/Logic here
//         }
//         break;

//       case 3: // TONE MATCHER
//         if (mode == 0) { // Original Match
//           // ADDED: Tone Matcher Theme 1
//         } else { // PT Mode
//           // ADDED: Tone Matcher Theme 2
//         }
//         break;
//     }
//   }

//   // --- GAME START SOUNDS ---
//   static void playStartSound(int gs) {
//     switch (gs) {
//       case 2: // Flappy Bird Intro (Countdown from Video)
//         for (int i = 0; i < 3; i++) {
//           tone(speakerPin, 440, 200); // Three short "Beeps"
//           delay(1000);
//         }
//         tone(speakerPin, 880, 500); // One high "GO" pitch
//         break;
//       case 3: 
//         // ADDED: Tone Matcher specific intro
//         break;
//     }
//     digitalWrite(speakerPin, LOW); // Prevent hum
//   }

//   // --- GAME OVER SOUNDS ---
//   static void playEndSound(int gs) {
//     switch (gs) {
//       case 2: // Flappy Bird Game Over (Splat/Fall)
//         tone(speakerPin, 150, 200); delay(200);
//         tone(speakerPin, 100, 300); // Low "thud" sound
//         break;
//       case 3: // Tone Matcher Game Over (Missed Tile)
//         tone(speakerPin, 200, 500); // Buzz sound
//         break;
//     }
//     digitalWrite(speakerPin, LOW); // Prevent hum
//   }
// };

// Shared Start Sound (Countdown)
const int startMelody[] PROGMEM = { 440, 440, 440, 880 };
const int startRhythm[] PROGMEM = { 200, 200, 200, 500 };

// Shared Game Over Sound
const int sharedGameOverMelody[] PROGMEM = { 392, 330, 262, 294, 247, 262, 0, 196, 131 };
const int sharedGameOverRhythm[] PROGMEM = { 150, 150, 150, 150, 150, 300, 100, 150, 400 };

// Flappy Bird Theme (10-second loop)
const int flappyThemeMelody[] PROGMEM = { 660, 660, 0, 660, 0, 510, 660, 0, 770, 0, 380, 0, 510, 380, 320, 440, 480, 450, 440, 380, 660, 770, 880, 690, 770, 660, 510, 570, 480 };
const int flappyThemeRhythm[] PROGMEM = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 200, 150, 150, 150, 150, 150, 100, 150, 150, 120, 120, 120, 120, 120, 150, 150, 150, 150 };

// const int flappyThemeMelody[] PROGMEM = { 660, 660, 0, 660, 0, 510, 660, 0, 770, 0, 380, 0, 510, 380, 320, 440, 480, 450, 440, 380, 660, 770, 880, 690, 770, 660, 510, 570, 480 };
// const int flappyThemeRhythm[] PROGMEM = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 200, 150, 150, 150, 150, 150, 100, 150, 150, 120, 120, 120, 120, 120, 150, 150, 150, 150 };

// const int flappyThemeMelody[] PROGMEM = {
//   659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 392, 0,      // Intro "Sting"
//   523, 392, 330, 440, 494, 466, 440,                     // Phrase 1
//   392, 659, 784, 880, 698, 784,                          // Phrase 2
//   659, 523, 587, 494                                     // Turnaround
// };

// const int flappyThemeRhythm[] PROGMEM = {
//   100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 200, // Intro timing
//   150, 150, 150, 150, 150, 100, 150,                          // Swing feel
//   150, 120, 120, 120, 120, 120, 
//   150, 150, 150, 150
// };

// Tone Matcher Theme
const int toneThemeMelody[] PROGMEM = { 262, 330, 392, 523, 392, 330 };
const int toneThemeRhythm[] PROGMEM = { 200, 200, 200, 200, 200, 200 };

// class SoundManager {
// public:
//   static void playTheme(int gs, int mode = 0) {
//     static unsigned long lastNoteTime = 0;
//     static int noteIdx = 0;
    
//     if (gameOver || gamePaused) { 
//       noTone(speakerPin); 
//       digitalWrite(speakerPin, LOW); // ADDED: Hum fix
//       noteIdx = 0; 
//       return; 
//     }

//     switch (gs) {
//       case 2: // FLAPPY BIRD (All modes use this loop)
//         int totalNotes = sizeof(marioThemeMelody) / sizeof(int);
//         int currentNote = pgm_read_word(&marioThemeMelody[noteIdx]);
//         int currentDuration = pgm_read_word(&marioThemeRhythm[noteIdx]);

//         if (millis() - lastNoteTime >= marioThemeRhythm[noteIdx] + 20) {
//           lastNoteTime = millis();
//           if (marioThemeMelody[noteIdx] > 0) {
//             tone(speakerPin, marioThemeMelody[noteIdx], marioThemeRhythm[noteIdx]);
//           } else {
//             noTone(speakerPin);
//           }
//           noteIdx = (noteIdx + 1) % totalNotes; // ADDED: Repeat audio loop
//         }
//         break;

//       case 3: // TONE MATCHER
//         // ADDED: Nested condition for Tone Matcher modes
//         if (mode == 1) { /* PT Mode Theme */ }
//         break;
//     }
//   }

//   static void playStartSound(int gs) {
//     switch (gs) {
//       case 2: // Flappy Bird Intro Countdown
//         for (int i = 3; i > 0; i--) {
//           tone(speakerPin, 440, 200);
//           delay(1000); // Only place where delay is okay (before game starts)
//         }
//         tone(speakerPin, 880, 500); 
//         break;
//     }
//     digitalWrite(speakerPin, LOW); 
//   }

//   // static void playEndSound(int gs) {
//   //   switch (gs) {
//   //     case 2: // Flappy Bird Game Over "Splat"
//   //       tone(speakerPin, 150, 300);
//   //       delay(300);
//   //       tone(speakerPin, 75, 500);
//   //       break;
//   //   }
//   //   digitalWrite(speakerPin, LOW); 
//   // }
//   static void playEndSound(int gs) {
//     switch (gs) {
//       case 2: // FLAPPY BIRD (All modes: Normal, Hard, Harder)
//         for (int i = 0; i < 9; i++) {
//           if (marioGameOverMelody[i] == 0) {
//             noTone(speakerPin);
//           } else {
//             tone(speakerPin, marioGameOverMelody[i]);
//           }
//           delay(marioGameOverRhythm[i]); // ADDED: Timing for Game Over tune
//           noTone(speakerPin);
//           delay(20); 
//         }
//         break;

//       case 3: // TONE MATCHER
//         tone(speakerPin, 150, 500); // ADDED: Missed note buzz
//         break;
//     }
//     digitalWrite(speakerPin, LOW); // ADDED: Hum fix
//   }
// };

class SoundManager {
public:
  static void playTheme(int gs, int mode = 0) {
    static unsigned long lastNoteTime = 0;
    static int noteIdx = 0;
    
    if (gameOver || gamePaused) { 
      showHighscoreScreen();
      noTone(speakerPin); 
      digitalWrite(speakerPin, LOW); 
      noteIdx = 0; 
      return; 
    }

    const int* currentMelody;
    const int* currentRhythm;
    int totalNotes;

    // Switch case for different background sounds per game [cite: 57, 60]
    switch (gs) {
      case 2: // Flappy Bird
        currentMelody = flappyThemeMelody;
        currentRhythm = flappyThemeRhythm;
        totalNotes = sizeof(flappyThemeMelody) / sizeof(int);
        break;
      case 3: // Tone Matcher
        currentMelody = toneThemeMelody;
        currentRhythm = toneThemeRhythm;
        totalNotes = sizeof(toneThemeMelody) / sizeof(int);
        break;
      default: return;
    }

    int note = pgm_read_word(&currentMelody[noteIdx]);
    int duration = pgm_read_word(&currentRhythm[noteIdx]);

    if (millis() - lastNoteTime >= (unsigned long)duration + 20) {
      lastNoteTime = millis();
      if (note > 0) tone(speakerPin, note, duration);
      else noTone(speakerPin);
      noteIdx = (noteIdx + 1) % totalNotes;
    }
  }

  // Shared start sound for all games [cite: 61, 62]
  static void playStartSound() {
    for (int i = 0; i < 4; i++) {
      tone(speakerPin, pgm_read_word(&startMelody[i]), pgm_read_word(&startRhythm[i]));
      delay(pgm_read_word(&startRhythm[i]) + 50);
    }
    digitalWrite(speakerPin, LOW);
  }

  // Shared end sound for all games [cite: 65, 66]
  static void playEndSound() {
    for (int i = 0; i < 9; i++) {
      int note = pgm_read_word(&sharedGameOverMelody[i]);
      int duration = pgm_read_word(&sharedGameOverRhythm[i]);
      if (note == 0) noTone(speakerPin);
      else tone(speakerPin, note, duration);
      delay(duration + 20);
    }
    digitalWrite(speakerPin, LOW);
  }
};

void resetFlappy() {
  gateX = 127;
  gameOver = false;
  handY = 32;
  flappySpeed = 2;
  gameStartTime = millis();
}
void resetToneMatch() {
  targetNote = random(0, 12);
  toneStep = PLAY_TARGET;
  toneStateStartTime = millis();
}

void playPointBeep() {
  tone(speakerPin, 1000, 50);  // Short high beep for scoring
}

void playCrashSound() {
  tone(speakerPin, 150, 300);  // Longer low sound for errors/crashes
}



void setup() {
  Serial.begin(9600);  // Required for debugging and Plotter
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(speakerPin, OUTPUT);  // [cite: 271, 281, 282]
  pinMode(resetBtnPin, INPUT_PULLUP);
  pinMode(calBtnPin, INPUT_PULLUP);  // [cite: 282]

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    for (;;)
    pinMode(13, OUTPUT);
    // this was for the speaker to shush
    while(1) { digitalWrite(13, !digitalRead(13)); delay(100); }
      ;  // Loop forever if display fails
  }
  display.clearDisplay();
  display.display();
  randomSeed(analogRead(A1));  // [cite: 282]
  //highScore = EEPROM.read(0); // Read the high score from address 0
  for (int i=0; i<5; i++) {
    EEPROM.get(i * sizeof(int), highScores[i]); // Loads all 5 scores at once
}
}


static int calStep = 0;

void runCalibration() {
  static int calStep = 0;
  currentFlex = analogRead(flexPin);

  // Real-time plotting for the IDE Serial Plotter
   Serial.print(F("Current_Flex:"));
   Serial.println(currentFlex); // [cite: 351]

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  // int hld = 0;
  switch (calStep) {
    case 0:  // STEP 1: REST
      display.setCursor(10, 10);
      display.println(F("CALIBRATION: STEP 1"));
      display.setCursor(10, 25);
      display.println(F("DO NOT FLEX"));
      display.setCursor(10, 45);
      display.println(F("Press Pin 5 Start"));
      if (digitalRead(calBtnPin) == HIGH) {
        flexRestVal = 0;
        calTimer = millis();
        calStep = 1;
        delay(300);
      }
      break;

    case 1:  // RECORDING REST
      if (millis() - calTimer < 5000) {
        sumrest += currentFlex;
        display.setCursor(30, 20);
        display.print(F("RECORDING..."));
        display.setCursor(60, 40);
        display.print(5 - (millis() - calTimer) / 1000);  // [cite: 357]
        Serial.print(F("Hold Rest: "));
        Serial.println(hld);
        Serial.print(F("Current Flex: "));
        Serial.println(currentFlex);
        hld++;
        // Progress bar
        int barWidth = map(millis() - calTimer, 0, 5000, 0, 128);
        display.drawRect(0, 52, 128, 10, WHITE);
        display.fillRect(0, 54, barWidth, 6, WHITE);

      } else {
        Serial.print(F("Average Rest: "));
        avgrest = sumrest / hld;
        Serial.println(avgrest);
        calStep = 2;
      }
      break;

    case 2:  // STEP 2: BENT
      display.setCursor(10, 10);
      display.println(F("CALIBRATION: STEP 2"));
      display.setCursor(10, 25);
      display.println(F("FLEX FINGER 3 TIMES"));
      //display.setCursor(10, 45); display.println(F("BEND 3 TIMES"));
      display.setCursor(10, 45);
      display.println(F("Press Pin 5 Start"));
      if (digitalRead(calBtnPin) == HIGH) {
        flexBentVal = 1023;
        calTimer = millis();
        calStep = 3;
        delay(300);
        hld = 0;
      }

      break;

    case 3:  // RECORDING BENT
             //hard reset hold to 0.
      if (millis() - calTimer < 5000) {
        // if (currentFlex < flexBentVal) flexBentVal = currentFlex;
        //aflexbentval[hld] = currentFlex;
        sumbent += currentFlex;
        display.setCursor(30, 20);
        display.print(F("RECORDING..."));
        // display.setCursor(40, 25); display.print(F("BEND 3 TIMES"));
        display.setCursor(60, 30);
        display.print(5 - (millis() - calTimer) / 1000);
        Serial.print(F("Current Flex: "));
        Serial.println(currentFlex);
        Serial.print(F("Hold Bent: "));
        Serial.println(hld);
        hld++;

        // Progress Bar
        int barWidth = map(millis() - calTimer, 0, 5000, 0, 128);
        display.drawRect(0, 52, 128, 10, WHITE);
        display.fillRect(0, 54, barWidth, 6, WHITE);

      } else {
        Serial.print(F("Average Bent: "));
        avgbent = 2 * float(sumbent) / hld;
        Serial.println(avgbent);
        //calStep = 2;
        thld = avgrest + (avgbent - avgrest) / 2;
        calStep = 4;
      }
      break;

    case 4:
      // Validation: Ensure significant range between Rest and Flex [cite: 363]
      if ((avgbent - avgrest) >= 150) {
        display.clearDisplay();
        display.setCursor(25, 20);
        display.print(F("CALIBRATED!"));
        display.setCursor(10, 40);
        display.print(F("ENJOY YOUR GAME"));
        display.display();
        delay(2500);
        calStep = 0;
        gameState = 1;  // Move to Menu [cite: 364]
      } else {
        // Error Path: Reset to Step 0 (Start over)
        display.clearDisplay();
        display.setCursor(10, 10);
        display.print(F("ERROR!"));
        display.setCursor(10, 25);
        display.print(F("Rest:"));
        display.print(avgrest);
        display.setCursor(10, 45);
        display.print(F("Flex:"));
        display.print(avgbent);
        display.display();
        delay(3000);
        calStep = 0;  // LOOP BACK TO STEP 1 [cite: 366]
      }
      break;
  }
  display.display();
  // display.display();
}

class ToneMatcherPT {
public:
  static void run() {
    if (ptGameOver) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(10, 10);
      display.print("OUT!");
      display.setTextSize(1);
      display.setCursor(10, 40);
      display.print("Wave to Restart");
      display.display();
      if (getDistance() < 10) {
        ptGameOver = false;
        ptScore = 0;
        currentTile.active = false;
        delay(500);
      }
      return;
    }
    float rawDist = getDistance();
    if (rawDist < 50) smoothedDistance = (0.85 * smoothedDistance) + (0.15 * rawDist);
    int px = map(constrain(smoothedDistance, 5, 35), 5, 35, 0, SCREEN_WIDTH - 20);
    tone(speakerPin, noteFrequencies[map(constrain(smoothedDistance, 5, 35), 5, 35, 0, 11)]);
    if (!currentTile.active) {
      currentTile.x = random(0, SCREEN_WIDTH - 40);
      currentTile.width = random(30, 50);
      currentTile.active = true;
      tileY = -10;
    }
    tileY += 1.5;
    if (tileY > SCREEN_HEIGHT) {
      if (px >= currentTile.x && (px + 10) <= (currentTile.x + currentTile.width)) {
        ptScore++;
        currentTile.active = false;
      } else {
        ptGameOver = true;
        SoundManager::playEndSound();
        noTone(speakerPin);
        }
      }
    
    display.clearDisplay();
    display.fillRect(currentTile.x, (int)tileY, currentTile.width, 8, WHITE);
    display.fillRect(px, SCREEN_HEIGHT - 10, 20, 4, WHITE);
    display.drawFastHLine(0, SCREEN_HEIGHT - 8, SCREEN_WIDTH, WHITE);
    display.setCursor(0, 0);
    display.print("PT Score: ");
    display.print(ptScore);
    display.display();
  }};




void runStartupAnimation() {
  if (millis() - lastFrameTime >= 250) {
    lastFrameTime = millis();
    display.clearDisplay();

    // Draw animation frames
    const unsigned char* bitmap = (const unsigned char*)pgm_read_word(&(frames[currentFrame]));  // [cite: 858]
    display.drawBitmap(32, 0, bitmap, 64, 64, WHITE);

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(90, 25);
    display.print("MAC");  // [cite: 861]
    display.display();

    currentFrame = (currentFrame + 1) % 2;  // [cite: 863]
  }

  // After 5 seconds, go to Calibration [cite: 864, 865]
  if (millis() > 5000) {
    gameState = 0;
    display.clearDisplay();
    display.display();
  }
}


// void showMenu() {
//   int pot = analogRead(potPin);
//   if (abs(pot - lastPotValue) > 20) { selectedOption = map(pot, 0, 1023, 0, 2); lastPotValue = pot; }

//   display.clearDisplay();
//   display.setTextSize(1);
//   display.setCursor(35, 0);
//   display.print(F("GAME MENU"));

//   display.setTextColor(selectedOption == 0 ? BLACK : WHITE, selectedOption == 0 ? WHITE : BLACK);
//   display.setCursor(10, 15); display.print(F("> Flappy Stabilize"));

//   display.setTextColor(selectedOption == 1 ? BLACK : WHITE, selectedOption == 1 ? WHITE : BLACK);
//   display.setCursor(10, 30); display.print(F("> Tone Matcher"));

//   display.setTextColor(selectedOption == 2 ? BLACK : WHITE, selectedOption == 2 ? WHITE : BLACK);
//   display.setCursor(10, 45); display.print(F("> Recalibrate Flex"));

//   display.setTextColor(WHITE); display.setCursor(5, 56);
//   display.print(F("Wave close to START"));
//   display.display();

//   if (getDistance() < 15) {
//     if (selectedOption == 0) { gameState = 2; lives = 3; score = 0; resetFlappy(); }
//     else if (selectedOption == 1) { gameState = 3; toneStep = SUB_MENU; }
//     else { gameState = 0; }
//     delay(1000);
//   }
// }

void showMenu() {
  int pot = analogRead(potPin);
  Serial.println(pot);
  if (abs(pot - lastPotValue) > 20) {
    selectedOption = map(pot, 0, 1023, 0, 4);
    lastPotValue = pot;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(35, 0);
  display.print(F("GAME MENU"));

  display.setTextColor(selectedOption == 0 ? BLACK : WHITE, selectedOption == 0 ? WHITE : BLACK);
  display.setCursor(10, 12);
  display.print(F("> Flappy Bird"));

  display.setTextColor(selectedOption == 1 ? BLACK : WHITE, selectedOption == 1 ? WHITE : BLACK);
  display.setCursor(10, 25);
  display.print(F("> Tone Matcher"));

  display.setTextColor(selectedOption == 2 ? BLACK : WHITE, selectedOption == 2 ? WHITE : BLACK);
  display.setCursor(10, 38);
  display.print(F("> Skiiii"));

  display.setTextColor(selectedOption == 3 ? BLACK : WHITE, selectedOption == 3 ? WHITE : BLACK);
  display.setCursor(10, 51);
  display.print(F("> Recalibrate"));

  display.display();

  if (getDistance() < 15) {
    if (selectedOption == 0) {
      // Enter Difficulty Sub-menu
      delay(500);
      bool selecting = true;
      while (selecting) {
        int subPot = analogRead(potPin);
        if (subPot < 340) {
        flappyDifficulty = 0;
        shooterOn = true;
        flappySpeed = 3;
        }
        else if (subPot < 681) {
        flappyDifficulty = 1;
        shooterOn = false;
        flappySpeed = 3;
        }
        else {
        flappyDifficulty = 2;
        shooterOn = false;
        flappySpeed = 3;
        }
        //flappyDifficulty = (subPot < 512) ? 0 : 1;
        display.clearDisplay();
        display.setCursor(30, 0);
        display.print(F("DIFFICULTY"));
        display.setTextColor(flappyDifficulty == 0 ? BLACK : WHITE, flappyDifficulty == 0 ? WHITE : BLACK);
        display.setCursor(10, 19);
        display.print(F("> Normal"));
        display.setTextColor(flappyDifficulty == 1 ? BLACK : WHITE, flappyDifficulty == 1 ? WHITE : BLACK);
        display.setCursor(10, 38);
        display.print(F("> Hard"));
        display.setTextColor(flappyDifficulty == 2 ? BLACK : WHITE, flappyDifficulty == 2 ? WHITE : BLACK);
        display.setCursor(10, 57);
        display.print(F("> Harder"));
        display.display();
        if (getDistance() < 10) {
          selecting = false;
          resetFlappy(); // Initialize positions before countdown
        
          gameState = 2; // Start game after countdown
          };
      }
      
      // NEW: Instructions/Start Screen
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(20, 10);
      display.print(F("FLAPPY BIRD"));
      if(flappyDifficulty == 0){
        display.setCursor(0, 30);
        display.print(F("Flex to Shoot"));
      }
      display.setCursor(0, 45);
      display.print(F("Distance = Altitude"));
      display.display();
      delay(3000); // Give the user 3 seconds to read

    
      flashGo(); // Call the flash function
      SoundManager::playStartSound(); // Play Intro Audio AFTER countdown

      gameState = 2;
      lives = 3;
      score = 0;
      resetFlappy();
    } 
    


    else if (selectedOption == 1) {
      gameState = 3;
      SoundManager::playStartSound();
      toneStep = SUB_MENU;
    }
    
    else if (selectedOption == 2){
      gameState = 4;
    }
    else {
      gameState = 0;
    }
    delay(1000);
  }
}


// void runFlappyGame() {
//   SoundManager::playBackground(1);
//   if (gameOver) { showFlappyChoices(); return; }
//   unsigned long elapsed = millis() - gameStartTime;
//   if (elapsed < 5000) {
//     display.clearDisplay(); display.setTextSize(2); display.setCursor(30, 15); display.print("READY?");
//     display.setCursor(55, 40); display.print(5 - (elapsed / 1000)); display.display();
//     return;
//   }
//   float dist = getDistance();
//   if (dist > 2 && dist < 45) { int targetY = map(constrain(dist, 5, 35), 5, 35, SCREEN_HEIGHT - 8, 8); handY = (handY * 0.7) + (targetY * 0.3); }
//   gateX -= 3; if (gateX < -gateWidth) { gateX = SCREEN_WIDTH; gapY = random(16, SCREEN_HEIGHT - 16); score++; }
//   if (birdX + 5 > gateX && birdX - 5 < (gateX + gateWidth)) {
//     if (handY - 3 < (gapY - gapSize / 2) || handY + 3 > (gapY + gapSize / 2)) { gameOver = true; tone(speakerPin, 150, 300); }
//   }
//   display.clearDisplay();
//   display.fillRect(gateX, 0, gateWidth, gapY - gapSize / 2, WHITE);
//   display.fillRect(gateX, gapY + gapSize / 2, gateWidth, SCREEN_HEIGHT - (gapY + gapSize / 2), WHITE);
//   bool wingPos = (millis() / 150) % 2;
//   display.drawBitmap(birdX - 6, (int)handY - 4, wingPos ? bird_wing_up : bird_wing_down, 12, 8, WHITE);

//   // --- UPDATED SCORE SECTION ---
//   display.setTextSize(1);      // Force text to smallest size
//   display.setTextColor(WHITE); // Ensure color is set
//   display.setCursor(0, 0);
//   display.print("Score: ");
//   display.print(score);
//   // -----------------------------

//   for (int i = 0; i < lives; i++) display.drawBitmap(95 + (i * 10), 2, heart_bmp, 8, 7, WHITE);
//   display.display();
// }

// 3-second buffer game start animation
// void flashGo() {
//   display.clearDisplay();
//   display.setTextSize(4);
//   display.setTextColor(WHITE);
//   display.setCursor(40, 20);
//   display.print(F("GO!")); // F() macro saves RAM
//   display.display();
//   delay(500);
// }

void flashGo() {

  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(30, 15);
  display.print(F("GO!"));
  display.display();
  tone(speakerPin, 880, 400); // Higher pitch for "GO"
  delay(600);
  display.setTextSize(1); // Return to standard size
}

// void flashGo() {
//   for (int size = 2; size <= 6; size += 2) { // Quickly zooms from size 2 to 6
//     display.clearDisplay();
//     drawFlappyStatic(); // Shows the game screen behind the text
//     display.setTextSize(size);
//     display.setTextColor(WHITE);
//     // Center logic: approx (64 - (charWidth * size))
//     display.setCursor(64 - (size * 18), 32 - (size * 4)); 
//     display.print(F("GO!")); 
//     display.display();
//     delay(50); 
//   }
//   tone(speakerPin, 880, 400); 
//   delay(400);
// }

//to store high score
void showHighscoreScreen() {
    if (!newHighScoreReached) return; // Only show if they actually broke the record
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print(F("NEW HIGH!"));
    display.setTextSize(1);
    display.setCursor(10, 40);
    display.print(F("Score: "));
    display.print(highScores[currentScoreIndex]);
    display.display();
    
    delay(2500); // Show for 2.5 seconds
    newHighScoreReached = false; // Reset for next time
}

// --- ADDED CHANGES FOR BULLET LOGIC & UI ---
// int bulletCount = 0; // Tracking bullets based on flex
// const int flexThreshold = 600; // Adjust based on your flex high read
// //Adding a global variable at the top, to check for the flex state.
// bool flexWasBent = false;

//draw flappy start page

void runFlappyGame() {
  //SoundManager::playBackground(1);
  // to check for the flexVal to go low in order to go high.
  // new logic
  Serial.println(currentFlex);
   if (gameOver) {
    showFlappyChoices();
    return;
  }
  if (flappyDifficulty == 2) {
  // Trigger blinking as soon as the gate is fully on screen
  bool gateFullyVisible = (gateX <= SCREEN_WIDTH - gateWidth);

  if (gateFullyVisible && !blinkDone) {
    unsigned long blinkInterval = gateVisible ? 120 : 60;

    if (millis() - lastBlinkTime > blinkInterval) {
      gateVisible = !gateVisible;
      lastBlinkTime = millis();

      if (gateVisible) blinkCount++;  // Rising edge = completed blink
    }

    // After 2 blinks, move the gap and stop blinking
    if (blinkCount >= 2) {
      gapY = random(16, SCREEN_HEIGHT - 16);  // ← This now actually fires
      gateVisible = true;
      blinkDone = true;
      blinkCount = 0;
    }
  }
} else if (flappyDifficulty == 1) {
    if (millis() - lastBlinkTime > 400) {  // Blink every 400ms
      gateVisible = !gateVisible;
      lastBlinkTime = millis();
      if (!gateVisible) blinkCount++;

      // After 3 blinks, change the gate's Y position
      if (blinkCount >= 3) {
        gapY = random(16, SCREEN_HEIGHT - 16);  // Change position
        blinkCount = 0;
      }
    }
    
  } else {
    gateVisible = true;  // Always visible in Normal mode
    int sensorVal = analogRead(A1);
    if (sensorVal > thld && !shotsFired){
      shotsFired = true;
      bulletY = handY;
      bulletX = birdX + 1;
      if(barrierActive){
        display.drawRect(bulletX, handY, bulletLength, 1, WHITE);
        }
    }
  }
  // if (shooterOn) {
  //       bulletX += bulletSpeed; 
  //       display.drawRect(bulletX, bulletY, bulletLength, 1, WHITE); 

  //       // --- ADD THE NEW COLLISION LOGIC HERE ---
  //       // CHECK COLLISION WITH GATE
  //       if (gateVisible && bulletX > gateX && bulletX < gateX + gateWidth) {
  //           // If bullet is NOT in the gap, it hits the gate
  //           if (bulletY < (gapY - gapSize / 2) || bulletY > (gapY + gapSize / 2)) {
  //               gateVisible = false; // This removes the barrier [cite: 29]
  //               shooterOn = false;   // Remove bullet
  //               score++;             // Reward for destroying the gate
  //               tone(speakerPin, 1000, 100); // Hit sound
  //           }
  //       }
  //   }
    /////////////////////////////// - new logic
  // SoundManager::playTheme(2, flappyDifficulty);
  

  // Handle hand movement and distance [cite: 148, 149]
  static float smoothDist = 20.0;
  float rawDist = getDistance();

  // Only update smoothing if reading is valid
  if (abs(rawDist - smoothDist) > 10) {
  rawDist = smoothDist;  // ignore crazy jumps
  }
  if (rawDist > 2 && rawDist < 50){
    smoothDist = (0.7 * smoothDist) + (0.3 * rawDist);
  }

  // ALWAYS use smoothDist (not rawDist)
  int targetY = map(constrain(smoothDist, 6, 28), 5, 35, SCREEN_HEIGHT - 8, 8);

  // Smooth movement
  handY = (handY * 0.3) + (targetY * 0.7);

  gateX -= flappySpeed;

  // ADDED: SCORING AND SPEED RESTORATION LOGIC
  // This checks if the bird has just passed the gate
  // if (gateX < birdX && !barrierActive) { 
  //     score++;
  //     barrierActive = true; // Prevents scoring multiple times for one gate
      
  //     // Restore normal speed after the first gate is cleared
  //     // if (score == 1) {
  //     //     if (flappyDifficulty == 0) flappySpeed = 2; // Normal
  //     //     else flappySpeed = 3;                       // Hard/Harder
  //     // }
  // }

  ///reset gate when it goes off screen
  if (gateX < -gateWidth) {
    gateX = SCREEN_WIDTH;
    gapY = random(16, SCREEN_HEIGHT - 16);
    score++;
    blinkCount = 0;
    blinkDone = false;  // ← ADD THIS so next gate blinks too
    gateVisible = true; // ← ensure gate starts visible
    barrierActive = true;
  }
  // Collision Logic with lower/upper Pipe
  if ((birdX + 2 > gateX) && (birdX - 5 < gateX + gateWidth) && (handY - 2 < (gapY - gapSize / 2) || handY + 2 > (gapY + gapSize / 2))) {
  // if (gateVisible && birdX + 4 > gateX && birdX - 4 < (gateX + gateWidth)) {
    // if (handY - 3 < (gapY - gapSize / 2) || handY + 3 > (gapY + gapSize / 2)) {
            currentScoreIndex = flappyDifficulty; 
      if (score > highScores[currentScoreIndex]) {
          highScores[currentScoreIndex] = score;
          EEPROM.put(currentScoreIndex * sizeof(int), highScores[currentScoreIndex]);
          newHighScoreReached = true;
      }
      gameOver = true;
      SoundManager::playEndSound();
    }
    // tone(speakerPin, 150, 300);
  //SoundManager::playTheme(2, flappyDifficulty);

  //Collision with barrier
  if((shooterOn) && (birdX + 1 > gateX + gateWidth) && (barrierActive) && (handY - 2 > (gapY - gapSize / 2) && handY + 2 < (gapY + gapSize / 2))) {
      gameOver = true;
      //tone(speakerPin, 150, 300);
      SoundManager::playEndSound();
    }
  

  //Breaks barrier
  if(shotsFired && barrierActive){
    if((bulletY > (gapY - gapSize / 2)) && (bulletY < (gapY + gapSize / 2)) && (bulletX + bulletLength > gateX + gateWidth)){
      barrierActive = false;
      tone(speakerPin, 150, 300);
    }
  }
  //Or bullet misses
  if ((bulletX + bulletLength > gateX) && (bulletY < (gapY - gapSize / 2)) || (bulletY > (gapY + gapSize / 2))){
    shotsFired = false;
    }

  display.clearDisplay();
  //draw gates
  if (gateVisible) {  // Only draw if not currently "blinked out"
    display.fillRect(gateX, 0, gateWidth, gapY - gapSize / 2, WHITE);
    display.fillRect(gateX, gapY + gapSize / 2, gateWidth, SCREEN_HEIGHT - (gapY + gapSize / 2), WHITE);
    if(shooterOn && barrierActive){
      display.fillRect(gateX + gateWidth + 1, gapY - gapSize / 2, 1, gapSize - 1, WHITE);
      }
  }
  //draw bullet
  if (shotsFired){
    bulletX += bulletSpeed;
    display.drawRect(bulletX, bulletY, bulletLength, 1, WHITE);
    if(bulletX > SCREEN_WIDTH || !barrierActive){
      shotsFired = false;
    }
  }

  // bool wingPos = (millis() / 150) % 2;
  // display.drawBitmap(birdX - 6, (int)handY - 4, wingPos ? bird_wing_up : bird_wing_down, 12, 8, WHITE);
  bool wingPos = (millis() / 150) % 2;
  display.drawBitmap(birdX - 6, (int)handY - 4, wingPos ? bird_wing_up : bird_wing_down, 12, 8, WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("Score: "));

  display.print(score);
  for (int i = 0; i < lives; i++) display.drawBitmap(95 + (i * 10), 2, heart_bmp, 8, 7, WHITE);
  display.display();
}

void showFlappyChoices() {
  int potVal = analogRead(potPin);
  int flappyChoice = map(potVal, 0, 1023, 0, 3);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(30, 0);
  display.print(gamePaused ? "PAUSED" : "CRASHED!");

  const char* labels[] = { "> CONTINUE", "> RETRY (Score 0)", "> EXIT" };
  for (int i = 0; i < 3; i++) {
    display.setCursor(10, 20 + (i * 12));
    if (flappyChoice == i) display.setTextColor(BLACK, WHITE);
    else display.setTextColor(WHITE);

    if (i == 0 && lives <= 0) display.print("> NO LIVES");
    else display.print(labels[i]);
  }
  display.display();

  if (getDistance() < 10) {
    if (flappyChoice == 0 && lives > 0) {
      lives--;
      gamePaused = false;
      resetFlappy();
    } else if (flappyChoice == 1) {
      score = 0;
      lives = 3;
      gamePaused = false;
      resetFlappy();
    } else if (flappyChoice == 2) {
      gameState = 1;  // GO TO MENU
      gamePaused = false;
      gameOver = false;
    }
    delay(1000);
  }
}


void runToneMatchGame() {
  if (toneStep == SUB_MENU) {
    int potVal = analogRead(potPin);
    subGameChoice = (potVal < 512) ? 0 : 1;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(25, 0);
    display.print("SELECT MODE");
    display.setTextColor(subGameChoice == 0 ? BLACK : WHITE, subGameChoice == 0 ? WHITE : BLACK);
    display.setCursor(10, 20);
    display.print("> ORIGINAL MATCH");
    display.setTextColor(subGameChoice == 1 ? BLACK : WHITE, subGameChoice == 1 ? WHITE : BLACK);
    display.setCursor(10, 40);
    display.print("> TONE MATCHER_PT");
    display.display();
    if (getDistance() < 15) {
      if (subGameChoice == 0) {
        matchCount = 0;
        resetToneMatch();
      } else {
        ptScore = 0;
        ptGameOver = false;
        currentTile.active = false;
      }
      toneStep = PLAYER_MATCH;
      delay(1000);
    }
    return;
  }

  if (subGameChoice == 1) {
    ToneMatcherPT::run();
  } else {
    static unsigned long matchStartTime = 0;
    switch (toneStep) {
      case PLAY_TARGET:
        if (millis() - toneStateStartTime < 2000) {
          tone(speakerPin, noteFrequencies[targetNote]);
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(20, 20);
          display.print("LISTEN...");
          display.display();
        } else {
          noTone(speakerPin);
          toneStep = PLAYER_MATCH;
          toneStateStartTime = millis();
        }
        break;
      case PLAYER_MATCH:
        {
          float rawDist = getDistance();
          if (rawDist < 50) smoothedDistance = (0.85 * smoothedDistance) + (0.15 * rawDist);
          playerNote = map(constrain(smoothedDistance, 5, 35), 5, 35, 0, 11);
          tone(speakerPin, (int)noteFrequencies[playerNote]);
          display.clearDisplay();
          display.setTextSize(1);
          display.setCursor(0, 0);
          display.print("Target: ");
          display.print(targetNote);
          display.setCursor(80, 0);
          display.print("Wins:");
          display.print(matchCount);
          display.setCursor(0, 20);
          display.setTextSize(2);
          display.print("Note: ");
          display.print(playerNote);
          int barWidth = map(constrain(smoothedDistance, 5, 35), 5, 35, 0, 128);
          display.drawRect(0, 52, 128, 10, WHITE);
          display.fillRect(0, 54, barWidth, 6, WHITE);
          if (playerNote == targetNote) {
            if (matchStartTime == 0) matchStartTime = millis();
            if (millis() - matchStartTime >= 1000) {
              toneStep = SUCCESS;
              matchCount++;
              matchStartTime = 0;
            }
          } else matchStartTime = 0;
          display.display();
        }
        break;
      case SUCCESS:
        noTone(speakerPin);
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(10, 25);
        display.print("MATCHED!!");
        display.display();
        delay(1000);
        if (matchCount >= 3) toneStep = ASK_CONTINUE;
        else resetToneMatch();
        break;
      case ASK_CONTINUE:
        {
          int choice = (analogRead(potPin) < 512) ? 0 : 1;
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(choice == 0 ? BLACK : WHITE, choice == 0 ? WHITE : BLACK);
          display.setCursor(10, 20);
          display.print("> CONTINUE");
          display.setTextColor(choice == 1 ? BLACK : WHITE, choice == 1 ? WHITE : BLACK);
          display.setCursor(10, 40);
          display.print("> EXIT");
          display.display();
          if (getDistance() < 15) {
            if (choice == 0) {
              matchCount = 0;
              resetToneMatch();
            } else gameState = 1;
            delay(1000);
          }
        }
        break;
    }
  }
}


void loop() {
  static bool lastBtnState = HIGH;
  bool currentBtnState = digitalRead(resetBtnPin);  // [cite: 194]

  if (lastBtnState == LOW && currentBtnState == HIGH) {
    unsigned long dur = millis() - lastBtnPress;  // [cite: 195]

    // Short Press: 50ms - 500ms (Pause/Sub-menu)
    if (dur > 50 && dur < 500) {
      noTone(speakerPin);
      if (gameState == 2) {
        gamePaused = true;
        gameOver = true;
        currentScoreIndex = flappyDifficulty; // 0, 1, or 2
      }
      if (gameState == 3) toneStep = SUB_MENU;  // [cite: 196, 197]
    }
    // Long Press: > 500ms (Return to Main Menu)
    else if (dur >= 500) {
      gameState = 1;  // FIX: Changed from 0 to 1 to go to MENU [cite: 198]
      gamePaused = false;
      gameOver = false;
      noTone(speakerPin);
    }
  }

  if (currentBtnState == LOW && lastBtnState == HIGH) 
    lastBtnPress = millis();  // [cite: 199]
  lastBtnState = currentBtnState;

  if (gamePaused) {
    showFlappyChoices();  // [cite: 200]
  } else {
    switch (gameState) {
      case -1: runStartupAnimation(); break;  // [cite: 852]
      case 0: runCalibration(); break;        // New Calibration State
      case 1: showMenu(); break;              // [cite: 853]
      case 2: runFlappyGame(); break;         // [cite: 854]
      case 3: runToneMatchGame(); break;      // [cite: 855]
    }
  }
}
