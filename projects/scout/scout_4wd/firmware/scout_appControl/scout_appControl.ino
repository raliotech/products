// *************************
//
// RALIO TECHNOLOGIES LLP
//
// *************************

// scout_appControl.ino
// This code controls a car robot, enabling it to drive in multiple directions.
// Each directional movement controlled through the mobile app.
// The App supports buttons and voice command interface to control the robot's movements.
// The App uses UDP protocol to interface with the microcontroller board.

// Electronics needed
// Any Ralio microcontroller board: Below code is specific to Mercury v2
// 2x dc motors

// board: Mercury v3
// 
// board: Mercury v2
// uncomment line 43

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUDP.h>
#include <Adafruit_NeoPixel.h>

// program variables
uint16_t delay_prog = 1000;
int analogResolution = 8;

// WiFi credentials
const char* ssid = "SCOUT";
const char* password = "scout@2025";

// UDP setup
WiFiUDP Udp;
unsigned int localUdpPort = 8888;  // local port to listen on
char incomingPacket[255];
int cmdCode = 0;

// RGB
// #define RGB 0  // default pin RGB LED WS2812 on Mercury <- uncomment for Mercury v2
#define NUM_LEDS 1  // Number of LEDs on the Mercury
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, RGB, NEO_GRB + NEO_KHZ800);

// user functions
void init_hardware() {
  pinMode(MOTOR_1_PWM, OUTPUT);
  pinMode(MOTOR_2_PWM, OUTPUT);
  pinMode(MOTOR_1_DIR, OUTPUT);
  pinMode(MOTOR_2_DIR, OUTPUT);
}

void cruise() {
  digitalWrite(MOTOR_1_PWM, LOW);
  digitalWrite(MOTOR_1_DIR, LOW);

  digitalWrite(MOTOR_2_PWM, LOW);
  digitalWrite(MOTOR_2_DIR, LOW);
}

void forward(int pwmValue) {
  // 0 < pwmValue < 255;
  analogWrite(MOTOR_1_PWM, pwmValue);
  digitalWrite(MOTOR_1_DIR, LOW);

  analogWrite(MOTOR_2_PWM, pwmValue);
  digitalWrite(MOTOR_2_DIR, LOW);
}

void backward(int pwmValue) {
  // 0 < pwmValue < 255;
  digitalWrite(MOTOR_1_PWM, LOW);
  analogWrite(MOTOR_1_DIR, pwmValue);

  digitalWrite(MOTOR_2_PWM, LOW);
  analogWrite(MOTOR_2_DIR, pwmValue);
}

void brake() {
  digitalWrite(MOTOR_1_PWM, HIGH);
  digitalWrite(MOTOR_1_DIR, HIGH);

  digitalWrite(MOTOR_2_PWM, HIGH);
  digitalWrite(MOTOR_2_DIR, HIGH);
}

void left(int pwmValue) {
  //PWM: 0 -> Slow; 255 -> Fast
  analogWrite(MOTOR_1_PWM, int(pwmValue * 0.8));
  digitalWrite(MOTOR_1_DIR, LOW);

  analogWrite(MOTOR_2_PWM, pwmValue);
  digitalWrite(MOTOR_2_DIR, LOW);
}

void right(int pwmValue) {
  //PWM: 0 -> Slow; 255 -> Fast
  analogWrite(MOTOR_1_PWM, pwmValue);
  digitalWrite(MOTOR_1_DIR, LOW);

  analogWrite(MOTOR_2_PWM, int(pwmValue * 0.8));
  digitalWrite(MOTOR_2_DIR, LOW);
}

void left_back(int pwmValue) {
  //PWM: 0 -> Slow; 255 -> Fast
  digitalWrite(MOTOR_1_PWM, LOW);
  analogWrite(MOTOR_1_DIR, int(pwmValue * 0.8));

  digitalWrite(MOTOR_2_PWM, LOW);
  analogWrite(MOTOR_2_DIR, pwmValue);
}

void right_back(int pwmValue) {
  //PWM: 0 -> Slow; 255 -> Fast
  digitalWrite(MOTOR_1_PWM, LOW);
  analogWrite(MOTOR_1_DIR, pwmValue);

  digitalWrite(MOTOR_2_PWM, LOW);
  analogWrite(MOTOR_2_DIR, int(pwmValue * 0.8));
}

// function - recieve data from UDP sender
void receiveUdpPacket(char* message) {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // Receive incoming UDP packets
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(message, 255);
    if (len > 0) {
      message[len] = 0;  // Null-terminate the string
      //Serial.printf("UDP packet contents: %s\n", message);
    }
  }
}

void setup() {
  // Serial.begin(115200);
  init_hardware();
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid, password);
  delay(delay_prog * 1);
  Udp.begin(localUdpPort);
  // Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  strip.begin();
  strip.show();
}

void loop() {
  receiveUdpPacket(incomingPacket);
  cruise();
  if (strlen(incomingPacket) > 0) {
    // Serial.printf("Received message: %s\n", incomingPacket);
    String prefix = String(incomingPacket);

    if (prefix == "S") cmdCode = 0;
    else if (prefix == "F") cmdCode = 1;
    else if (prefix == "B") cmdCode = 2;
    else if (prefix == "L") cmdCode = 3;
    else if (prefix == "R") cmdCode = 4;
    else if (prefix == "LF") cmdCode = 5;
    else if (prefix == "RF") cmdCode = 6;
    else if (prefix == "LB") cmdCode = 7;
    else if (prefix == "RB") cmdCode = 8;

    switch (cmdCode) {
      case 0:
        // stop
        cruise();
        for (int i = 0; i < NUM_LEDS; i++) {
          strip.setPixelColor(i, 255, 0, 0);
        }
        strip.show();
        break;
      case 1:
        forward(255);
        delay(50);
        for (int i = 0; i < NUM_LEDS; i++) {
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
        break;
      case 2:
        backward(224);
        delay(50);
        for (int i = 0; i < NUM_LEDS; i++) {
          strip.setPixelColor(i, 255, 255, 255);
        }
        strip.show();
        break;
      case 3:
        left(224);
        for (int i = 0; i < NUM_LEDS / 2; i++) {
          strip.setPixelColor(i, 0, 0, 0);
        }
        for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) {
          strip.setPixelColor(i, 250, 90, 0);
        }
        strip.show();
        break;
      case 4:
        right(224);
        for (int i = 0; i < NUM_LEDS / 2; i++) {
          strip.setPixelColor(i, 250, 90, 0);
        }
        for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) {
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
        break;
      case 5:
        //left-fwd
        left(224);
        for (int i = 0; i < NUM_LEDS / 2; i++) {
          strip.setPixelColor(i, 0, 0, 0);
        }
        for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) {
          strip.setPixelColor(i, 250, 90, 0);
        }
        strip.show();
        break;
      case 6:
        //right-fwd
        right(224);
        for (int i = 0; i < NUM_LEDS / 2; i++) {
          strip.setPixelColor(i, 250, 90, 0);
        }
        for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) {
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
        break;
      case 7:
        //left-back
        left_back(200);
        for (int i = 0; i < NUM_LEDS; i++) {
          strip.setPixelColor(i, 255, 255, 255);
        }
        strip.show();
        break;
      case 8:
        //right-back
        right_back(200);
        for (int i = 0; i < NUM_LEDS; i++) {
          strip.setPixelColor(i, 255, 255, 255);
        }
        strip.show();
        break;

      // Add more cases as needed
      default:
        // Serial.println("Unknown command");
        break;
    }
  }
}

// void loop() {
//   receiveUdpPacket(incomingPacket);
//   cruise();
//   if (strlen(incomingPacket) > 0) {
//     // Serial.printf("Received message: %s\n", incomingPacket);
//     switch (incomingPacket[0]) {
//       case 'F':
//         forward(225);
//         delay(50);
//         strip.setPixelColor(0, 0, 0, 0);
//         strip.show();
//         break;
//       case 'S':
//         cruise();
//         strip.setPixelColor(0, 255, 0, 0);
//         strip.show();
//         break;
//       case 'L':
//         left(225);
//         strip.setPixelColor(0, 220, 40, 10);
//         strip.show();
//         break;
//       case 'R':
//         right(225);
//         strip.setPixelColor(0, 220, 40, 10);
//         strip.show();
//         break;
//       case 'B':
//         backward(200);
//         delay(50);
//         strip.setPixelColor(0, 255, 255, 255);
//         strip.show();
//         break;
//       // Add more cases as needed
//       default:
//         // Serial.println("Unknown command");
//         break;
//     }
//   }
// }