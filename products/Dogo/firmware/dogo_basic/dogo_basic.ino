#include <Arduino.h>
#include <Servo.h>

// prepare servo objects
Servo frontServo;
Servo backServo;

const uint8_t frontServoPin = 13;
const uint8_t backServoPin = 15;
uint8_t front_servo_init_angle = 90;
uint8_t back_servo_init_angle = 90;
uint8_t front_change_angle = 12;
uint8_t back_change_angle = 18;

// define ultrasonic pins
const uint8_t trigPin = 14;
const uint8_t echoPin = 02;
double stop_distance = 0.0;
uint8_t uss_measure_samples = 3;

// program variables
uint16_t prog_delay = 1000;
uint16_t delay_motion = 400;

// user functions
void init_hardware() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  frontServo.attach(frontServoPin);
  frontServo.write(front_servo_init_angle);
  backServo.attach(backServoPin);
  backServo.write(back_servo_init_angle);
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

void dogo_cat_walk() {
  frontServo.write(front_servo_init_angle + front_change_angle);
  delay(50);
  backServo.write(back_servo_init_angle - back_change_angle);
  delay(delay_motion - 50);

  frontServo.write(front_servo_init_angle - front_change_angle);
  delay(50);
  backServo.write(back_servo_init_angle + back_change_angle);
  delay(delay_motion - 50);
  Serial.println("dogo_walk");
}

void dogo_pause() {
  frontServo.write(front_servo_init_angle);
  backServo.write(back_servo_init_angle);
  Serial.println("dogo_stop");
}

void setup() {
  Serial.begin(9600);
  init_hardware();
  delay(prog_delay * 2);
}

void loop() {
  stop_distance = measureDistance();
  if (stop_distance > 5) {
    dogo_cat_walk();
  } else {
    dogo_pause();
  }
}