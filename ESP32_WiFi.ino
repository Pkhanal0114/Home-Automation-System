
#define BLYNK_TEMPLATE_ID "TMPL6aD-T7woD"
#define BLYNK_TEMPLATE_NAME "Home "
#define BLYNK_AUTH_TOKEN "aUqLR82j29mbopS1Jwo-PopYzgaW4yxJ"

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "NTFiber-1A48";
char pass[] = "Mahakal@6580";

bool fetch_blynk_state = true;  // true or false

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <ESP32Servo.h>

#define DHTPIN 16 // D16 pin connected with DHT
#define DHTTYPE DHT11 // DHT 11

// Define the GPIO connected with Relays and switches
#define RelayPin1 27 
#define RelayPin2 21  
#define RelayPin3 19  

#define SwitchPin1 14 // D13

// Change the virtual pins according the rooms
#define VPIN_BUTTON_1 V1
#define VPIN_BUTTON_2 V2
#define VPIN_BUTTON_3 V3 
#define VPIN_SERVO V8

#define VPIN_TEMPERATURE V6
#define VPIN_HUMIDITY V7

// Servo setup
Servo myServo;   // Create servo object to control a servo
int servoPin = 18; // Define the pin to which the servo is connected

// Relay State
bool toggleState_1 = LOW; // Define integer to remember the toggle state for relay 1
bool toggleState_2 = LOW; // Define integer to remember the toggle state for relay 2
bool toggleState_3 = LOW; // Define integer to remember the toggle state for relay 3

// Switch State
bool SwitchState_1 = LOW;

int wifiFlag = 0;
float temperature1 = 0;
float humidity1 = 0;

char auth[] = BLYNK_AUTH_TOKEN;

BlynkTimer timer;
DHT dht(DHTPIN, DHTTYPE);

// When App button is pushed - switch the state

BLYNK_WRITE(VPIN_BUTTON_1) {
  toggleState_1 = param.asInt();
  digitalWrite(RelayPin1, !toggleState_1);
}

BLYNK_WRITE(VPIN_BUTTON_2) {
  toggleState_3 = param.asInt();
  digitalWrite(RelayPin3, !toggleState_3);
}

BLYNK_WRITE(VPIN_BUTTON_3) {
  // RelayPin3 is controlled by temperature condition
  if (param.asInt()) {
    if (temperature1 > 25) {
      toggleState_3 = true;
      digitalWrite(RelayPin3, LOW); // Turn on RelayPin3
    } else {
      toggleState_3 = false;
      digitalWrite(RelayPin3, HIGH); // Turn off RelayPin3
      Blynk.virtualWrite(VPIN_BUTTON_3, LOW); // Update app state to off
    }
  } else {
    toggleState_3 = false;
    digitalWrite(RelayPin3, HIGH); // Turn off RelayPin3
  }
}

BLYNK_WRITE(VPIN_BUTTON_C) {
  all_SwitchOff();
}

BLYNK_WRITE(VPIN_SERVO) {
  int servoState = param.asInt();
  if (servoState) {
    myServo.write(90); // Open the gate (move the servo to 90 degrees)
    Serial.println("Gate opened");
  } else {
    myServo.write(0); // Close the gate (move the servo to 0 degrees)
    Serial.println("Gate closed");
  }
}

void all_SwitchOff() {
  toggleState_1 = 0;
  digitalWrite(RelayPin1, HIGH);
  Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  delay(100);

  toggleState_2 = 0;
  digitalWrite(RelayPin1, HIGH);
  Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
  delay(100);

  toggleState_3 = 0;
  digitalWrite(RelayPin3, HIGH);
  Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3);
  delay(100);

  Blynk.virtualWrite(VPIN_HUMIDITY, humidity1);
  Blynk.virtualWrite(VPIN_TEMPERATURE, temperature1);
}

void checkBlynkStatus() { // called every 2 seconds by SimpleTimer
  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    wifiFlag = 1;
    Serial.println("Blynk Not Connected");
  }
  if (isconnected == true) {
    wifiFlag = 0;
    if (!fetch_blynk_state) {
      Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    }
    // Serial.println("Blynk Connected");
  }
}

BLYNK_CONNECTED() {
  // Request the latest state from the server
  if (fetch_blynk_state) {
    Blynk.syncVirtual(VPIN_BUTTON_1);
    Blynk.syncVirtual(VPIN_BUTTON_2);
    Blynk.syncVirtual(VPIN_SERVO);
    Blynk.syncVirtual(VPIN_BUTTON_3); // Sync VPIN_BUTTON_3 for Relay 3 with temperature condition
  }
  Blynk.syncVirtual(VPIN_TEMPERATURE);
  Blynk.syncVirtual(VPIN_HUMIDITY);
}

void readSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  } else {
    humidity1 = h;
    temperature1 = t;
    // Serial.println(temperature1);
    // Serial.println(humidity1);
  }
}

void sendSensor() {
  readSensor();
  // You can send any value at any time.
  // Please don't send more than 10 values per second.
  Blynk.virtualWrite(VPIN_HUMIDITY, humidity1);
  Blynk.virtualWrite(VPIN_TEMPERATURE, temperature1);
}

void setup() {
  Serial.begin(9600);

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);
  pinMode(SwitchPin1, INPUT_PULLUP);

  // During Starting all Relays should TURN OFF
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);
  digitalWrite(RelayPin3, HIGH);

  dht.begin(); // Enabling DHT sensor

  // Allow allocation of all timers for servo control
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Attach the servo
  myServo.attach(servoPin);

  // Blynk.begin(auth, ssid, pass);
  WiFi.begin(ssid, pass);
  timer.setInterval(2000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds
  timer.setInterval(1000L, sendSensor); // Sending Sensor Data to Blynk Cloud every 1 second
  Blynk.config(auth);
  delay(1000);

  if (!fetch_blynk_state) {
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  }
}

void loop() {
  Blynk.run();
  timer.run();

  // Read the state of the physical switch
  bool switchState = digitalRead(SwitchPin1);

  // Check if the switch is pressed (assuming LOW means pressed)
  if (switchState == LOW) {
    toggleState_2 = HIGH;
    digitalWrite(RelayPin2, LOW); // Turn on RelayPin2
  } else {
    toggleState_2 = LOW;
    digitalWrite(RelayPin2, HIGH); // Turn off RelayPin2
  }

}
