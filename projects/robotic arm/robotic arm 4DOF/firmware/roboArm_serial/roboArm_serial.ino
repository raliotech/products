// *************************
//
// RALIO TECHNOLOGIES LLP
//
// *************************

// roboArm_serial.ino
// This code controls a robotic arm, enabling it to drive in multiple directions.
// Each directional movement is controlled by 3 servos.
// The arm is equipped with a gripper mechanism to grap on to objects.

// Electronics needed
// Any Ralio microcontroller board: Below code is specific to Mercury v2
// 4x dc motors

// board: Mercury v2

#include <Arduino.h>
#include <Servo.h>

String cmd = "";

// prepare servo objects
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

const uint8_t ServoPin1 = D4;  // wrist angle Y1
const uint8_t ServoPin2 = D6;  // hand angle Y2
const uint8_t ServoPin3 = D3;  // gripper angle X1 *** maximum rotational freedom on 45 degree ***
const uint8_t ServoPin4 = D5;  // base rotation X2

uint8_t servo1_init_angle = 90;
uint8_t servo2_init_angle = 90;
uint8_t servo3_init_angle = 90;
uint8_t servo4_init_angle = 90;

uint8_t servo1_angle = servo1_init_angle;
uint8_t servo2_angle = servo2_init_angle;
uint8_t servo3_angle = servo3_init_angle;
uint8_t servo4_angle = servo4_init_angle;

// Really important to tune these parameters to ensure
// the servo arm rotation is within permissible limits

//wrist
uint8_t servo1_min = 30;
uint8_t servo1_max = 150;

//hand
uint8_t servo2_min = 60;
uint8_t servo2_max = 110;

//gripper *** maximum rotational freedom on 45 degree ***
uint8_t servo3_min = 45;
uint8_t servo3_max = 90;

// base
uint8_t servo4_min = 30;
uint8_t servo4_max = 150;

void init_hardware() {
  servo1.attach(ServoPin1, 600, 2400);
  servo1.write(servo1_init_angle);
  servo2.attach(ServoPin2, 600, 2400);
  servo2.write(servo2_init_angle);
  servo3.attach(ServoPin3, 600, 2400);
  servo3.write(servo3_init_angle);
  servo4.attach(ServoPin4, 600, 2400);
  servo4.write(servo4_init_angle);
}

uint8_t servoAngleCheck(uint8_t angle, uint8_t minA, uint8_t maxA) {
  if (angle <= minA) {
    return (minA);
  } else if (angle >= maxA) {
    return (maxA);
  } else {
    return (angle);
  }
}

void setup() {
  Serial.begin(115200);
  init_hardware();
  Serial.println("Ready to roll!");
  randomSeed(100);
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();

    if (data.length() > 0) {
      String axis = data.substring(0, 2);
      uint8_t angle = data.substring(2).toInt();

      if (axis == "X1") {
        servo3_angle = angle;
      } else if (axis == "X2") {
        servo4_angle = angle;
      } else if (axis == "Y1") {
        servo1_angle = angle;
      } else if (axis == "Y2") {
        servo2_angle = angle;
      }
    }
  }

  // servo1_angle = servo1_angle + random(-5, 6);
  // servo2_angle = servo2_angle + random(-5, 6);
  // servo3_angle = servo3_angle + random(-5, 6);
  // servo4_angle = servo4_angle + random(-5, 6);

  servo1_angle = servoAngleCheck(servo1_angle, servo1_min, servo1_max);
  servo2_angle = servoAngleCheck(servo2_angle, servo2_min, servo2_max);
  servo3_angle = servoAngleCheck(servo3_angle, servo3_min, servo3_max);
  servo4_angle = servoAngleCheck(servo4_angle, servo4_min, servo4_max);

  servo1.write(servo1_angle);
  servo2.write(servo2_angle);
  servo3.write(servo3_angle);
  servo4.write(servo4_angle);

  Serial.print(servo1_angle);
  Serial.print('\t');
  Serial.print(servo2_angle);
  Serial.print('\t');
  Serial.print(servo3_angle);
  Serial.print('\t');
  Serial.println(servo4_angle);

  delay(100);
}
