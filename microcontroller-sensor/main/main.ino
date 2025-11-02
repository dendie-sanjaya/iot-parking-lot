// Program Gabungan: Sensor Parkir Ultrasonic, OLED Display, LED, dan MQTT
// Menggunakan ESP8266 (NodeMCU/Wemos D1 Mini)
// Pin Sensor dan LED DIKEMBALIKAN ke D1, D2, D5, D6 sesuai permintaan user.
// Pin I2C OLED dipindah ke D3 (SCL) dan D4 (SDA) untuk menghindari konflik sensor.

// --- LIBRARY YANG DIBUTUHKAN ---
// 1. ESP8266WiFi.h, PubSubClient, ArduinoJson
// 2. Adafruit GFX Library, Adafruit SSD1306 (untuk OLED)
#include <ESP8266WiFi.h> // DIKEMBALIKAN ke ESP8266
#include <PubSubClient.h>
#include <ArduinoJson.h> 
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- KONFIGURASI WIFI DAN MQTT ---
const char* ssid = "Mama adelia";
const char* password = "uu311009";
const char* mqtt_server = "192.168.100.35"; 
const char* mqtt_client_id = "ESP8266_001"; 
const char* topic_update = "parkir/slot/update"; 
const int SLOT_KODE = 1; 

WiFiClient espClient;
PubSubClient client(espClient);

// --- KONFIGURASI PIN HARDWARE (D-PIN ESP8266 sesuai permintaan user) ---
// Pin Sensor (Menggunakan pin I2C default ESP8266, sehingga I2C OLED harus dipindah)
#define TRIG_PIN D1 // GPIO 5
#define ECHO_PIN D2 // GPIO 4

// Pin LED
#define LED_MERAH D5 // GPIO 14 - Terisi
#define LED_HIJAU D6 // GPIO 12 - Kosong

// Pin I2C OLED (DIPINDAHKAN ke pin lain untuk menghindari konflik)
#define OLED_SDA D4 // GPIO 2 (Harap berhati-hati, pin ini HIGH saat boot)
#define OLED_SCL D3 // GPIO 0 (Harap berhati-hati, pin ini LOW saat boot)

// Batas Jarak Sensor
const float JARAK_PENUH_CM = 10.0; 

int last_status = -1; // Status terakhir yang dikirim

// --- KONFIGURASI OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
// Alamat I2C umum untuk OLED 0.96" adalah 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// ------------------------------------
// --- FUNGSI UTILITY ---
// ------------------------------------

// --- Fungsi Pengukuran Jarak ---
long measureDistance() {
  // Clear the trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Set the trigger pin high for 10 microseconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Measure the duration of the echo pin high pulse
  long duration = pulseIn(ECHO_PIN, HIGH, 25000); 
  // Calculate distance in cm (Speed of sound = 343 m/s or 0.0343 cm/µs)
  long distanceCm = duration * 0.0343 / 2;
  
  // Return a large value for out-of-range or error
  if (distanceCm <= 0 || distanceCm > 400) return 999; 
  return distanceCm;
}

// --- Fungsi Mengirim Status via JSON MQTT ---
void publishStatus(int status) {
  DynamicJsonDocument doc(64); 
  doc["slot_kode"] = SLOT_KODE; 
  doc["status"] = status; // 1=Terisi, 0=Kosong

  char jsonBuffer[64];
  serializeJson(doc, jsonBuffer);
  
  if (client.publish(topic_update, jsonBuffer)) {
    Serial.print("Status dikirim: ");
    Serial.println(jsonBuffer);
    last_status = status; // Update status terakhir
  } else {
    Serial.println("Gagal mengirim pesan MQTT.");
  }
}

// --- Fungsi Update Tampilan OLED ---
void updateOLED(long distance, int status) {
  display.clearDisplay();
  
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Slot Parkir: A0");
  display.println(SLOT_KODE); 
  
  display.drawFastHLine(0, 12, SCREEN_WIDTH, SSD1306_WHITE); 

  // Tampilkan Jarak
  display.setTextSize(1); 
  display.setCursor(0, 20);
  display.print("Jarak: ");
  display.print(distance);
  display.println(" cm");

  // Tampilkan Status (TERISI/KOSONG)
  display.setTextSize(2); 
  display.setCursor(0, 45);

  if (status == 1) {
    display.print("FULL");
  } else {
    display.print("EMPTY");
  }

  display.display();
}

// --- Fungsi Koneksi WiFi ---
void setup_wifi() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.println(ssid);
  display.display();

  Serial.print("\nMenghubungkan ke ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.setCursor(0, 20);
    display.print(".");
    display.display();
  }
  
  Serial.println("\nWiFi Terhubung.");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected!");
  display.println(WiFi.localIP());
  display.display();
  delay(1000);
}

// --- Fungsi Koneksi Ulang MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Mencoba koneksi MQTT...");
    // Update OLED saat mencoba koneksi
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("MQTT Lost!");
    display.setTextSize(1);
    display.setCursor(0, 25);
    display.println("Retrying...");
    display.display();
    
    if (client.connect(mqtt_client_id)) {
      Serial.println("terhubung");
      // Update OLED saat terhubung
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println("MQTT OK");
      display.display();
      delay(500);
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

// ------------------------------------
// --- SETUP DAN LOOP UTAMA ---
// ------------------------------------

void setup() {
  Serial.begin(115200);
  
  // 1. Setup Pin Sensor dan LED
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(LED_HIJAU, OUTPUT);
  
  digitalWrite(LED_MERAH, LOW);
  digitalWrite(LED_HIJAU, HIGH); // Awalnya slot Kosong

  // 2. Setup OLED (I2C) - PENTING: Pindah I2C ke D3/D4
  // Pin I2C didefinisikan secara eksplisit untuk menggunakan D3/D4.
  Wire.begin(OLED_SDA, OLED_SCL); 
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 alokasi gagal"));
    for(;;); 
  }
  display.display();
  delay(100);
  display.clearDisplay();
  
  // 3. Setup WiFi dan MQTT
  setup_wifi();
  client.setServer(mqtt_server, 1883); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Pertahankan koneksi MQTT

  long jarak = measureDistance();
  int current_status;
  
  // --- Logika Kontrol Lampu Parkir dan Penentuan Status ---
  if (jarak <= JARAK_PENUH_CM && jarak > 0) {
    // KONDISI: TERISI
    digitalWrite(LED_MERAH, HIGH);
    digitalWrite(LED_HIJAU, LOW);
    current_status = 1; // Terisi
  } else {
    // KONDISI: KOSONG
    digitalWrite(LED_MERAH, LOW);
    digitalWrite(LED_HIJAU, HIGH);
    current_status = 0; // Kosong
  }
  
  // 1. Kirim status ke MQTT hanya jika ada perubahan status
  if (current_status != last_status) {
    publishStatus(current_status);
  }

  // 2. Update Tampilan OLED
  updateOLED(jarak, current_status);

  delay(1000); // Jeda 1 detik
}
