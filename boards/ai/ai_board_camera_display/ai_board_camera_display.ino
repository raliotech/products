#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "esp_camera.h"

// ── TFT pins ─────────────────────────────────────────────────────
#define TFT_CS   -1
#define TFT_RST   38
#define TFT_DC    36
#define TFT_MOSI  37
#define TFT_SCLK  35

// ── Camera pins ──────────────────────────────────────────────────
#define PWDN_GPIO_NUM   -1
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM   15
#define SIOD_GPIO_NUM   47
#define SIOC_GPIO_NUM   48
#define Y9_GPIO_NUM     16
#define Y8_GPIO_NUM     17
#define Y7_GPIO_NUM     18
#define Y6_GPIO_NUM     12
#define Y5_GPIO_NUM     10
#define Y4_GPIO_NUM      8
#define Y3_GPIO_NUM      9
#define Y2_GPIO_NUM     11
#define VSYNC_GPIO_NUM   6
#define HREF_GPIO_NUM    7
#define PCLK_GPIO_NUM   13

#define BOOT_BTN   0
#define TFT_W    160
#define TFT_H    128

// ── Display resolution to use ─────────────────────────────────────
// With PSRAM we can buffer a larger frame and scale it down
// QVGA (320×240) → scale 2:1 → 160×120 on TFT
// This gives much better image quality than native QQVGA
#define CAM_W    320
#define CAM_H    240
#define DST_W    160
#define DST_H    120
#define Y_PAD    ((TFT_H - DST_H) / 2)   // 4px top/bottom padding

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// PSRAM line buffer for one scaled row — allocated once
uint16_t* rowBuf = nullptr;

// ── PSRAM init ────────────────────────────────────────────────────
void initPSRAM() {
  if (psramInit()) {
    Serial.printf("PSRAM OK: %.2f MB free\n",
                  ESP.getFreePsram() / 1048576.0f);
  } else {
    Serial.println("PSRAM init failed — continuing with heap");
  }
}

// ── Camera init ───────────────────────────────────────────────────
bool initCamera() {
  camera_config_t config;
  config.ledc_channel  = LEDC_CHANNEL_0;
  config.ledc_timer    = LEDC_TIMER_0;
  config.pin_d0        = Y2_GPIO_NUM;
  config.pin_d1        = Y3_GPIO_NUM;
  config.pin_d2        = Y4_GPIO_NUM;
  config.pin_d3        = Y5_GPIO_NUM;
  config.pin_d4        = Y6_GPIO_NUM;
  config.pin_d5        = Y7_GPIO_NUM;
  config.pin_d6        = Y8_GPIO_NUM;
  config.pin_d7        = Y9_GPIO_NUM;
  config.pin_xclk      = XCLK_GPIO_NUM;
  config.pin_pclk      = PCLK_GPIO_NUM;
  config.pin_vsync     = VSYNC_GPIO_NUM;
  config.pin_href      = HREF_GPIO_NUM;
  config.pin_sccb_sda  = SIOD_GPIO_NUM;
  config.pin_sccb_scl  = SIOC_GPIO_NUM;
  config.pin_pwdn      = PWDN_GPIO_NUM;
  config.pin_reset     = RESET_GPIO_NUM;
  config.xclk_freq_hz  = 20000000;
  config.pixel_format  = PIXFORMAT_RGB565;

  if (psramFound()) {
    size_t freePsram = ESP.getFreePsram();
    Serial.printf("PSRAM free before cam init: %d bytes\n", freePsram);
    if (freePsram < 400000) {   // QVGA double buf needs ~300KB
      Serial.println("Not enough PSRAM for QVGA — falling back to QQVGA heap");
      config.frame_size  = FRAMESIZE_QQVGA;
      config.fb_count    = 1;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return false;
  }

  // Sensor tuning
  sensor_t* s = esp_camera_sensor_get();
  if (s) {
    s->set_brightness(s, 1);       // slight boost
    s->set_contrast(s, 1);
    s->set_saturation(s, 0);
    s->set_whitebal(s, 1);         // auto white balance
    s->set_awb_gain(s, 1);
    s->set_exposure_ctrl(s, 1);    // auto exposure
    s->set_aec2(s, 1);
    s->set_gainceiling(s, (gainceiling_t)2);
    s->set_bpc(s, 1);              // black pixel correction
    s->set_wpc(s, 1);              // white pixel correction
    s->set_raw_gma(s, 1);
    s->set_lenc(s, 1);             // lens correction
  }

  Serial.println("Camera OK");
  return true;
}

// ── Scale QVGA (320×240) → 160×120 using 2:1 box averaging ───────
// Box averaging (average 2×2 block → 1 pixel) gives much better
// quality than nearest-neighbour dropping every other pixel
//
// Called per-row to avoid needing a full scaled frame buffer
// src    : pointer to start of source frame (RGB565, big-endian)
// dstRow : output row buffer (DST_W pixels)
// srcY   : which destination row we're computing (0..DST_H-1)

void scaleRow(uint16_t* src, uint16_t* dstRow, int srcY) {
  int sy = srcY * 2;   // source row = dest row × 2
  for (int dx = 0; dx < DST_W; dx++) {
    int sx = dx * 2;

    // Four source pixels in the 2×2 block
    // Row sy, cols sx and sx+1
    // Row sy+1, cols sx and sx+1
    uint16_t p00 = src[sy       * CAM_W + sx];
    uint16_t p01 = src[sy       * CAM_W + sx + 1];
    uint16_t p10 = src[(sy + 1) * CAM_W + sx];
    uint16_t p11 = src[(sy + 1) * CAM_W + sx + 1];

    // Byte-swap each pixel (camera = big-endian, TFT = little-endian)
    p00 = __builtin_bswap16(p00);
    p01 = __builtin_bswap16(p01);
    p10 = __builtin_bswap16(p10);
    p11 = __builtin_bswap16(p11);

    // Extract RGB channels and average
    uint8_t r = (((p00 >> 11) & 0x1F) + ((p01 >> 11) & 0x1F) +
                 ((p10 >> 11) & 0x1F) + ((p11 >> 11) & 0x1F)) >> 2;
    uint8_t g = (((p00 >> 5)  & 0x3F) + ((p01 >> 5)  & 0x3F) +
                 ((p10 >> 5)  & 0x3F) + ((p11 >> 5)  & 0x3F)) >> 2;
    uint8_t b = (((p00)       & 0x1F) + ((p01)       & 0x1F) +
                 ((p10)       & 0x1F) + ((p11)       & 0x1F)) >> 2;

    dstRow[dx] = (r << 11) | (g << 5) | b;
  }
}

// ── Capture and display ───────────────────────────────────────────
void captureAndShow() {
  // Show capturing indicator
  tft.fillRect(0, 0, TFT_W, 14, ST77XX_BLACK);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(40, 3);
  tft.print("CAPTURING...");

  // Discard first frame — camera AGC needs one frame to settle
  camera_fb_t* fb = esp_camera_fb_get();
  if (fb) esp_camera_fb_return(fb);

  // Capture the settled frame
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Capture failed");
    tft.fillRect(0, 50, TFT_W, 30, ST77XX_BLACK);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(30, 60);
    tft.print("CAPTURE FAILED");
    return;
  }

  Serial.printf("Frame: %dx%d  %d bytes  (PSRAM: %d free)\n",
                fb->width, fb->height, fb->len, ESP.getFreePsram());

  uint16_t* src = (uint16_t*)fb->buf;
  bool isPSRAMFrame = (fb->width == CAM_W);

  tft.startWrite();
tft.setAddrWindow(0, Y_PAD, DST_W - 1, Y_PAD + DST_H - 1);

if (isPSRAMFrame && rowBuf) {
  // QVGA path: scale 320×240 → 160×120 row by row
  for (int dy = 0; dy < DST_H; dy++) {
    scaleRow(src, rowBuf, dy);
    for (int dx = 0; dx < DST_W; dx++) {
      tft.SPI_WRITE16(rowBuf[dx]);
    }
  }
} else {
  // QQVGA fallback: direct push
  for (int i = 0; i < DST_W * DST_H; i++) {
    uint16_t px = __builtin_bswap16(src[i]);
    tft.SPI_WRITE16(px);
  }
}

tft.endWrite();
  esp_camera_fb_return(fb);

  // Header overlay
  tft.fillRect(0, 0, TFT_W, 12, ST77XX_BLACK);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(1);
  tft.setCursor(2, 2);
  tft.print("SNAP");
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(34, 2);
  tft.printf("%dx%d  %lums", fb->width, fb->height, millis());

  Serial.println("Frame displayed");
}

// ── Idle screen ───────────────────────────────────────────────────
void drawIdleScreen() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawFastHLine(0, 15, TFT_W, ST77XX_CYAN);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(1);
  tft.setCursor(32, 4);
  tft.print("CAMERA VIEWER");

  // Camera icon placeholder
  tft.drawRoundRect(60, 40, 40, 30, 4, ST77XX_WHITE);
  tft.fillCircle(80, 55, 8, tft.color565(60, 60, 60));
  tft.drawCircle(80, 55, 8, ST77XX_WHITE);
  tft.fillCircle(80, 55, 4, ST77XX_WHITE);

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(14, 84);
  tft.print("Press BOOT to snap");

  // PSRAM status
  tft.setTextColor(psramFound() ? ST77XX_GREEN : ST77XX_YELLOW);
  tft.setCursor(4, 100);
  tft.printf("PSRAM: %s",
             psramFound() ? "OK" : "heap fallback");
  if (psramFound()) {
    tft.setCursor(4, 112);
    tft.setTextColor(ST77XX_WHITE);
    tft.printf("%.1f MB free", ESP.getFreePsram() / 1048576.0f);
  }
}

// ── Setup ─────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);

  // 1. PSRAM first
  initPSRAM();

  // 2. rowBuf from heap — it's tiny (320 bytes), save ALL PSRAM for camera
  rowBuf = (uint16_t*)malloc(DST_W * sizeof(uint16_t));
  if (!rowBuf) Serial.println("rowBuf alloc failed");
  else Serial.println("rowBuf OK (heap)");

  Serial.printf("PSRAM free before camera init: %d bytes\n", ESP.getFreePsram());

  // 3. Button
  pinMode(BOOT_BTN, INPUT_PULLUP);

  // 4. TFT
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(44, 60);
  tft.print("STARTING...");

  // 5. Camera last — needs full PSRAM available
  if (!initCamera()) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(20, 55);
    tft.print("CAMERA FAILED");
    tft.setCursor(8, 70);
    tft.printf("0x%x", esp_camera_init(nullptr));
    while (1) delay(1000);
  }

  drawIdleScreen();
  Serial.printf("Ready. Heap: %d  PSRAM: %d\n",
                ESP.getFreeHeap(), ESP.getFreePsram());
}
// ── Loop ──────────────────────────────────────────────────────────
bool     lastBtn   = HIGH;
bool     imgShown  = false;
uint32_t imgTime   = 0;
#define  IMG_HOLD_MS  10000   // return to idle after 10s

void loop() {
  bool btn = digitalRead(BOOT_BTN);

  if (lastBtn == HIGH && btn == LOW) {
    delay(30);
    if (digitalRead(BOOT_BTN) == LOW) {
      captureAndShow();
      imgShown = true;
      imgTime  = millis();
    }
  }
  lastBtn = btn;

  if (imgShown && millis() - imgTime > IMG_HOLD_MS) {
    imgShown = false;
    drawIdleScreen();
  }
}