#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

// --- Display Pins (Using your confirmed working setup) ---
#define TFT_RST    38  
#define TFT_DC     36
#define TFT_MOSI   37
#define TFT_SCLK   35
#define TFT_CS     -1  // CS tied to GND

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// --- Ball Variables ---
int ballX = 80;        // Start X position (center)
int ballY = 64;        // Start Y position (center)
int ballDX = 2;        // Speed in X direction
int ballDY = 2;        // Speed in Y direction
int ballRadius = 5;
uint16_t ballColor = ST77XX_GREEN;

// Screen dimensions for 1.8" TFT in rotation 1
const int screenWidth = 160;
const int screenHeight = 128;

void setup() {
  tft.initR(INITR_BLACKTAB);   // Initialize display
  tft.setRotation(1);          // Set to landscape mode
  tft.fillScreen(ST77XX_BLACK);
  
  drawStaticShapes();
}

void drawStaticShapes() {
  // 1. Draw Text
  tft.setCursor(10, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print("DISPLAY TEST MODE");

  // 2. Draw Shapes (Fixed)
  tft.drawRect(5, 5, 150, 118, ST77XX_CYAN);        // Border
  tft.drawCircle(30, 100, 10, ST77XX_MAGENTA);     // Static Circle
  tft.fillRect(120, 90, 20, 20, ST77XX_ORANGE);    // Static Square
}

void loop() {
  // --- Animation Logic ---

  // 1. Erase the old ball (draw black over it)
  tft.fillCircle(ballX, ballY, ballRadius, ST77XX_BLACK);

  // 2. Update position
  ballX += ballDX;
  ballY += ballDY;

  // 3. Collision Detection with screen edges
  // Left or Right wall
  if (ballX <= (5 + ballRadius) || ballX >= (155 - ballRadius)) {
    ballDX = -ballDX;           // Reverse direction
    ballColor = randomColor();  // Change color
  }

  // Top or Bottom wall
  if (ballY <= (5 + ballRadius) || ballY >= (123 - ballRadius)) {
    ballDY = -ballDY;           // Reverse direction
    ballColor = randomColor();  // Change color
  }

  // 4. Draw the new ball
  tft.fillCircle(ballX, ballY, ballRadius, ballColor);

  // 5. Small delay to control speed
  delay(20); 
}

// Helper function to pick a random color on collision
uint16_t randomColor() {
  uint16_t colors[] = {ST77XX_RED, ST77XX_YELLOW, ST77XX_GREEN, ST77XX_MAGENTA, ST77XX_ORANGE};
  return colors[random(0, 5)];
}