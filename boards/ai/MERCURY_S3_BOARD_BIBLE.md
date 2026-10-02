# Mercury S3 AI — Board Bible
> **Ralio Technologies | Project: 100032-02-A | Rev: 01 | Last Updated: 2026-03**
> This is the single source of truth for all firmware, hardware, and tutorial development.

---

## 1. System Summary

| Field | Value |
|---|---|
| Board Name | Mercury S3 AI |
| MCU | ESP32-S3-WROOM-1-N16R8 (16MB Flash, 8MB PSRAM) |
| USB VID/PID | 0xFFFF / 0x7EA1 |
| USB Product | "Mercury S3 Ai" |
| Framework | Arduino on ESP-IDF |
| Target Use Cases | Edge Vision, Voice Assistant, Robotics, TinyML |
| Schematic Rev | 100032-02-A |
| Engineer | Gaurang |

---

## 2. GPIO Master Map

### Camera (OV2640 — DVP Interface)

| Signal | ESP32-S3 GPIO | Notes |
|---|---|---|
| DVP_VSYNC | IO6 | Frame sync |
| DVP_HREF | IO7 | Line valid |
| DVP_XCLK | IO15 | Clock OUT to sensor |
| DVP_PCLK | IO13 | Pixel clock IN |
| DVP_Y9 | IO16 | Data bit 9 (MSB) |
| DVP_Y8 | IO17 | Data bit 8 |
| DVP_Y7 | IO18 | Data bit 7 |
| DVP_Y6 | IO12 | Data bit 6 |
| DVP_Y5 | IO10 | Data bit 5 |
| DVP_Y4 | IO8 | Data bit 4 |
| DVP_Y3 | IO9 | Data bit 3 |
| DVP_Y2 | IO11 | Data bit 2 (LSB) |
| CAM_I2C_SDA | IO47 (via R27, 0Ω) | Camera SCCB — connects to I2C_SDA_1 |
| CAM_I2C_SCL | IO48 (via R25, 0Ω) | Camera SCCB — connects to I2C_SCL_1 |

> **⚠️ Critical:** Camera uses I2C_1 (IO47/IO48), NOT the primary I2C bus. R24–R27 are 0Ω bridging resistors.  
> **⚠️ Power Rails:** AVDD=2V8, DVDD=1V5, DOVDD=3V3 — all generated on-board (verify power page).  

### Display (HS180S10B — SPI)

| Signal | ESP32-S3 GPIO | Notes |
|---|---|---|
| LCD_CLK | IO35 | SPI Clock |
| LCD_MOSI | IO37 | SPI Data |
| LCD_DC | IO36 | Data/Command select |
| LCD_RST | IO38 (shared M_EN) | **REWORK** — see note below |
| LCD_FCS | Tied via R19 | Frame CS |
| LCD_BLK | Via R17 | Backlight control |

> **Note:** Display uses a CNI connector (CN1_4SPI). LCD_FCS has 10k pull-up.  
> **⚠️ REWORK (Rev 01 hardware):** LCD_RST was originally pulled up via R18 — this failed display bring-up. LCD_RST is now hardwired to IO38 (M_EN). Firmware **must** assert IO38 LOW briefly during LCD init, then HIGH permanently. IO38 HIGH = Motor driver awake + LCD out of reset. This pin must NEVER be driven LOW during normal operation. Will be separated into dedicated GPIO in next schematic revision.

### Microphone (LMD3526T261-OA5 — PDM)

| Signal | ESP32-S3 GPIO | Notes |
|---|---|---|
| MIC_CLK | IO14 | PDM Clock |
| MIC_D0 | IO21 | PDM Data |

> L/R pin tied to GND → Left channel selected.

### Motor Driver (DRV8833 equivalent, U6)

| Signal | ESP32-S3 GPIO | Notes |
|---|---|---|
| M_EN (nSLEEP) | IO38 | **Shared with LCD_RST (REWORK)** — keep HIGH after LCD init |
| M_A1 (AIN1) | IO45 | Motor A direction 1 |
| M_A2 (AIN2) | IO46 | Motor A direction 2 |
| M_B1 (BIN1) | IO39 | Motor B direction 1 |
| M_B2 (BIN2) | IO40 | Motor B direction 2 |

> Motor connectors: J10 (Motor 1), J11 (Motor 2).  
> 4.7k pull-down (R28) on FAULT pin. ASEN/BSEN for current sensing — not yet routed to MCU.

### RGB LED (WS2812 / NeoPixel)

| Signal | ESP32-S3 GPIO | Notes |
|---|---|---|
| RGB_IN | IO3 | Single-wire NeoPixel data |

> D2 is the LED (VDD=5V, driven via 220Ω R9). 2.2µF decoupling on LED power.

### Onboard LED & Button

| Signal | GPIO | Notes |
|---|---|---|
| LED_BUILTIN | IO0 | Also BOOT button |
| BTN | IO0 | Boot/User button |

### I2C Buses

| Bus | SDA | SCL | Usage |
|---|---|---|---|
| I2C Primary (Wire) | IO5 (A5) | IO4 (A4) | General peripherals |
| I2C_1 (Wire1) | IO47 | IO48 | Camera SCCB |

> R22/R23 are 4.7k pull-ups on CAM I2C. R20/R21 are 10k pull-ups on display connector.

### Analog Pins

| Label | GPIO |
|---|---|
| A1 | IO1 |
| A2 | IO2 |
| A4 | IO4 |
| A5 | IO5 |

### UART

| Signal | GPIO |
|---|---|
| TX | IO43 |
| RX | IO44 |

### Free GPIO (Expansion Connectors)

| GPIO | Notes |
|---|---|
| IO1 / IO2 | Available on J3 connector, also A1/A2 |
| IO41 / IO42 | Available on J6 connector |
| IO1, IO2, TX, RX, IO41, IO42 | Broken out on 6x JST-style connectors (J1–J6) |

---

## 3. Reserved / DO NOT USE Pins

| GPIO | Reason |
|---|---|
| IO0 | BOOT strap — use with caution, is BTN |
| IO43 / IO44 | UART0 TX/RX — USB/debug serial |
| IO19 / IO20 | USB D- / D+ |
| IO3 | NeoPixel — already driven by LED driver |
| IO35 | LCD CLK — do not reassign |

---

## 4. Power Tree

```
USB 5V ──┬── 5V Rail → Motor Driver VIN, NeoPixel VDD
         └── LDO → 3V3 Rail → MCU VDD, Display DOVDD, Mic VDD
                            → Camera: 
                              ├── 3V3 → DOVDD
                              ├── LDO → 2V8 → AVDD
                              └── LDO → 1V5 → DVDD
```

> **⚠️ Unknown:** Camera 2V8 and 1V5 LDOs not visible in provided schematic pages. Need power page schematic to confirm regulators and sequencing.

---

## 5. Peripheral Ownership Table

| Peripheral | Bus/Interface | Driver File (planned) | Status |
|---|---|---|---|
| OV2640 Camera | DVP + I2C_1 | `drivers/camera/ov2640.cpp` | ⬜ Not started |
| HS180S10B Display | SPI (bit-bang or hw) | `drivers/display/hs180s10b.cpp` | ⬜ Not started |
| LMD3526 Microphone | PDM I2S | `drivers/audio/pdm_mic.cpp` | ⬜ Not started |
| Motor Driver | GPIO PWM | `drivers/motor/drv_motor.cpp` | ⬜ Not started |
| NeoPixel LED | RMT / WS2812 | `drivers/led/neopixel.cpp` | ⬜ Not started |
| Boot Button | GPIO Input | `drivers/io/button.cpp` | ⬜ Not started |

---

## 6. Firmware Architecture

### Framework Choice: Arduino on ESP-IDF
**Why:** ESP-IDF gives access to FreeRTOS, camera DVP driver, I2S PDM, and DMA. Arduino layer keeps it beginner-friendly for tutorials.

### Folder Structure

```
mercury-s3-firmware/
├── boards/
│   └── mercury_s3/
│       └── pins_arduino.h          ← GPIO definitions
├── drivers/
│   ├── camera/
│   │   ├── ov2640.h / .cpp
│   │   └── camera_config.h         ← resolution, format presets
│   ├── display/
│   │   ├── hs180s10b.h / .cpp
│   │   └── display_fonts/
│   ├── audio/
│   │   └── pdm_mic.h / .cpp
│   ├── motor/
│   │   └── drv_motor.h / .cpp
│   └── led/
│       └── neopixel.h / .cpp
├── hal/
│   ├── power.h                     ← power management helpers
│   └── system_init.h               ← boot sequence, clock init
├── ml/
│   ├── tflite_wrapper.h / .cpp
│   └── models/                     ← .tflite model binaries
├── examples/
│   ├── 00_bringup/
│   ├── 01_camera_stream/
│   ├── 02_display_hello/
│   ├── 03_microphone_test/
│   ├── 04_motor_control/
│   ├── 05_neopixel/
│   ├── 06_face_detect/
│   ├── 07_keyword_detect/
│   └── 08_robot_vision/
├── docs/
│   ├── BOARD_BIBLE.md              ← THIS FILE
│   ├── HARDWARE_ERRATA.md
│   └── TUTORIAL_CHECKLIST.md
├── tests/
│   └── unit/
└── platformio.ini / arduino_boards/
```

### Coding Standards

- **Language:** C++ (Arduino-style for examples, proper C++ for drivers)
- **Naming:** `snake_case` for variables/functions, `PascalCase` for classes
- **Constants:** `#define` or `constexpr` — never magic numbers
- **Pin references:** Always use `pins_arduino.h` constants — never raw GPIO numbers
- **Error handling:** Every peripheral init MUST return a bool or error code
- **Serial logging:** Use `#define LOG_TAG "MODULE_NAME"` + macro wrapper for log levels
- **Comments:** All public functions need a doc comment

### Logging Convention

```cpp
#define LOGI(tag, fmt, ...) Serial.printf("[INFO][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define LOGE(tag, fmt, ...) Serial.printf("[ERROR][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define LOGD(tag, fmt, ...) if(DEBUG_ENABLE) Serial.printf("[DEBUG][%s] " fmt "\n", tag, ##__VA_ARGS__)
```

---

## 7. Development Roadmap

### Phase 0 — Board Bring-Up (Week 1)
- [ ] Flash "Hello World" + blink LED_BUILTIN
- [ ] Confirm USB serial works
- [ ] NeoPixel color test
- [ ] I2C bus scan (both buses)
- [ ] Button interrupt test

### Phase 1 — Peripheral Validation (Week 1–2)
- [ ] Display: white screen → text → image
- [ ] Camera: I2C init → QVGA frame capture
- [ ] Microphone: PDM I2S init → FFT sanity
- [ ] Motor Driver: enable → fwd/rev/brake per channel
- [ ] Analog read on A1/A2

### Phase 2 — Driver Development (Week 2–3)
- [ ] OV2640 driver with resolution presets (QQVGA, QVGA, VGA)
- [ ] Display driver with Adafruit GFX compatibility layer
- [ ] PDM mic driver with circular DMA buffer
- [ ] Motor driver with speed/direction API

### Phase 3 — Demo Projects (Week 3–4)
- [ ] Camera → Display live viewfinder (marketable!)
- [ ] Clap detection via microphone
- [ ] Line follower robot (motors + camera)
- [ ] WiFi camera stream to browser

### Phase 4 — AI/ML Examples (Month 2)
- [ ] Face detection (ESP-WHO / TFLite)
- [ ] Keyword spotting ("Hey Mercury")
- [ ] Color/object classification
- [ ] Edge impulse integration tutorial

### Phase 5 — Advanced (Month 3+)
- [ ] VLA-style vision + action pipeline
- [ ] Multi-task FreeRTOS architecture
- [ ] OTA firmware update example
- [ ] BLE + WiFi concurrent demo

---

## 8. Known Issues / Hardware Errata

| # | Issue | Severity | Status |
|---|---|---|---|
| 1 | LCD_RST shared with M_EN (IO38) — rework on Rev 01 boards | HIGH | ✅ Workaround in firmware: pulse LOW during init, then keep HIGH |
| 2 | FAULT pin on motor driver not connected to MCU | MEDIUM | Not monitored in current design |
| 3 | ASEN/BSEN motor current sense pins floating | LOW | Not used |
| 4 | Boot button shared with IO0 and LED_BUILTIN | INFO | Guard against driving IO0 HIGH at boot |

---

## 9. Libraries Installed (Arduino IDE)

| Library | Version | Usage |
|---|---|---|
| Adafruit GFX | 1.12.6 | Display graphics core |
| Adafruit NeoPixel | 1.15.4 | RGB LED (update to 1.15.5 pending) |
| Adafruit ST7735/ST7789 | 1.11.0 | **ST7735 confirmed** — primary display driver |

> **ESP32-S3 Arduino Core:** v2.0.18-arduino5

---

## 10. Tutorial Checklist (per example)

- [ ] Clear title + one-line description
- [ ] Hardware required list
- [ ] Wiring diagram (even if on-board)
- [ ] Code with inline comments explaining WHY
- [ ] Serial monitor expected output
- [ ] "What just happened?" explanation section
- [ ] "Going further" suggestions
- [ ] Hackster-ready cover image
- [ ] GitHub README.md
- [ ] Tested on fresh flash

---

## 11. Outstanding Questions / Blockers

1. **Display controller** — ✅ Confirmed: **ST7735**. Adafruit ST7735 library already installed (v1.11.0). Ready to drive.
2. **Power schematic** — ✅ Not needed. Power tree is tested and working. Camera rails confirmed stable.
3. **Camera lens/module** — ✅ Irrelevant to firmware. OV2640 module confirmed.
4. **PlatformIO board definition** — ⬜ To be created when needed.
5. **ESP32-S3 Arduino core** — ✅ **v2.0.18-arduino5**. Camera DVP API confirmed available in this version.

---

## 12. "AI Board" Feature Scope

| Capability | Hardware | Status |
|---|---|---|
| Edge Vision | OV2640 + ESP32-S3 PSRAM | Hardware ready |
| Voice Assistant | PDM Mic + ESP32-S3 DSP | Hardware ready |
| Robotics | Motor Driver + Camera | Hardware ready |
| TinyML | ESP32-S3 + 8MB PSRAM | Hardware ready |
| WiFi/BLE Connectivity | ESP32-S3 onboard | Native |
| On-device LLM | NOT supported | Needs external processing |

---

*Last updated: March 2026 | Maintained by: Ralio Technologies | Ganapati Bappa Morya 🙏*
