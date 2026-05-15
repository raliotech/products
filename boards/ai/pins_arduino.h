#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

#define USB_VID          0xFFFF
#define USB_PID          0x7EA1
#define USB_PRODUCT      "Mercury S3 Ai"
#define USB_SERIAL       ""

// Camera
static const uint8_t DVP_VSYNC = 6;
static const uint8_t DVP_HREF = 7;
static const uint8_t DVP_XCLK = 15;
static const uint8_t DVP_Y9 = 16;
static const uint8_t DVP_Y8 = 17;
static const uint8_t DVP_Y7 = 18;
static const uint8_t DVP_Y4 = 8;
static const uint8_t DVP_Y3 = 9;
static const uint8_t DVP_Y5 = 10;
static const uint8_t DVP_Y2 = 11;
static const uint8_t DVP_Y6 = 12;
static const uint8_t DVP_PCLK = 13;

// LED
static const uint8_t RGB_IN = 3;
#define LED_BUILTIN 0
#define BUILTIN_LED LED_BUILTIN

// BUTTON
static const uint8_t BTN = 0;

// MIC
static const uint8_t MIC_CLK = 14;
static const uint8_t MIC_D0 = 21;

// I2C
#define WIRE1_PIN_DEFINED 1
static const uint8_t SCL = 48;
static const uint8_t SDA = 47;
//static const uint8_t SCL = 4;
//static const uint8_t SDA = 5;

// PERIPHERAL ENABLE
// used to enable Motor Driver & Reset control for OLED
static const uint8_t M_EN = 38;

// Motor Driver
static const uint8_t M_A2 = 46;
static const uint8_t M_A1 = 45;
static const uint8_t M_B1 = 39;
static const uint8_t M_B2 = 40;

//OLED
static const uint8_t LCD_CLK = 35;
static const uint8_t LCD_DC = 36;
static const uint8_t LCD_MOSI = 37;

// ANALOG PINS
static const uint8_t A1 = 1;
static const uint8_t A2 = 2;
static const uint8_t A4 = 4;
static const uint8_t A5 = 5;

// OTHER GPIO
// IO41; IO42;
static const uint8_t TX = 43;
static const uint8_t RX = 44;

#endif /* Pins_Arduino_h */
