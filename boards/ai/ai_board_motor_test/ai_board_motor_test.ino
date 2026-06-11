// Pin Definitions
const int M_A1 = 45; // PWM Capable
const int M_A2 = 46; // PWM Capable
const int M_B1 = 39; // PWM Capable
const int M_B2 = 40; // PWM Capable
const int MOT_EN = 38; // Digital Enable

int motorSpeed = 200; // 0 - 255

void setup() {
  pinMode(M_A1, OUTPUT);
  pinMode(M_A2, OUTPUT);
  pinMode(M_B1, OUTPUT);
  pinMode(M_B2, OUTPUT);
  pinMode(MOT_EN, OUTPUT);

  // Enable the driver
  digitalWrite(MOT_EN, HIGH);
}

void stop() {
  analogWrite(M_A1, 0);
  analogWrite(M_A2, 0);
  analogWrite(M_B1, 0);
  analogWrite(M_B2, 0);
}

void forward(int speed) {
  analogWrite(M_A1, speed);
  analogWrite(M_A2, 0);
  analogWrite(M_B1, speed);
  analogWrite(M_B2, 0);
}

void backward(int speed) {
  analogWrite(M_A1, 0);
  analogWrite(M_A2, speed);
  analogWrite(M_B1, 0);
  analogWrite(M_B2, speed);
}

void left(int speed) {
  analogWrite(M_A1, 0);
  analogWrite(M_A2, speed);
  analogWrite(M_B1, speed);
  analogWrite(M_B2, 0);
}

void right(int speed) {
  analogWrite(M_A1, speed);
  analogWrite(M_A2, 0);
  analogWrite(M_B1, 0);
  analogWrite(M_B2, speed);
}

void loop() {
  forward(motorSpeed);
  delay(2000);
  stop();
  delay(500);

  backward(motorSpeed);
  delay(2000);
  stop();
  delay(500);

  left(motorSpeed);
  delay(1000);
  stop();
  delay(500);

  right(motorSpeed);
  delay(1000);
  stop();
  delay(2000);
}