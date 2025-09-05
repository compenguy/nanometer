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

static char textbuf[64] = { 0 };
static int32_t dC = INT_MIN;
static int16_t humPm = -1;
static int64_t time = -1;
static bool degF = true;

// 2.9'' EPD Module
//GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(GxEPD2_290_BS(CS_PIN, DC_PIN, RES_PIN, BUSY_PIN)); // DEPG0290BS 128x296, SSD1680
GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(CS_PIN, DC_PIN, RES_PIN, BUSY_PIN));  // GDEM029C90 128x296, SSD1680

static void redraw_display();

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
  redraw_display();

  Serial.println("[display] Putting display to sleep");
  display.hibernate();
}

void redraw_display() {
  Serial.println("[display] Full window drawing");
  display.setFullWindow();
  Serial.println("[display] Starting display paging");
  display.firstPage();
  do {
    // Fill entire window with black, then paint the insides of of each display box white
    // this should leave 1px black borders around each display area
    display.fillScreen(GxEPD_BLACK);
    display.fillRect(HIST_ORIGIN_X + 1, HIST_ORIGIN_Y + 1, HIST_WIDTH - 2, HIST_HEIGHT - 2, GxEPD_WHITE);
    display.fillRect(TEMP_ORIGIN_X + 1, TEMP_ORIGIN_Y + 1, TEMP_WIDTH - 2, TEMP_HEIGHT - 2, GxEPD_WHITE);
    display.fillRect(HUM_ORIGIN_X + 1, HUM_ORIGIN_Y + 1, HUM_WIDTH - 2, HUM_HEIGHT - 2, GxEPD_WHITE);
    display.fillRect(TIME_ORIGIN_X + 1, TIME_ORIGIN_Y + 1, TIME_WIDTH - 2, TIME_HEIGHT - 2, GxEPD_WHITE);
    Serial.println("[display] Page updated");
    // TODO: Print small text labels in the upper left corner of each box
  } while (display.nextPage());
  Serial.println("[display] Finished drawing section dividers");
}

void update_temperature(int32_t dC, bool fahrenheit) {
  // INT_MIN is the flag value for an invalid temperature
  // don't update display if it gets passed to us
  if (dC == INT_MIN) {
    return;
  }
  if (fahrenheit) {
    int32_t dF = ((dC * 9) / 5) + 320;
    snprintf(textbuf, sizeof(textbuf), "%d.%dF", dF / 10, dF % 10);
  } else {
    snprintf(textbuf, sizeof(textbuf), "%d.%dC", dC / 10, dC % 10);
  }
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(textbuf, TEMP_ORIGIN_X, TEMP_ORIGIN_Y, &tbx, &tby, &tbw, &tbh);
  // TODO: Verify text bounds width and height vs allocated area width and height
  // Center text in the display area
  uint16_t x = ((TEMP_WIDTH - tbw) / 2) - tbx;
  uint16_t y = ((TEMP_HEIGHT - tbh) / 2) - tby;
  do {
    display.setCursor(x, y);
    display.print(textbuf);
  } while (display.nextPage());
}
