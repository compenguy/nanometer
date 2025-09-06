#include <limits.h>
#include <Arduino.h>

// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0
#define ENABLE_GxEPD2_display 0

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

// Arduino Nano RP2040 Connect CS(SS)=D10,SCL(SCK)=D13,SDA(SPI0_TX)=D11,BUSY=D7,RES(RST)=D9,DC=D8
#define SCK_PIN (SCK)
#define SPI0_RX (MISO)
#define SPI0_TX (MOSI)
#define CS_PIN (SS)
#define D7 (p19)
#define D8 (p20)
#define D9 (p21)
#define BUSY_PIN (D7)
#define RES_PIN (D9)
#define DC_PIN (D8)

// The rendering area (296x128) is divided thus:
// +---------+---------+---------+
// |   tmp   |   hum   |   tim   |
// +---------+---------+---------+
// |       history graph         |
// +-----------------------------+
// if the origin is the bottom left corner, then the display regions have the following bounding boxes
// history graph:
//   (0,0) (295,0) (0,63) (295,63)
#define _HIST_WIDTH (296)
#define _HIST_HEIGHT (64)
static const uint16_t HIST_ORIGIN_X = 0;
static const uint16_t HIST_ORIGIN_Y = 0;
static const uint16_t HIST_WIDTH = _HIST_WIDTH;
static const uint16_t HIST_HEIGHT = _HIST_HEIGHT;
// temperature box:
//   (0,64) (98,64) (0,127) (98,127)
#define _TEMP_WIDTH (98)
#define _TEMP_HEIGHT (64)
static const uint16_t TEMP_ORIGIN_X = 0;
static const uint16_t TEMP_ORIGIN_Y = HIST_HEIGHT;
static const uint16_t TEMP_WIDTH = _TEMP_WIDTH;
static const uint16_t TEMP_HEIGHT = _TEMP_WIDTH;
// humidity box:
//   (99,64) (196,64) (99,127) (196,127)
#define _HUM_WIDTH (98)
#define _HUM_HEIGHT (64)
static const uint16_t HUM_ORIGIN_X = TEMP_ORIGIN_X + TEMP_WIDTH;
static const uint16_t HUM_ORIGIN_Y = HIST_HEIGHT;
static const uint16_t HUM_WIDTH = _HUM_WIDTH;
static const uint16_t HUM_HEIGHT = _HUM_HEIGHT;
// time box:
//   (64,197) (64,295) (127,197) (127,295)
#define _TIME_WIDTH (98)
#define _TIME_HEIGHT (64)
static const uint16_t TIME_ORIGIN_X = HUM_ORIGIN_X + HUM_WIDTH;
static const uint16_t TIME_ORIGIN_Y = HIST_HEIGHT;
static const uint16_t TIME_WIDTH = _TIME_WIDTH;
static const uint16_t TIME_HEIGHT = _TIME_HEIGHT;

static const uint32_t REDRAW_TIMEOUT_MS = 1000 * 60 * 60 * 4;  // ms * seconds * minutes * hours = 4 hours

static char textbuf[64] = { 0 };

static bool dispF = true;
static const int32_t INVALID_DECIC = INT_MIN;
static int32_t deciC = INVALID_DECIC;
static bool tempDirty = true;

static const int32_t INVALID_HUMPM = -1;
static int16_t humPm = INVALID_HUMPM;
static bool humDirty = true;

static const int32_t INVALID_TIME = -1;
static int64_t time = INVALID_TIME;
static bool timeDirty = true;

static uint32_t lastRedrawMillis = 0;

// 2.9'' EPD Module
//GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(GxEPD2_290_BS(CS_PIN, DC_PIN, RES_PIN, BUSY_PIN)); // DEPG0290BS 128x296, SSD1680
GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(CS_PIN, DC_PIN, RES_PIN, BUSY_PIN));  // GDEM029C90 128x296, SSD1680

static void draw_temperature();
static void flush_temperature();

void setup_display() {
  Serial.println("[display] Calling init");
  display.init(115200, true, 50, false);

  Serial.println("[display] Setting rotation");
  display.setRotation(1);
  Serial.println("[display] Setting font");
  display.setFont(&FreeMonoBold9pt7b);
  Serial.println("[display] Setting text color");
  display.setTextColor(GxEPD_RED);

  Serial.println("[display] Drawing section dividers");
  update_display(true, true);

  Serial.println("[display] Putting display to sleep");
  display.hibernate();
}

void update_temperature(int32_t newDeciC) {
  // INVALID_DECIC is the flag value for an invalid temperature
  // don't update display if it gets passed to us
  if (deciC == newDeciC && newDeciC != INVALID_DECIC) {
    return;
  }
  deciC = newDeciC;
  tempDirty = true;
  update_display(true);
}

static void draw_temperature() {
  if (deciC == INVALID_DECIC) {
    Serial.println("Not drawing temperature, as the last temperature update was invalid");
    return;
  }

  Serial.println("[display] Drawing temperature update");
  if (dispF) {
    int32_t deciF = ((deciC * 9) / 5) + 320;
    snprintf(textbuf, sizeof(textbuf), "%d.%deciF", deciF / 10, deciF % 10);
  } else {
    snprintf(textbuf, sizeof(textbuf), "%d.%deciC", deciC / 10, deciC % 10);
  }

  // draw border
  display.fillRect(TEMP_ORIGIN_X, TEMP_ORIGIN_Y, TEMP_WIDTH, TEMP_HEIGHT, GxEPD_BLACK);
  display.fillRect(TEMP_ORIGIN_X + 1, TEMP_ORIGIN_Y + 1, TEMP_WIDTH - 2, TEMP_HEIGHT - 2, GxEPD_WHITE);

  // Get text bounds
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(textbuf, 0, 0, &tbx, &tby, &tbw, &tbh);
  // TODO: Verify text bounds width and height vs allocated area width and height
  // Center text in the display area
  uint16_t x = ((TEMP_WIDTH - tbw) / 2) - tbx;
  uint16_t y = ((TEMP_HEIGHT - tbh) / 2) - tby;
  // Render text centered in the bounding box
  display.setCursor(TEMP_ORIGIN_X + x, TEMP_ORIGIN_Y + y);
  display.print(textbuf);
  Serial.println("[display] Finished drawing temperature update");
}

static void flush_temperature() {
  Serial.println("[display] Flushing temperature update to screen");
  display.displayWindow(TEMP_ORIGIN_X, TEMP_ORIGIN_Y, TEMP_WIDTH, TEMP_HEIGHT);
  Serial.println("[display] Finished flushing temperature update to screen");
  tempDirty = false;
}

static void draw_humidity() {
  if (humPm == INVALID_HUMPM) {
    Serial.println("Not drawing humidity, as the last humidity update was invalid");
    return;
  }

  Serial.println("[display] Drawing humidity update");
  snprintf(textbuf, sizeof(textbuf), "%d.%d%%", humPm / 10, humPm % 10);

  // draw border
  display.fillRect(HUM_ORIGIN_X, HUM_ORIGIN_Y, HUM_WIDTH, HUM_HEIGHT, GxEPD_BLACK);
  display.fillRect(HUM_ORIGIN_X + 1, HUM_ORIGIN_Y + 1, HUM_WIDTH - 2, HUM_HEIGHT - 2, GxEPD_WHITE);

  // Get text bounds
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(textbuf, 0, 0, &tbx, &tby, &tbw, &tbh);
  // TODO: Verify text bounds width and height vs allocated area width and height
  // Center text in the display area
  uint16_t x = ((HUM_WIDTH - tbw) / 2) - tbx;
  uint16_t y = ((HUM_HEIGHT - tbh) / 2) - tby;
  // Render text centered in the bounding box
  display.setCursor(HUM_ORIGIN_X + x, HUM_ORIGIN_Y + y);
  display.print(textbuf);
  Serial.println("[display] Finished drawing humidity update");
}

static void flush_humidity() {
  Serial.println("[display] Flushing humidity update to screen");
  display.displayWindow(HUM_ORIGIN_X, HUM_ORIGIN_Y, HUM_WIDTH, HUM_HEIGHT);
  Serial.println("[display] Finished flushing humidity update to screen");
  humDirty = false;
}

static void draw_time() {
  if (time == INVALID_TIME) {
    Serial.println("Not drawing time, as the last time update was invalid");
    return;
  }

  Serial.println("[display] Drawing time update");
  snprintf(textbuf, sizeof(textbuf), "%d", time);

  // draw border
  display.fillRect(TIME_ORIGIN_X, TIME_ORIGIN_Y, TIME_WIDTH, TIME_HEIGHT, GxEPD_BLACK);
  display.fillRect(TIME_ORIGIN_X + 1, TIME_ORIGIN_Y + 1, TIME_WIDTH - 2, TIME_HEIGHT - 2, GxEPD_WHITE);

  // Get text bounds
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(textbuf, 0, 0, &tbx, &tby, &tbw, &tbh);
  // TODO: Verify text bounds width and height vs allocated area width and height
  // Center text in the display area
  uint16_t x = ((TIME_WIDTH - tbw) / 2) - tbx;
  uint16_t y = ((TIME_HEIGHT - tbh) / 2) - tby;
  // Render text centered in the bounding box
  display.setCursor(TIME_ORIGIN_X + x, TIME_ORIGIN_Y + y);
  display.print(textbuf);
  Serial.println("[display] Finished drawing time update");
}

static void flush_time() {
  Serial.println("[display] Flushing time update to screen");
  display.displayWindow(TIME_ORIGIN_X, TIME_ORIGIN_Y, TIME_WIDTH, TIME_HEIGHT);
  Serial.println("[display] Finished flushing time update to screen");
  timeDirty = false;
}

static void draw_history() {
  Serial.println("[display] Drawing history update");

  // draw border
  display.fillRect(HIST_ORIGIN_X, HIST_ORIGIN_Y, HIST_WIDTH, HIST_HEIGHT, GxEPD_BLACK);
  display.fillRect(HIST_ORIGIN_X + 1, HIST_ORIGIN_Y + 1, HIST_WIDTH - 2, HIST_HEIGHT - 2, GxEPD_WHITE);

  Serial.println("[display] Finished drawing history update");
}

static void flush_history() {
  Serial.println("[display] Flushing history update to screen");
  display.displayWindow(HIST_ORIGIN_X, HIST_ORIGIN_Y, HIST_WIDTH, HIST_HEIGHT);
  Serial.println("[display] Finished flushing history update to screen");
  histDirty = false;
}

static void flush_fullscreen() {
  if (!redraw_expired()) {
    Serial.println("[display] fullscreen flush requested, but the redraw period hasn't expired");
    return;
  }
  Serial.println("[display] Full screen redraw timeout expired");
  Serial.println("[display] Flushing buffer to screen...");
  display.displayWindow(0, 0, display.width(), display.height());
  Serial.println("[display] Finished redrawing full screen");
  lastRedrawMillis = millis();
  tempDirty = false;
  humDirty = false;
  timeDirty = false;
}

static bool redraw_expired() {
  return (lastRedrawMillis == 0) || ((millis() - lastRedrawMillis) > REDRAW_TIMEOUT_MS);
}

static void update_display(bool flushTemp, bool flushHum, bool flushTime, bool flushHist) {
  bool expired = redraw_expired();
  if (tempDirty || expired) {
    Serial.println("[display] Temperature requires redraw");
    draw_temperature();
    if (flushTemp && !expired) {
      Serial.println("[display] Flushing redrawn temperature to display");
      flush_temperature();
    }
  }

  if (humDirty || expired) {
    Serial.println("[display] Humidity requires redraw");
    draw_humidity();
    if (flushHum && !expired) {
      Serial.println("[display] Flushing redrawn humidity to display");
      flush_humidity();
    }
  }

  /*
  if (timeDirty || expired) {
    Serial.println("[display] Time requires redraw");
    draw_time();
    if (flushTime && !expired) {
      Serial.println("[display] Flushing redrawn time to display");
      flush_time();
    }
  }

  if (histDirty || expired) {
    Serial.println("[display] History requires redraw");
    draw_history();
    if (flushHist && !expired) {
      Serial.println("[display] Flushing redrawn history to display");
      flush_hist();
    }
  }
  */

  if (expired) {
    flush_fullscreen();
  }
}
