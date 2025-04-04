// *************************
//
// RALIO TECHNOLOGIES LLP
//
// *************************

// scout_basic.ino
// This code controls a car robot, enabling it to drive in multiple directions.
// Each directional movement lasts for 3 seconds.
// The robot is equipped with ultrasonic sensors that continuously scan for obstacles in its path.
// As it moves forward, the sensors monitor the environment.
// If an obstacle is detected within a certain range, the robot immediately halts its forward motion to avoid a collision.

// Electronics needed
// Any Ralio microcontroller board: Below code is specific to Mercury v2
// 2x dc motors
// 1x ultrasonic sensor

// board: Mercury v2

#include <Arduino.h>

// define ultrasonic pins
const int trigPin = D7;
const int echoPin = D8;
double dogo_stop_distance = 0.0;
uint8_t uss_measure_samples = 3;
double uss_dist_measure_alpha = 0.9;

// program variables
uint16_t delay_prog = 1000;
int analogResolution = 8;

// user functions
void init_hardware() {
  pinMode(MOTOR_1_PWM, OUTPUT);
  pinMode(MOTOR_2_PWM, OUTPUT);
  pinMode(MOTOR_1_DIR, OUTPUT);
  pinMode(MOTOR_2_DIR, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

int pwmValueReverse(int pwmValue) {
  int pwmValueReverse = 2 ^ analogResolution - pwmValue;
  return (pwmValueReverse);
}

void cruise() {
  digitalWrite(MOTOR_1_PWM, LOW);
  digitalWrite(MOTOR_1_DIR, LOW);

  digitalWrite(MOTOR_2_PWM, LOW);
  digitalWrite(MOTOR_2_DIR, LOW);
}

void forward(int pwmValue) {
  //PWM: 0 -> Slow; 255 -> Fast
  analogWrite(MOTOR_1_PWM, pwmValue);
  digitalWrite(MOTOR_1_DIR, LOW);

  analogWrite(MOTOR_2_PWM, pwmValue);
  digitalWrite(MOTOR_2_DIR, LOW);
}

void backward(int pwmValue) {
  //PWM: 0 -> Slow; 255 -> Fast
  analogWrite(MOTOR_1_PWM, pwmValueReverse(pwmValue));
  digitalWrite(MOTOR_1_DIR, HIGH);

  analogWrite(MOTOR_2_PWM, pwmValueReverse(pwmValue));
  digitalWrite(MOTOR_2_DIR, HIGH);
}

void brake() {
  digitalWrite(MOTOR_1_PWM, HIGH);
  digitalWrite(MOTOR_2_PWM, HIGH);
  digitalWrite(MOTOR_1_DIR, HIGH);
  digitalWrite(MOTOR_2_DIR, HIGH);
}

void spin_right(int pwmValue) {
  //PWM: 0 -> Slow; 255 -> Fast
  analogWrite(MOTOR_1_PWM, pwmValueReverse(pwmValue));
  digitalWrite(MOTOR_1_DIR, HIGH);

  analogWrite(MOTOR_2_PWM, pwmValue);
  digitalWrite(MOTOR_2_DIR, LOW);
}

void spin_left(int pwmValue) {
  //PWM: 0 -> Slow; 255 -> Fast
  analogWrite(MOTOR_1_PWM, pwmValue);
  digitalWrite(MOTOR_1_DIR, LOW);

  analogWrite(MOTOR_2_PWM, pwmValueReverse(pwmValue));
  digitalWrite(MOTOR_2_DIR, HIGH);
}

// function - distance measurement using ultrasonic sensor
int measureDistance() {
  double uss_distance = 0.0;
  for (int i = 0; i < uss_measure_samples; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long echo_time = pulseIn(echoPin, HIGH);
    uss_distance += echo_time * 0.034 / 2;
    delay(10);
  }

  return int(uss_distance / uss_measure_samples);
}

void setup() {
  Serial.begin(115200);
  init_hardware();
  delay(delay_prog * 2);
}

void loop() {
  analogWriteResolution(analogResolution);
  dogo_stop_distance = measureDistance();
  Serial.println(dogo_stop_distance);
  if (dogo_stop_distance > 10) {
    forward(200);
    delay(delay_prog * 1);
  } else {
    backward(200);
    delay(delay_prog * 1);
    brake();
    delay(delay_prog * 1);
    spin_left(200);
    delay(delay_prog * 1);
    brake();
    delay(delay_prog * 1);
  }
}
