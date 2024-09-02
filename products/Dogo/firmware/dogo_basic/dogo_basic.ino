// dogo_basic.ino
// This code controls the motion of a dog robot, allowing it to walk forward.
// The robot is equipped with ultrasonic sensors that continuously scan for obstacles in its path.
// As it moves forward, the sensors monitor the environment.
// If an obstacle is detected within a certain range, the robot immediately halts its forward motion to avoid a collision.

// Electronics needed
// Any Ralio microcontroller board: Below code is specific to Mercury v2
// 2x servo motors
// 1x ultrasonic sensor

// board: Mercury v2

#include <Arduino.h>
#include <Servo.h>

// prepare servo objects
Servo frontServo;
Servo backServo;

const uint8_t frontServoPin = D5;
const uint8_t backServoPin = D6;
uint8_t frontServo_initial_angle = 90;
uint8_t backServo_initial_angle = 90;
uint8_t frontServo_change_angle = 12;
uint8_t backServo_change_angle = 18;

// define ultrasonic pins
const uint8_t uss_trigPin = D8;
const uint8_t uss_echoPin = D7;
double dogo_stop_distance = 0.0;
uint8_t uss_measure_samples = 3;

// program variables
uint16_t delay_prog = 1000;
uint16_t delay_motion = 400;

// user functions
void init_hardware() {
  pinMode(uss_trigPin, OUTPUT);
  pinMode(uss_echoPin, INPUT);
  frontServo.attach(frontServoPin);
  frontServo.write(frontServo_initial_angle);
  backServo.attach(backServoPin);
  backServo.write(backServo_initial_angle);
}

// function - distance measurement using ultrasonic sensor
int measureDistance() {
  double uss_distance = 0.0;
  for (int i = 0; i < uss_measure_samples; i++) {
    digitalWrite(uss_trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(uss_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(uss_trigPin, LOW);
    long echo_time = pulseIn(uss_echoPin, HIGH);
    uss_distance += echo_time * 0.034 / 2;
    delay(10);
  }

  return int(uss_distance / uss_measure_samples);
}

void dogo_cat_walk() {
  frontServo.write(frontServo_initial_angle + frontServo_change_angle);
  delay(50);
  backServo.write(backServo_initial_angle - backServo_change_angle);
  delay(delay_motion - 50);

  frontServo.write(frontServo_initial_angle - frontServo_change_angle);
  delay(50);
  backServo.write(backServo_initial_angle + backServo_change_angle);
  delay(delay_motion - 50);
  Serial.println("dogo_walk");
}

void dogo_pause() {
  frontServo.write(frontServo_initial_angle);
  backServo.write(backServo_initial_angle);
  Serial.println("dogo_stop");
}

void setup() {
  Serial.begin(9600);
  init_hardware();
  delay(delay_prog * 2);
}

void loop() {
  dogo_stop_distance = measureDistance();
  if (dogo_stop_distance > 5) {
    dogo_cat_walk();
  } else {
    dogo_pause();
  }
}