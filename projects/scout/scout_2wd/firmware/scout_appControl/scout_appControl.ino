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

// board: Mercury v2

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUDP.h>
#include <Adafruit_NeoPixel.h>

// program variables
uint16_t delay_prog = 1000;
int analogResolution = 8;

// WiFi credentials
const char* ssid = "SCOUT-2WD";
const char* password = "scout@2025";

// UDP setup
WiFiUDP Udp;
unsigned int localUdpPort = 8888;  // local port to listen on
char incomingPacket[255];

#define RGB 0  // default pin RGB LED WS2812 on Mercury
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
  digitalWrite(MOTOR_1_PWM, LOW);
  digitalWrite(MOTOR_1_DIR, LOW);

  analogWrite(MOTOR_2_PWM, pwmValue);
  digitalWrite(MOTOR_2_DIR, LOW);
}

void right(int pwmValue) {
  //PWM: 0 -> Slow; 255 -> Fast
  analogWrite(MOTOR_1_PWM, pwmValue);
  digitalWrite(MOTOR_1_DIR, LOW);

  digitalWrite(MOTOR_2_PWM, LOW);
  digitalWrite(MOTOR_2_DIR, LOW);
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
    switch (incomingPacket[0]) {
      case 'F':
        forward(250);
        delay(50);
        strip.setPixelColor(0, 0, 0, 0);
        strip.show();
        break;
      case 'S':
        cruise();
        strip.setPixelColor(0, 255, 0, 0);
        strip.show();
        break;
      case 'L':
        left(225);
        strip.setPixelColor(0, 220, 40, 10);
        strip.show();
        break;
      case 'R':
        right(255);
        strip.setPixelColor(0, 220, 40, 10);
        strip.show();
        break;
      case 'B':
        backward(250);
        delay(50);
        strip.setPixelColor(0, 255, 255, 255);
        strip.show();
        break;
      // Add more cases as needed
      default:
        // Serial.println("Unknown command");
        break;
    }
  }
}
