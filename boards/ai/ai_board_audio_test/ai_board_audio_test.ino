#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "driver/i2s_pdm.h"

// ── Config ───────────────────────────────────────────────────────
const char* WIFI_SSID     = "JioFiber-2g";
const char* WIFI_PASS     = "malanaimahit";
const char* WHISPER_URL   = "http://192.168.29.63:8080/inference"; // your PC IP

#define RECORD_SECONDS    3       // how long to record each clip
#define SAMPLE_RATE       16000
#define SAMPLE_BITS       16
#define RECORD_SAMPLES    (SAMPLE_RATE * RECORD_SECONDS)
#define RECORD_BYTES      (RECORD_SAMPLES * (SAMPLE_BITS / 8))

// ── Pins ─────────────────────────────────────────────────────────
#define TFT_CS   -1
#define TFT_RST   38
#define TFT_DC    36
#define TFT_MOSI  37
#define TFT_SCLK  35
#define LED_PIN    3
#define NUM_LEDS   1
#define MIC_CLK   14
#define MIC_DATA  21
const int MOTOR_EN = 38;
const int M1_IN1 = 39, M1_IN2 = 40;
const int M2_IN1 = 45, M2_IN2 = 46;

// ── Objects ──────────────────────────────────────────────────────
Adafruit_ST7735  tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_NeoPixel pixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
i2s_chan_handle_t rx_chan;

// Audio buffer lives in PSRAM
int16_t* recBuf = nullptr;

// ── State ────────────────────────────────────────────────────────
enum MotorState { MOT_STOP, MOT_FWD, MOT_BWD, MOT_LEFT, MOT_RIGHT };
MotorState motorState = MOT_STOP;
uint32_t   ledColor   = 0;

// ── Motor ────────────────────────────────────────────────────────
void setMotor(MotorState s) {
  motorState = s;
  switch (s) {
    case MOT_FWD:
      analogWrite(M1_IN1,200); analogWrite(M1_IN2,0);
      analogWrite(M2_IN1,200); analogWrite(M2_IN2,0);   break;
    case MOT_BWD:
      analogWrite(M1_IN1,0);   analogWrite(M1_IN2,200);
      analogWrite(M2_IN1,0);   analogWrite(M2_IN2,200); break;
    case MOT_LEFT:
      analogWrite(M1_IN1,0);   analogWrite(M1_IN2,150);
      analogWrite(M2_IN1,150); analogWrite(M2_IN2,0);   break;
    case MOT_RIGHT:
      analogWrite(M1_IN1,150); analogWrite(M1_IN2,0);
      analogWrite(M2_IN1,0);   analogWrite(M2_IN2,150); break;
    case MOT_STOP:
    default:
      analogWrite(M1_IN1,0); analogWrite(M1_IN2,0);
      analogWrite(M2_IN1,0); analogWrite(M2_IN2,0);     break;
  }
}

// ── LED ──────────────────────────────────────────────────────────
void setLED(uint8_t r, uint8_t g, uint8_t b) {
  ledColor = pixel.Color(r, g, b);
  pixel.setPixelColor(0, ledColor);
  pixel.setBrightness(ledColor ? 120 : 0);
  pixel.show();
}

// ── WAV header builder ───────────────────────────────────────────
// whisper.cpp /inference endpoint expects a proper WAV file
void buildWavHeader(uint8_t* hdr, uint32_t dataBytes) {
  uint32_t fileSize   = dataBytes + 36;
  uint32_t byteRate   = SAMPLE_RATE * 1 * (SAMPLE_BITS / 8);
  uint16_t blockAlign = 1 * (SAMPLE_BITS / 8);

  memcpy(hdr,      "RIFF", 4);
  memcpy(hdr + 4,  &fileSize,   4);
  memcpy(hdr + 8,  "WAVEfmt ", 8);
  uint32_t subchunk1 = 16;       memcpy(hdr + 16, &subchunk1,  4);
  uint16_t audioFmt  = 1;        memcpy(hdr + 20, &audioFmt,   2);
  uint16_t channels  = 1;        memcpy(hdr + 22, &channels,   2);
  uint32_t sr        = SAMPLE_RATE; memcpy(hdr + 24, &sr,       4);
  memcpy(hdr + 28, &byteRate,   4);
  memcpy(hdr + 32, &blockAlign, 2);
  uint16_t bps       = SAMPLE_BITS; memcpy(hdr + 34, &bps,     2);
  memcpy(hdr + 36, "data",      4);
  memcpy(hdr + 40, &dataBytes,  4);
}

// ── TFT ──────────────────────────────────────────────────────────
void drawHeader() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(1);
  tft.setCursor(22, 4);
  tft.print("WHISPER CONTROL");
  tft.drawFastHLine(0, 15, 160, ST77XX_CYAN);
}

void showStatus(const char* msg, uint16_t col) {
  tft.fillRect(0, 18, 160, 14, ST77XX_BLACK);
  tft.fillRoundRect(4, 18, 152, 12, 3, col);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  // centre the text
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((160 - w) / 2, 20);
  tft.print(msg);
}

void showTranscript(String txt) {
  tft.fillRect(0, 34, 160, 28, ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(4, 35);
  tft.print("Heard:");
  tft.setCursor(4, 46);
  // truncate to fit display (approx 26 chars at size 1)
  if (txt.length() > 26) txt = txt.substring(0, 26);
  tft.print(txt);
}

void showCommand(const char* cmd) {
  tft.fillRect(0, 65, 160, 14, ST77XX_BLACK);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(4, 66);
  tft.print("CMD: ");
  tft.setTextColor(ST77XX_WHITE);
  tft.print(cmd);
}

void showMotorArrow(MotorState s) {
  tft.fillRect(0, 82, 80, 28, ST77XX_BLACK);
  int cx = 24, cy = 96;
  uint16_t col = ST77XX_GREEN;
  switch (s) {
    case MOT_FWD:   tft.fillTriangle(cx, cy-8, cx-6, cy+4, cx+6, cy+4, col); break;
    case MOT_BWD:   tft.fillTriangle(cx, cy+8, cx-6, cy-4, cx+6, cy-4, col); break;
    case MOT_LEFT:  tft.fillTriangle(cx-8, cy, cx+4, cy-6, cx+4, cy+6, col); break;
    case MOT_RIGHT: tft.fillTriangle(cx+8, cy, cx-4, cy-6, cx-4, cy+6, col); break;
    case MOT_STOP:  tft.fillRect(cx-5, cy-5, 10, 10, ST77XX_RED);            break;
  }
}

void showLEDSwatch(uint32_t color) {
  tft.fillRect(84, 82, 76, 28, ST77XX_BLACK);
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8)  & 0xFF;
  uint8_t b =  color        & 0xFF;
  if (color == 0) {
    tft.drawRect(90, 86, 60, 18, ST77XX_WHITE);
    tft.setTextColor(ST77XX_WHITE); tft.setTextSize(1);
    tft.setCursor(107, 91); tft.print("OFF");
  } else {
    tft.fillRect(90, 86, 60, 18, tft.color565(r, g, b));
  }
}

// ── Command parser ───────────────────────────────────────────────
// Converts Whisper transcript → action
// Simple keyword matching — extend as needed
void parseAndExecute(String transcript) {
  transcript.toLowerCase();
  transcript.trim();

  Serial.println("Transcript: " + transcript);
  showTranscript(transcript);

  const char* cmdLabel = "unknown";

  // Motor commands
  if (transcript.indexOf("forward")  >= 0) { setMotor(MOT_FWD);   cmdLabel = "FORWARD"; }
  else if (transcript.indexOf("backward") >= 0 ||
           transcript.indexOf("reverse")  >= 0) { setMotor(MOT_BWD); cmdLabel = "BACKWARD"; }
  else if (transcript.indexOf("turn left")  >= 0 ||
           transcript.indexOf("go left")    >= 0) { setMotor(MOT_LEFT);  cmdLabel = "LEFT"; }
  else if (transcript.indexOf("turn right") >= 0 ||
           transcript.indexOf("go right")   >= 0) { setMotor(MOT_RIGHT); cmdLabel = "RIGHT"; }
  else if (transcript.indexOf("stop") >= 0) { setMotor(MOT_STOP); cmdLabel = "STOP"; }

  // LED colour commands
  else if (transcript.indexOf("red")    >= 0) { setLED(255,   0,   0); cmdLabel = "LED RED"; }
  else if (transcript.indexOf("green")  >= 0) { setLED(  0, 255,   0); cmdLabel = "LED GREEN"; }
  else if (transcript.indexOf("blue")   >= 0) { setLED(  0,   0, 255); cmdLabel = "LED BLUE"; }
  else if (transcript.indexOf("white")  >= 0) { setLED(255, 255, 255); cmdLabel = "LED WHITE"; }
  else if (transcript.indexOf("yellow") >= 0) { setLED(255, 200,   0); cmdLabel = "LED YELLOW"; }
  else if (transcript.indexOf("purple") >= 0 ||
           transcript.indexOf("violet") >= 0) { setLED(180,   0, 255); cmdLabel = "LED PURPLE"; }
  else if (transcript.indexOf("led off")  >= 0 ||
           transcript.indexOf("light off") >= 0) { setLED(0, 0, 0); cmdLabel = "LED OFF"; }

  showCommand(cmdLabel);
  showMotorArrow(motorState);
  showLEDSwatch(ledColor);
}

// ── I2S PDM init ─────────────────────────────────────────────────
void initMic() {
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  i2s_new_channel(&chan_cfg, NULL, &rx_chan);

  i2s_pdm_rx_config_t pdm_cfg = {
    .clk_cfg  = I2S_PDM_RX_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
    .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(
                  I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
    .gpio_cfg = {
      .clk = (gpio_num_t)MIC_CLK,
      .din = (gpio_num_t)MIC_DATA,
      .invert_flags = { .clk_inv = false }
    }
  };
  i2s_channel_init_pdm_rx_mode(rx_chan, &pdm_cfg);
  i2s_channel_enable(rx_chan);
}

// ── Record audio → PSRAM buffer ──────────────────────────────────
void recordAudio() {
  showStatus("  RECORDING...  ", ST77XX_RED);
  size_t bytesRead = 0;
  size_t totalRead = 0;

  while (totalRead < RECORD_BYTES) {
    size_t toRead = min((size_t)1024, RECORD_BYTES - totalRead);
    i2s_channel_read(rx_chan, (uint8_t*)recBuf + totalRead,
                     toRead, &bytesRead, portMAX_DELAY);
    totalRead += bytesRead;
  }
  Serial.printf("Recorded %d bytes\n", totalRead);
}

// ── Send WAV to whisper.cpp server ───────────────────────────────
String transcribeAudio() {
  showStatus("  SENDING...    ", tft.color565(255, 140, 0));

  // Build WAV header
  uint8_t wavHdr[44];
  buildWavHeader(wavHdr, RECORD_BYTES);

  // Build boundary strings
  String boundary = "ESP32Boundary";
  String bodyHead = "--" + boundary + "\r\n"
                    "Content-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\n"
                    "Content-Type: audio/wav\r\n\r\n";
  String bodyTail = "\r\n--" + boundary + "--\r\n";

  uint32_t totalLen = bodyHead.length() + 44 + RECORD_BYTES + bodyTail.length();

  // Allocate POST buffer on heap (we know heap works)
  uint8_t* postBuf = (uint8_t*)malloc(totalLen);
  if (!postBuf) {
    Serial.println("malloc failed for POST buffer");
    showStatus("  MEM ERROR     ", ST77XX_RED);
    delay(2000);
    return "";
  }

  // Fill buffer
  uint32_t pos = 0;
  memcpy(postBuf + pos, bodyHead.c_str(), bodyHead.length()); pos += bodyHead.length();
  memcpy(postBuf + pos, wavHdr, 44);                          pos += 44;
  memcpy(postBuf + pos, recBuf, RECORD_BYTES);                pos += RECORD_BYTES;
  memcpy(postBuf + pos, bodyTail.c_str(), bodyTail.length()); pos += bodyTail.length();

  Serial.printf("POST buffer size: %d bytes\n", totalLen);
  Serial.printf("Free heap before POST: %d\n", ESP.getFreeHeap());

  HTTPClient http;
  http.begin(WHISPER_URL);
  http.setTimeout(20000);
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

  int code = http.POST(postBuf, totalLen);
  free(postBuf);

  String result = "";
  if (code == 200) {
    String body = http.getString();
    Serial.println("Response: " + body);

    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (!err && doc.containsKey("text")) {
      result = doc["text"].as<String>();
      result.trim();
      Serial.println("Transcript: " + result);
    } else {
      Serial.println("JSON parse error or no 'text' key");
      Serial.println("Raw: " + body);
    }
  } else {
    Serial.printf("HTTP error: %d\n", code);
    showStatus("  HTTP ERROR     ", ST77XX_RED);
    delay(2000);
  }

  http.end();
  return result;
}
// ── Setup ────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);

   // Manually init PSRAM before anything else
  if (!psramInit()) {
    Serial.println("PSRAM init failed!");
  } else {
    Serial.printf("PSRAM OK: %d bytes free\n", ESP.getFreePsram());
  }

  // Test ps_malloc explicitly
  void* test = ps_malloc(96000);
  if (!test) {
    Serial.println("ps_malloc FAILED even after psramInit");
    // Fall back to regular malloc
    recBuf = (int16_t*)malloc(RECORD_BYTES);
    Serial.println("Falling back to heap malloc");
  } else {
    free(test);
    recBuf = (int16_t*)ps_malloc(RECORD_BYTES);
    Serial.println("ps_malloc OK — using PSRAM buffer");
  }

  if (!recBuf) {
    Serial.println("FATAL: no memory available at all");
    while(1) delay(1000);
  }
  delay(1000);
  // 1. Motors first (just pin modes, instant)
  pinMode(MOTOR_EN, OUTPUT);
  digitalWrite(MOTOR_EN, HIGH);
  setMotor(MOT_STOP);

  // 2. LED
  pixel.begin();
  pixel.setBrightness(0);
  pixel.show();

  // 3. WiFi — do this BEFORE TFT init
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi FAILED");
    while(1) delay(1000);
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());

  // 4. TFT — after WiFi is stable
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  drawHeader();
  showStatus("  CONNECTED     ", ST77XX_GREEN);
  showMotorArrow(MOT_STOP);
  showLEDSwatch(0);

  

  // 6. Mic last
  initMic();

  showStatus("  READY         ", ST77XX_GREEN);
  showTranscript("say a command...");
  Serial.println("Ready. Recording starts now.");

  
}

// ── Loop ─────────────────────────────────────────────────────────
void loop() {
  // Continuously record → transcribe → execute → repeat
  recordAudio();

  String transcript = transcribeAudio();

  if (transcript.length() > 0) {
    parseAndExecute(transcript);
    showStatus("  READY         ", ST77XX_GREEN);
  } else {
    showStatus("  READY         ", ST77XX_GREEN);
    showTranscript("(nothing heard)");
  }

  // Small gap before next recording
  delay(300);
}