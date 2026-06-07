#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// === Pin Konfigurasi ===
#define SOIL_PIN         35
#define TRIG_IN          13
#define ECHO_IN          12
#define TRIG_BASAH       27
#define ECHO_BASAH       26
#define TRIG_KERING      25
#define ECHO_KERING      33
#define SERVO_PIN        14

// === Traffic Light Sampah Masuk === 
#define LED_MERAH_IN     23
#define LED_KUNING_IN    19
#define LED_HIJAU_IN     18

// === Traffic Light Kapasitas Basah ===
#define LED_BASAH_MERAH   5
#define LED_BASAH_KUNING  4
#define LED_BASAH_HIJAU   2

// === Traffic Light Kapasitas Kering ===
#define LED_KERING_MERAH   15
#define LED_KERING_KUNING  17
#define LED_KERING_HIJAU   16

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;

const int DRY_VALUE = 3200;
const int WET_VALUE = 2000;
const int OBSTACLE_THRESHOLD = 12;

long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration * 0.034 / 2;
}

// Fungsi untuk menyalakan 1 dari 3 LED (mode traffic light)
void nyalakanTrafficLED(int hijau, int kuning, int merah, int persen) {
  if (persen <= 74) {
    digitalWrite(hijau, HIGH);
    digitalWrite(kuning, LOW);
    digitalWrite(merah, LOW);
  } else if (persen <= 95) {
    digitalWrite(hijau, LOW);
    digitalWrite(kuning, HIGH);
    digitalWrite(merah, LOW);
  } else {
    digitalWrite(hijau, LOW);
    digitalWrite(kuning, LOW);
    digitalWrite(merah, HIGH);
  }
}

// Fungsi nyalakan LED indikator sampah masuk
void nyalakanLEDMasuk(bool merah, bool kuning, bool hijau) {
  digitalWrite(LED_MERAH_IN, merah);
  digitalWrite(LED_KUNING_IN, kuning);
  digitalWrite(LED_HIJAU_IN, hijau);
}

// Fungsi kedip traffic light saat awal menyala
void kedipTrafficLight(int jumlahKedipan, int delayWaktu) {
  for (int i = 0; i < jumlahKedipan; i++) {
    nyalakanLEDMasuk(1, 1, 1);
    delay(delayWaktu);
    nyalakanLEDMasuk(0, 0, 0);
    delay(delayWaktu);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  // Pin Ultrasonik
  pinMode(TRIG_IN, OUTPUT); pinMode(ECHO_IN, INPUT);
  pinMode(TRIG_BASAH, OUTPUT); pinMode(ECHO_BASAH, INPUT);
  pinMode(TRIG_KERING, OUTPUT); pinMode(ECHO_KERING, INPUT);

  // LED Masuk
  pinMode(LED_MERAH_IN, OUTPUT);
  pinMode(LED_KUNING_IN, OUTPUT);
  pinMode(LED_HIJAU_IN, OUTPUT);

  // LED Basah
  pinMode(LED_BASAH_MERAH, OUTPUT);
  pinMode(LED_BASAH_KUNING, OUTPUT);
  pinMode(LED_BASAH_HIJAU, OUTPUT);

  // LED Kering
  pinMode(LED_KERING_MERAH, OUTPUT);
  pinMode(LED_KERING_KUNING, OUTPUT);
  pinMode(LED_KERING_HIJAU, OUTPUT);

  servo.attach(SERVO_PIN);
  servo.write(90);

  lcd.init(); lcd.backlight();
  lcd.setCursor(3, 0); 
  lcd.print("BeLing 2.0");
  lcd.setCursor(4, 1);
  lcd.print("Welcome");
  delay(2000); lcd.clear();

  kedipTrafficLight(3, 400);
}

void loop() {
  nyalakanLEDMasuk(0, 1, 0); // Idle = kuning nyala

  long distance = readUltrasonic(TRIG_IN, ECHO_IN);

  if (distance > 0 && distance < OBSTACLE_THRESHOLD) {
    int soilValue = analogRead(SOIL_PIN);
    int moisturePercent = map(soilValue, DRY_VALUE, WET_VALUE, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    String jenisSampah;

    if (moisturePercent <= 25) {
      jenisSampah = "Kering";
      servo.write(135);
      nyalakanLEDMasuk(1, 0, 0); // Merah
    } else {
      jenisSampah = "Basah";
      servo.write(45);
      nyalakanLEDMasuk(0, 0, 1); // Hijau
    }

    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Sampah  " + jenisSampah);
    lcd.setCursor(3, 3);
    lcd.print("Terdeteksi");
    delay(2000);
    servo.write(90);
  }

  long capBasah = readUltrasonic(TRIG_BASAH, ECHO_BASAH);
  long capKering = readUltrasonic(TRIG_KERING, ECHO_KERING);

  int persenBasah = constrain(map(capBasah, 30, 3, 0, 100), 0, 100);
  int persenKering = constrain(map(capKering, 30, 3, 0, 100), 0, 100);

  // Tampilkan peringatan jika ada yang penuh
if (persenBasah >= 100 || persenKering >= 100) {
  lcd.clear();
  lcd.setCursor(5, 1);
  lcd.print("Penuh!");
  lcd.setCursor(1, 0);
  if (persenBasah >= 100 && persenKering >= 100) {
    lcd.print("Basah & Kering");
  } else if (persenBasah >= 100) {
    lcd.print("Sampah Basah");
  } else {
    lcd.print("Sampah Kering");
  }

  delay(2000); // Tahan 3 detik sebelum kembali
}

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Silahkan Masukan");
  lcd.setCursor(2, 1);
  lcd.print("Sampah Anda");

  nyalakanTrafficLED(LED_BASAH_HIJAU, LED_BASAH_KUNING, LED_BASAH_MERAH, persenBasah);
  nyalakanTrafficLED(LED_KERING_HIJAU, LED_KERING_KUNING, LED_KERING_MERAH, persenKering);

  delay(2000);
}