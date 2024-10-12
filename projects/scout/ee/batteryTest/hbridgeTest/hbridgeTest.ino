#include <Arduino.h>

int analogResolution = 8;
String cmd = "";

// user functions
void init_hardware() {
  analogWriteResolution(analogResolution);
  pinMode(MOTOR_1_PWM, OUTPUT);
  pinMode(MOTOR_2_PWM, OUTPUT);
  pinMode(MOTOR_1_DIR, OUTPUT);
  pinMode(MOTOR_2_DIR, OUTPUT);
  pinMode(D8, INPUT_PULLUP);
}

void setup_harware() {
  if (Serial.available()) {
    cmd = Serial.readStringUntil('\n');
  }
  else if (digitalRead(D8) == HIGH) {
    cmd = 'd';
  }
  else if (digitalRead(D8) == LOW) {
    cmd = 'h';
  }
  switch (cmd[0]) {
    case 'f':
      forward(255);
      break;
    case 'b':
      backward(255);
      break;
    case 'd':
      dance();
      break;
    case 's':
      cruise();
      break;
    case 'h':
      brake();
      break;
    case 'l':
      spin_left(255);
      break;
    case 'r':
      spin_right(255);
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("hi");
  init_hardware();
  pinMode(0, OUTPUT);
}

void loop() {
  setup_harware();
  delay(1000);
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

void dance() {
  forward(255);
  delay(2000);
  backward(255);
  delay(1000);

}
