void setup() {
  // put your setup code here, to run once:
  analogReadResolution(10);
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  Serial.begin(115200);
}

void loop() {
  // A1 -> Vin shunt
  double vin_shunt = analogRead(A1);
  Serial.print(vin_shunt / 1024 * 3.3 * (4.86 + 9.8)/4.86);
  Serial.print("; ");
  // A2 -> Vout shunt
  double vout_shunt = analogRead(A2);
  Serial.print(vout_shunt / 1024 * 3.3 * (4.91 + 9.92)/4.91);
  Serial.print("; ");
  // A3 -> V5 shunt
  double v5 = analogRead(A3);
  Serial.print(v5 / 1024 * 3.3 * 2);
  Serial.print("; ");
  // A3 -> 3v3 shunt
  double v3 = analogRead(A4);
  Serial.print(v3 / 1024 * 3.3);
  Serial.println("; ");
  delay(10);

}
