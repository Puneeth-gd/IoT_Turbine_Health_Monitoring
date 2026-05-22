#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL3wCNBRjUn"
#define BLYNK_TEMPLATE_NAME "grafical"
#define BLYNK_AUTH_TOKEN "_SBbCsuPGsPmpJu6REWek1z4zKnvEBkl"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

/* WiFi credentials */
char ssid[] = "Tech";
char pass[] = "pun123456";

/* Pin definitions */
#define MIC_PIN A0
#define BUZZER_PIN D5

/* Thresholds */
const int SOUND_THRESHOLD = 600;
const float VIB_THRESHOLD = 2.5;

/* Alert control */
const unsigned long ALERT_COOLDOWN = 5000; // 5 seconds

Adafruit_MPU6050 mpu;
BlynkTimer timer;

unsigned long lastSoundAlert = 0;
unsigned long lastVibAlert = 0;

/* -------------------- SETUP -------------------- */
void setup() {
  Serial.begin(9600);
  Serial.println("System starting...");

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // I2C pins for ESP8266
  Wire.begin(D2, D1);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  if (mpu.begin()) {
    Serial.println("MPU6050 Ready");
  } else {
    Serial.println("MPU6050 NOT detected");
  }

  Serial.println("MAX4466 Ready");

  timer.setInterval(1000L, readSensors);
}

/* -------------------- BUZZER -------------------- */
void beepOnce() {
  Serial.println("[BUZZER] Beep triggered");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(300);
  digitalWrite(BUZZER_PIN, LOW);
}

/* -------------------- SENSOR READ -------------------- */
void readSensors() {
  unsigned long currentMillis = millis();

  /* -------- SOUND SENSOR -------- */
  int soundValue = analogRead(MIC_PIN);
  Blynk.virtualWrite(V2, soundValue);

  if (soundValue > SOUND_THRESHOLD &&
      (currentMillis - lastSoundAlert > ALERT_COOLDOWN)) {

    Serial.println("[SOUND ALERT] High sound detected!");
    Blynk.logEvent("sound_alert", "High sound detected!");
    beepOnce();
    lastSoundAlert = currentMillis;
  }

  /* -------- MPU6050 -------- */
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  float totalAccel = sqrt(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y +
    accel.acceleration.z * accel.acceleration.z
  );

  float vibration = abs(totalAccel - 9.8);
  Blynk.virtualWrite(V3, vibration);

  if (vibration > VIB_THRESHOLD &&
      (currentMillis - lastVibAlert > ALERT_COOLDOWN)) {

    Serial.println("[VIBRATION ALERT] Abnormal vibration detected!");
    Blynk.logEvent("vibration_alert", "Abnormal vibration detected!");
    beepOnce();
    lastVibAlert = currentMillis;
  }

  Serial.print("Sound: ");
  Serial.print(soundValue);
  Serial.print(" | Vibration: ");
  Serial.println(vibration);
}

/* -------------------- LOOP -------------------- */
void loop() {
  Blynk.run();
  timer.run();
}