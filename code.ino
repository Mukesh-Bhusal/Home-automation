#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10
#define SOLENOID_RELAY 7   // Relay for solenoid lock
#define MOTOR_RELAY 4      // Relay for motor
#define trigPin 3
#define echoPin 5

MFRC522 rfid(SS_PIN, RST_PIN);
byte allowedUID[] = {0xF3, 0x19, 0x1E, 0x14};  // Authorized RFID card UID

long duration;
float distance;
bool motorState = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(SOLENOID_RELAY, OUTPUT);
  digitalWrite(SOLENOID_RELAY, HIGH); // Lock the door (Active HIGH relay)

  pinMode(MOTOR_RELAY, OUTPUT);
  digitalWrite(MOTOR_RELAY, HIGH);    // Motor OFF (Active LOW relay)

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {

  // --- RFID Section ---
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    bool accessGranted = true;
    for (byte i = 0; i < 4; i++) {
      if (rfid.uid.uidByte[i] != allowedUID[i]) {
        accessGranted = false;
        break;
      }
    }

    if (accessGranted) {
      Serial.println("Access Granted");
      digitalWrite(SOLENOID_RELAY, LOW);  // Unlock
      delay(5000);                        // Keep unlocked for 5s
      digitalWrite(SOLENOID_RELAY, HIGH); // Lock again
    } else {
      Serial.println("Access Denied");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // --- Ultrasonic + Motor Control Section ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.0343 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (!motorState && distance > 25) {
    motorState = true;
  } else if (motorState && distance < 10) {
    motorState = false;
  }

  digitalWrite(MOTOR_RELAY, motorState ? LOW : HIGH);  // LOW = Motor ON

  Serial.print("Motor State: ");
  Serial.println(motorState ? "ON" : "OFF");

  delay(1000);
}
