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
#include <Servo.h>

// prepare servo objects
Servo servo1;
const uint8_t ServoPin1 = D6;
int servo_init_angle = 90;

// USS
const uint8_t trigPin = D7;
const uint8_t echoPin = D8;
double stop_distance = 0.0;
int stop_distance_threshold = 60;


// program variables
int cmdCode = 0;
String prefix = "S";
int currentModeState = 0;

// steering
int steerAngleMid = 15;
int steerAngleMax = 30;

// WiFi credentials
const char* ssid = "SCOUT";
const char* password = "scout@2025";

// UDP setup
WiFiUDP Udp;
unsigned int localUdpPort = 8888;  // local port to listen on
uint8_t data[5];


// RGB LED
#define RGB D0      // default pin RGB LED WS2812 on Mercury
#define NUM_LEDS 1  // Number of LEDs on the Mercury
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, RGB, NEO_GRB + NEO_KHZ800);

// user functions
void init_hardware() {
  pinMode(MOTOR_1_PWM, OUTPUT);
  pinMode(MOTOR_2_PWM, OUTPUT);
  pinMode(MOTOR_1_DIR, OUTPUT);
  pinMode(MOTOR_2_DIR, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ServoPin1, OUTPUT);
  servo1.attach(ServoPin1, 600, 2400);
  servo1.write(servo_init_angle);
}

void cruise() {
  digitalWrite(MOTOR_1_PWM, LOW);
  digitalWrite(MOTOR_1_DIR, LOW);

  digitalWrite(MOTOR_2_PWM, LOW);
  digitalWrite(MOTOR_2_DIR, LOW);
  servo1.write(servo_init_angle);
}

void forward(int pwmValue) {
  // 0 < pwmValue < 255;
  analogWrite(MOTOR_1_PWM, pwmValue);
  digitalWrite(MOTOR_1_DIR, LOW);

  analogWrite(MOTOR_2_PWM, pwmValue);
  digitalWrite(MOTOR_2_DIR, LOW);
  servo1.write(servo_init_angle);
}

void backward(int pwmValue) {
  // 0 < pwmValue < 255;
  digitalWrite(MOTOR_1_PWM, LOW);
  analogWrite(MOTOR_1_DIR, pwmValue);

  digitalWrite(MOTOR_2_PWM, LOW);
  analogWrite(MOTOR_2_DIR, pwmValue);
  servo1.write(servo_init_angle);
}

void brake() {
  digitalWrite(MOTOR_1_PWM, HIGH);
  digitalWrite(MOTOR_1_DIR, HIGH);

  digitalWrite(MOTOR_2_PWM, HIGH);
  digitalWrite(MOTOR_2_DIR, HIGH);
}

void left(int pwmValue, int steerAngle = steerAngleMax) {
  //PWM: 0 -> Slow; 255 -> Fast
  analogWrite(MOTOR_1_PWM, int(pwmValue * 0.9));
  digitalWrite(MOTOR_1_DIR, LOW);

  analogWrite(MOTOR_2_PWM, pwmValue);
  digitalWrite(MOTOR_2_DIR, LOW);
  servo1.write(servo_init_angle + steerAngle);
}

void right(int pwmValue, int steerAngle = steerAngleMax) {
  //PWM: 0 -> Slow; 255 -> Fast
  analogWrite(MOTOR_1_PWM, pwmValue);
  digitalWrite(MOTOR_1_DIR, LOW);

  analogWrite(MOTOR_2_PWM, int(pwmValue * 0.9));
  digitalWrite(MOTOR_2_DIR, LOW);
  servo1.write(servo_init_angle - steerAngle);
}

void left_back(int pwmValue, int steerAngle = steerAngleMax) {
  //PWM: 0 -> Slow; 255 -> Fast
  digitalWrite(MOTOR_1_PWM, LOW);
  analogWrite(MOTOR_1_DIR, int(pwmValue * 0.9));

  digitalWrite(MOTOR_2_PWM, LOW);
  analogWrite(MOTOR_2_DIR, pwmValue);
  servo1.write(servo_init_angle + steerAngle);
}

void right_back(int pwmValue, int steerAngle = steerAngleMax) {
  //PWM: 0 -> Slow; 255 -> Fast
  digitalWrite(MOTOR_1_PWM, LOW);
  analogWrite(MOTOR_1_DIR, pwmValue);

  digitalWrite(MOTOR_2_PWM, LOW);
  analogWrite(MOTOR_2_DIR, int(pwmValue * 0.9));
  servo1.write(servo_init_angle - steerAngle);
}

int measureDistance() {
  double uss_distance = 0.0;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long echo_time = pulseIn(echoPin, HIGH, 20000);
  if (echo_time == 0) {
    return (stop_distance_threshold * 2);
  } else {
    uss_distance = echo_time * 0.034 / 2;
    return int(uss_distance);
  }
}

void drive() {
  receiveUdpPacket();

  int pwmValue = 255;
  if (currentModeState) {
    stop_distance = measureDistance();
    if (stop_distance < stop_distance_threshold) {
      pwmValue = 0;
    }
  }

  switch (cmdCode) {
    case 0:
      // stop
      cruise();
      strip.setPixelColor(0, 255, 0, 0);
      strip.show();
      break;
    case 1:
      forward(pwmValue);
      strip.setPixelColor(0, 0, 0, 0);
      strip.show();
      break;
    case 2:
      backward(225);
      strip.setPixelColor(0, 255, 255, 255);
      strip.show();
      break;
    case 3:
      left(pwmValue * 0.9);
      strip.setPixelColor(0, 220, 40, 10);
      strip.show();
      break;
    case 4:
      right(pwmValue * 0.9);
      strip.setPixelColor(0, 220, 40, 10);
      strip.show();
      break;
    case 5:
      //left-fwd
      left(pwmValue * 0.9, steerAngleMid);
      strip.setPixelColor(0, 220, 40, 10);
      strip.show();
      break;
    case 6:
      //right-fwd
      right(pwmValue * 0.9, steerAngleMid);
      strip.setPixelColor(0, 220, 40, 10);
      strip.show();
      break;
    case 7:
      //left-back
      left_back(200);
      strip.setPixelColor(0, 255, 255, 255);
      strip.show();
      break;
    case 8:
      //right-back
      right_back(200);
      strip.setPixelColor(0, 255, 255, 255);
      strip.show();
      break;

    // Add more cases as needed
    default:
      break;
  }
}

// function - recieve data from UDP sender
void receiveUdpPacket() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Udp.read(data, 5);
    cmdCode = data[0];
    currentModeState = data[2];
    int servoPin = data[3];
    int servo_angle = data[4];

    if (servoPin == 6) {
      servo_init_angle = servo_angle;
    }
  }
}

void setup() {
  // Serial.begin(115200);
  init_hardware();
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid, password);
  delay(1000);
  Udp.begin(localUdpPort);
  // Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  strip.begin();
  strip.setBrightness(25);
  strip.show();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  }
  drive();
  yield();
}
