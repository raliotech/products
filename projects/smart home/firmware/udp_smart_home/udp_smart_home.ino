// *************************
//
// RALIO TECHNOLOGIES LLP
//
// *************************

// udp_smart_home.ino
// This code controls a smart home, enabling wireless connection to drive IoT nodes.
// The smart home is equipped with temeprature sensor, servo motor, brushed dc motor, motion sensor, two LED lamps.

// Electronics needed
// Any Ralio microcontroller board: Below code is specific to Mercury v2
// 1x dc motors
// 1x servo motor
// 1x DHT11 sensor
// 1x PIR motion sensor
// 2x LED Lamp boards

// board: Mercury v2


#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include <Servo.h>

// DHT11 Sensor setup
#define DHTPIN D5  // Pin where DHT11 is connected
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Servo setup
Servo myServo;
int servoPin = D6;  // Pin where servo is connected

// WiFi credentials
const char* ssid = "Smart Home";
const char* password = "ralio@123";

// Flutter app's IP address and port
const char* destinationIP = "192.168.4.2";  // Replace with your phone's IP address
const int destinationPort = 8080;           // Match the port your Flutter app listens on

// UDP instance
WiFiUDP UDP;

// Local UDP port to listen on
const int localUdpPort = 8888;  // Must match the listenPort in the Flutter app
char incomingPacket[255];

// Time interval for sending temperature and humidity
unsigned long lastSendTime = 0;
const long sendInterval = 10000;  // 10 seconds


// Pin configuration for the curtain and PIR sensor
const int pir_sensor = D3;
const int fan_pos = D2;       // Motor PWM control pin
const int fan_neg = D4;       // Motor direction pin
const int hall_light = D7;    // LED PWM positive pin
const int bedroom_light = D8; // LED PWM negative pin
const int buzz = D1;          // Buzzer

int windowPos = 10;
bool hall_light_state = false;
bool bedroom_light_state = false;

void setup() {
  // Start Serial Monitor
  Serial.begin(115200);

  setupServo();
  dht.begin();

  pinMode(pir_sensor, INPUT);
  pinMode(fan_pos, OUTPUT);
  pinMode(fan_neg, OUTPUT);
  pinMode(hall_light, OUTPUT);
  pinMode(bedroom_light, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid, password);

  UDP.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

void loop() {
  handleUDP();

  if (millis() - lastSendTime > sendInterval) {
    lastSendTime = millis();
    sendTemperatureAndHumidity();
  }
}

// Function to set up Wi-Fi
void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid, password);
  Serial.println("Setup Complete");
  delay(1000);
  Serial.println("\nWiFi connected.");
  Serial.print("Device IP: ");
  Serial.println(WiFi.localIP());
}

// Function to set up the Servo
void setupServo() {
  myServo.attach(servoPin, 600, 2400);
  myServo.write(10);  // Initialize servo to 0 degrees
}

// Function to handle incoming UDP commands
void handleUDP() {
  int packetSize = UDP.parsePacket();
  if (packetSize) {
    // Read the incoming packet
    int len = UDP.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;  // Null-terminate the packet
    }

    // Process the command
    String command = String(incomingPacket);
    Serial.println(command);
    if (command == "CU") {
      moveServo(180);  // Move servo to 0 degrees
    } else if (command == "CD") {
      moveServo(10);
    } else if (command == "L1ON") {
      hall_light_state = true;
    } else if (command == "L1OFF") {
      hall_light_state = false;
    } else if (command == "L2ON") {
      bedroom_light_state = true;
    } else if (command == "L2OFF") {
      bedroom_light_state = false;
    } else if (command == "FON") {
      fanOn(255);
    } else if (command == "FOFF") {
      fanOff();
    }
  }
  controlLights(hall_light_state, bedroom_light_state);
}

// Function to send temperature and humidity data via UDP
void sendTemperatureAndHumidity() {
  // Read temperature and humidity from DHT11 sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if any reads failed and try again
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    /*
    float temperature = random(280, 300) / 10.0;  // Generates values between 28.0 and 32.0
    float humidity = random(500, 650) / 10.0;  // Generates values between 50.0 and 65.0
    String data = "TEMP:" + String(temperature) + ",HUM:" + String(humidity);
    UDP.beginPacket(destinationIP, destinationPort);
    UDP.write(data.c_str());
    UDP.endPacket();
    Serial.println("Dummy data: " + data);
    */
  } else {
    // Create message with temperature and humidity
    String data = "TEMP:" + String(temperature) + ",HUM:" + String(humidity);
    UDP.beginPacket(destinationIP, destinationPort);
    UDP.write(data.c_str());
    UDP.endPacket();
    Serial.println("Sent data: " + data);
  }
}

void moveServo(int angle) {
  Serial.print(angle);
  Serial.println(windowPos);
  if (angle < 50) {
    for (int i = windowPos; i > angle; i -= 5) {
      myServo.write(i);
      delay(10);
    }
    windowPos = angle;
  } else {
    for (int i = windowPos; i < angle; i += 5) {
      myServo.write(i);
      delay(10);
    }
    windowPos = angle;
  }
}

// Function to move the curtain forward (open)
void fanOn(int pwmValue) {
  analogWrite(fan_pos, pwmValue);
  digitalWrite(fan_neg, LOW);
}

// Function to move the curtain backward (close)
void backward(int pwmValue) {
  digitalWrite(fan_pos, LOW);
  analogWrite(fan_neg, pwmValue);
}

// Function to stop the curtain motor
void fanOff() {
  digitalWrite(fan_pos, LOW);
  digitalWrite(fan_neg, LOW);
}

void controlLights(bool light1, bool light2) {
  if (light1 && light2) {
    digitalWrite(hall_light, HIGH);
    digitalWrite(bedroom_light, HIGH);
  } else if (light1 && !light2) {
    digitalWrite(hall_light, HIGH);
    digitalWrite(bedroom_light, LOW);
  } else if (!light1 && light2) {
    digitalWrite(hall_light, LOW);
    digitalWrite(bedroom_light, HIGH);
  } else {
    digitalWrite(hall_light, LOW);
    digitalWrite(bedroom_light, LOW);
  }
}
