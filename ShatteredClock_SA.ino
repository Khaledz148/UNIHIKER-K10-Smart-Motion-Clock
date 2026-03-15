#include "unihiker_k10.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <time.h>
#include <cmath>

UNIHIKER_K10 k10;
AHT20 aht20;

// =========================
// Display / Region Settings
// =========================
uint8_t screen_dir = 3;
const int SCREEN_W = 320;
const int SCREEN_H = 240;

// Saudi Arabia (Riyadh) UTC+3
const long gmtOffset_sec = 3 * 3600;
const int daylightOffset_sec = 0;

// RGB only in dark environment
const int ALS_COVER_THRESHOLD = 40;

// =========================
// Particle System
// =========================
// Time digits: 4 digits x 7 seg x 5 frags = 140
// Colon: 4
// Date text: 20 approx
// Bottom info: 40 approx
const int PARTICLE_COUNT = 250;

float partOffX[PARTICLE_COUNT] = {0};
float partOffY[PARTICLE_COUNT] = {0};
float partVelX[PARTICLE_COUNT] = {0};
float partVelY[PARTICLE_COUNT] = {0};

// =========================
// Colors
// =========================
const uint32_t BG_COLOR        = 0x07110C; // deep green-black
const uint32_t PANEL_COLOR     = 0x0D1B14; // dark panel
const uint32_t PANEL_LINE      = 0x173525; // subtle line
const uint32_t TIME_COLOR      = 0xF5FFF8; // bright off-white
const uint32_t DATE_COLOR      = 0x9FC7B1; // muted mint
const uint32_t ACCENT_GREEN    = 0x17AE81; // elegant green
const uint32_t TEMP_COLOR      = 0xFFB36B; // warm orange
const uint32_t HUM_COLOR       = 0x66CCFF; // cool blue
const uint32_t LABEL_COLOR     = 0x7F9D8A; // muted label
const uint32_t STATUS_OK       = 0x67E8A3;
const uint32_t STATUS_WARN     = 0xFF7070;

// =========================
// Helper: HSV -> RGB
// =========================
void hsv2rgb(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b) {
  float hh = fmod(h, 360.0f) / 60.0f;
  int i = (int)floor(hh);
  float f = hh - i;
  float p = v * (1.0f - s);
  float q = v * (1.0f - s * f);
  float t = v * (1.0f - s * (1.0f - f));
  float R = 0, G = 0, B = 0;

  switch (i) {
    case 0: R = v; G = t; B = p; break;
    case 1: R = q; G = v; B = p; break;
    case 2: R = p; G = v; B = t; break;
    case 3: R = p; G = q; B = v; break;
    case 4: R = t; G = p; B = v; break;
    default: R = v; G = p; B = q; break;
  }

  r = (uint8_t)round(R * 255.0f);
  g = (uint8_t)round(G * 255.0f);
  b = (uint8_t)round(B * 255.0f);
}

// =========================
// UI Helpers
// =========================
void fillRect(int x, int y, int w, int h, uint32_t c) {
  k10.canvas->canvasRectangle(x, y, w, h, c, c, true);
}

void drawPanelFrame() {
  // Background
  fillRect(0, 0, SCREEN_W, SCREEN_H, BG_COLOR);

  // Top strip
  fillRect(10, 10, 300, 34, PANEL_COLOR);
  k10.canvas->canvasRectangle(10, 10, 300, 34, PANEL_LINE, PANEL_LINE, false);

  // Bottom cards
  fillRect(18, 185, 120, 42, PANEL_COLOR);
  k10.canvas->canvasRectangle(18, 185, 120, 42, PANEL_LINE, PANEL_LINE, false);

  fillRect(182, 185, 120, 42, PANEL_COLOR);
  k10.canvas->canvasRectangle(182, 185, 120, 42, PANEL_LINE, PANEL_LINE, false);

  // Accent top line
  fillRect(10, 46, 300, 2, ACCENT_GREEN);
}

// =========================
// Fragmented Rect
// =========================
void drawFragmentedRect(int x, int y, int w, int h, bool isVertical, int splits, uint32_t color, float* offX, float* offY) {
  if (isVertical) {
    float segH = (float)h / splits;
    for (int i = 0; i < splits; i++) {
      int curY = y + (int)(i * segH);
      int curH = (int)((i + 1) * segH) - (int)(i * segH);
      fillRect(x + (int)offX[i], curY + (int)offY[i], w, curH, color);
    }
  } else {
    float segW = (float)w / splits;
    for (int i = 0; i < splits; i++) {
      int curX = x + (int)(i * segW);
      int curW = (int)((i + 1) * segW) - (int)(i * segW);
      fillRect(curX + (int)offX[i], y + (int)offY[i], curW, h, color);
    }
  }
}

// =========================
// Big Digit
// =========================
void drawBigDigit(int x, int y, int num, int scale, uint32_t color, float* offX, float* offY) {
  const uint8_t segs[10] = {
    0x7E, 0x30, 0x6D, 0x79, 0x33,
    0x5B, 0x5F, 0x70, 0x7F, 0x7B
  };

  uint8_t pattern = segs[num % 10];
  int w = 12 * scale;
  int h = 20 * scale;
  int t = 2 * scale;
  const int FRAGS = 5;

  // A
  if (pattern & 0x40) drawFragmentedRect(x + t, y, w - 2 * t, t, false, FRAGS, color, &offX[0],  &offY[0]);
  // B
  if (pattern & 0x20) drawFragmentedRect(x + w - t, y + t, t, h - 2 * t, true, FRAGS, color, &offX[5],  &offY[5]);
  // C
  if (pattern & 0x10) drawFragmentedRect(x + w - t, y + h + t, t, h - 2 * t, true, FRAGS, color, &offX[10], &offY[10]);
  // D
  if (pattern & 0x08) drawFragmentedRect(x + t, y + 2 * h, w - 2 * t, t, false, FRAGS, color, &offX[15], &offY[15]);
  // E
  if (pattern & 0x04) drawFragmentedRect(x, y + h + t, t, h - 2 * t, true, FRAGS, color, &offX[20], &offY[20]);
  // F
  if (pattern & 0x02) drawFragmentedRect(x, y + t, t, h - 2 * t, true, FRAGS, color, &offX[25], &offY[25]);
  // G
  if (pattern & 0x01) drawFragmentedRect(x + t, y + h, w - 2 * t, t, false, FRAGS, color, &offX[30], &offY[30]);
}

// =========================
// Big Colon (with subtle pulse)
// =========================
void drawBigColon(int x, int y, int scale, uint32_t color, float* offX, float* offY) {
  int h = 20 * scale;
  drawFragmentedRect(x, y + h / 2,       2 * scale, 2 * scale, false, 2, color, &offX[0], &offY[0]);
  drawFragmentedRect(x, y + h + h / 2,   2 * scale, 2 * scale, false, 2, color, &offX[2], &offY[2]);
}

// =========================
// Text Mosaic
// =========================
void drawTextMosaic(String text, int startX, int startY, uint32_t color, Canvas::eFontSize_t font, int charW, int startIdx) {
  int cursorX = startX;
  for (int i = 0; i < text.length(); i++) {
    int pIdx = startIdx + i;
    if (pIdx >= PARTICLE_COUNT) pIdx = PARTICLE_COUNT - 1;

    String ch = text.substring(i, i + 1);
    k10.canvas->canvasText(
      ch,
      cursorX + (int)partOffX[pIdx],
      startY + (int)partOffY[pIdx],
      color,
      font,
      charW + 5,
      false
    );
    cursorX += charW;
  }
}

// =========================
// WiFi Setup Screen
// =========================
void drawBootMessage(String msg, uint32_t color) {
  fillRect(0, 0, SCREEN_W, SCREEN_H, BG_COLOR);
  fillRect(16, 82, 288, 76, PANEL_COLOR);
  k10.canvas->canvasRectangle(16, 82, 288, 76, PANEL_LINE, PANEL_LINE, false);
  fillRect(16, 80, 288, 3, ACCENT_GREEN);
  k10.canvas->canvasText(msg, 28, 110, color, k10.canvas->eCNAndENFont24, 260, true);
  k10.canvas->updateCanvas();
}

// =========================
// Setup
// =========================
void setup() {
  Serial.begin(115200);

  k10.begin();
  k10.initScreen(screen_dir);
  k10.creatCanvas();
  k10.setScreenBackground(BG_COLOR);

  delay(200);

  drawBootMessage("Starting Clock...", TIME_COLOR);

  // WiFi
  WiFi.mode(WIFI_STA);
  drawBootMessage("Connecting WiFi...", TIME_COLOR);

  {
    WiFiManager wm;
    wm.setConfigPortalTimeout(180);

    bool ok = wm.autoConnect("K10-Clock-Setup");

    if (!ok) {
      drawBootMessage("WiFi Setup Failed", STATUS_WARN);
      delay(1500);
    } else {
      drawBootMessage("WiFi Connected", STATUS_OK);
      delay(900);
    }
  }

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.google.com", "time.windows.com");
  delay(300);

  k10.rgb->brightness(9);
  k10.rgb->write(-1, 0, 0, 0);
}

// =========================
// Loop
// =========================
void loop() {
  struct tm timeinfo;
  bool timeReady = getLocalTime(&timeinfo);

  float tempC = aht20.getData(AHT20::eAHT20TempC);
  float hum   = aht20.getData(AHT20::eAHT20HumiRH);

  int als = k10.readALS();
  int strength = k10.getStrength();
  int rawX = k10.getAccelerometerX();
  int rawY = k10.getAccelerometerY();

  float shakeDirX = rawY / 20.0f;
  float shakeDirY = -rawX / 20.0f;

  drawPanelFrame();

  // =========================
  // Physics
  // =========================
  int diff = abs(strength - 1000);

  for (int i = 0; i < PARTICLE_COUNT; i++) {
    if (diff >= 800) {
      float accX = random(-150, 151) / 2.0f;
      float accY = random(-150, 151) / 2.0f;

      partVelX[i] += accX + shakeDirX;
      partVelY[i] += accY + shakeDirY;

      partVelX[i] *= 0.995f;
      partVelY[i] *= 0.995f;
    }
    else if (diff >= 50) {
      float accX = random(-60, 61) / 4.0f;
      float accY = random(-60, 61) / 4.0f;

      partVelX[i] += accX + (shakeDirX * 0.3f);
      partVelY[i] += accY + (shakeDirY * 0.3f);

      float k = 0.015f;
      partVelX[i] += -k * partOffX[i];
      partVelY[i] += -k * partOffY[i];

      partVelX[i] *= 0.96f;
      partVelY[i] *= 0.96f;
    }
    else {
      float k = 0.35f;
      float forceX = -k * partOffX[i];
      float forceY = -k * partOffY[i];

      partVelX[i] += forceX;
      partVelY[i] += forceY;

      partVelX[i] *= 0.60f;
      partVelY[i] *= 0.60f;

      if (abs(partOffX[i]) < 1.0f && abs(partVelX[i]) < 0.5f) {
        partOffX[i] = 0;
        partVelX[i] = 0;
      }
      if (abs(partOffY[i]) < 1.0f && abs(partVelY[i]) < 0.5f) {
        partOffY[i] = 0;
        partVelY[i] = 0;
      }
    }

    partOffX[i] += partVelX[i];
    partOffY[i] += partVelY[i];
  }

  // =========================
  // Draw Time / Date / Info
  // =========================
  if (timeReady) {
    const char* wds[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

    char dateBuf[32];
    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d %s",
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             wds[timeinfo.tm_wday]);

    // top date line
    drawTextMosaic(String(dateBuf), 48, 20, DATE_COLOR, k10.canvas->eCNAndENFont24, 14, 144);

    // Time
    int scale = 3;
    int digitW = 12 * scale + 6;
    int colonW = 4 * scale + 6;
    int totalW = (4 * digitW) + colonW;

    int startX = (SCREEN_W - totalW) / 2;
    int startY = 58;

    int h1 = timeinfo.tm_hour / 10;
    int h2 = timeinfo.tm_hour % 10;
    int m1 = timeinfo.tm_min / 10;
    int m2 = timeinfo.tm_min % 10;

    // subtle pulse for colon
    unsigned long ms = millis();
    bool colonBright = ((ms / 500) % 2 == 0);
    uint32_t colonColor = colonBright ? ACCENT_GREEN : 0x2C5C47;

    drawBigDigit(startX,                         startY, h1, scale, TIME_COLOR,  &partOffX[0],   &partOffY[0]);
    drawBigDigit(startX + digitW,                startY, h2, scale, TIME_COLOR,  &partOffX[35],  &partOffY[35]);
    drawBigColon(startX + digitW * 2,            startY, scale, colonColor,      &partOffX[70],  &partOffY[70]);
    drawBigDigit(startX + digitW * 2 + colonW,   startY, m1, scale, TIME_COLOR,  &partOffX[74],  &partOffY[74]);
    drawBigDigit(startX + digitW * 3 + colonW,   startY, m2, scale, TIME_COLOR,  &partOffX[109], &partOffY[109]);

    // Bottom Labels
    drawTextMosaic("TEMP", 34, 191, LABEL_COLOR, k10.canvas->eCNAndENFont16, 9, 164);
    drawTextMosaic(String(tempC, 1) + "C", 34, 206, TEMP_COLOR, k10.canvas->eCNAndENFont24, 13, 174);

    drawTextMosaic("HUM", 200, 191, LABEL_COLOR, k10.canvas->eCNAndENFont16, 9, 184);
    drawTextMosaic(String(hum, 0) + "%", 200, 206, HUM_COLOR, k10.canvas->eCNAndENFont24, 13, 194);
  }
  else {
    k10.canvas->canvasText("SYNCING TIME...", 52, 110, TIME_COLOR, k10.canvas->eCNAndENFont24, SCREEN_W, true);
  }

  // =========================
  // Ambient RGB
  // =========================
  if (als >= 0 && als < ALS_COVER_THRESHOLD) {
    unsigned long ms = millis();
    float phase = (ms % 1200) / 1200.0f;
    float breath = 0.55f * (0.5f * (1.0f + sinf(2.0f * 3.1415926f * phase - 3.1415926f / 2.0f))) + 0.18f;

    // keep hue in green/cyan elegant range
    float hue = 145.0f + 20.0f * sinf(ms / 700.0f);

    uint8_t r, g, b;
    hsv2rgb(hue, 0.85f, breath, r, g, b);

    k10.rgb->brightness(9);
    k10.rgb->write(-1, r, g, b);
  } else {
    k10.rgb->write(-1, 0, 0, 0);
  }

  k10.canvas->updateCanvas();
  delay(30);
}