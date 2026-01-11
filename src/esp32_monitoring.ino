// Kod buraya gelecek
/*
 * ESP32 TAM İZLEME SİSTEMİ + TELNET DESTEĞİ
 * - ADXL345 İvmeölçer Sensörü (I2C)
 * - MAX4466 Mikrofon Sensörü (Analog)
 * - WiFi Bağlantısı
 * - Web Server (Veri Görüntüleme)
 * - Telnet Server (CMD ile izleme)
 * - ThingSpeak IoT Entegrasyonu
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <HTTPClient.h>

#include "config.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
String thingSpeakAPI = THINGSPEAK_API_KEY;
const char* thingSpeakServer = "http://api.thingspeak.com/update";

// ==================== PİN TANIMLARI ====================
#define MIC_PIN 34              // MAX4466 Analog Çıkış
#define LED_NORMAL 17           // Normal Durum LED (Yeşil)
#define LED_ALARM 16            // Alarm LED (Kırmızı)
#define BUZZER_PIN 18           // Buzzer
#define SDA_PIN 21              // I2C Data
#define SCL_PIN 22              // I2C Clock

// ==================== EŞIK DEĞERLERİ ====================
#define SOUND_THRESHOLD 2000         // Ses seviyesi eşiği (0-4095)
#define VIBRATION_THRESHOLD 10.0     // Titreşim eşiği (m/s²)
#define SAMPLE_WINDOW 50             // Ses örnekleme süresi (ms)
#define THINGSPEAK_DELAY 15000       // ThingSpeak gönderim aralığı (15 saniye)

// ==================== GLOBAL DEĞİŞKENLER ====================
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
WebServer server(80);

// *** TELNET SERVER EKLEME ***
WiFiServer telnetServer(23);  // Telnet portu
WiFiClient telnetClient;

// Sensör verileri
float accelX = 0, accelY = 0, accelZ = 0;
float totalAccel = 0;
int soundLevel = 0;
bool alarmActive = false;

// Zamanlayıcılar
unsigned long lastThingSpeakUpdate = 0;
unsigned long lastSerialPrint = 0;
unsigned long lastTelnetSend = 0;  // Telnet için

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n╔═══════════════════════════════════════╗");
  Serial.println("║   ESP32 TAM İZLEME SİSTEMİ v2.1      ║");
  Serial.println("║   + TELNET/CMD DESTEĞİ               ║");
  Serial.println("╚═══════════════════════════════════════╝\n");
  
  // GPIO Ayarları
  pinMode(LED_NORMAL, OUTPUT);
  pinMode(LED_ALARM, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MIC_PIN, INPUT);
  
  // ADC Ayarları
  analogReadResolution(12);  // 12-bit (0-4095)
  analogSetAttenuation(ADC_11db);  // 0-3.6V arası
  
  // I2C Başlat
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // ADXL345 Başlat
  Serial.print("🔧 ADXL345 başlatılıyor...");
  if (!accel.begin()) {
    Serial.println(" ❌ HATA!");
    Serial.println("⚠  ADXL345 bulunamadı! Bağlantıları kontrol edin.");
    while (1) {
      digitalWrite(LED_ALARM, HIGH);
      delay(200);
      digitalWrite(LED_ALARM, LOW);
      delay(200);
    }
  }
  Serial.println(" ✅ Başarılı!");
  accel.setRange(ADXL345_RANGE_16_G);
  
  // WiFi Bağlantısı
  connectWiFi();
  
  // Web Server Rotaları
  setupWebServer();
  
  // *** TELNET SERVER BAŞLAT ***
  setupTelnet();
  
  Serial.println("\n✅ SİSTEM HAZIR!\n");
  digitalWrite(LED_NORMAL, HIGH);
}

// ==================== ANA DÖNGÜ ====================
void loop() {
  // Web server isteklerini işle
  server.handleClient();
  
  // *** TELNET İSTEKLERİNİ İŞLE ***
  handleTelnet();
  
  // Sensör verilerini oku
  readSensors();
  
  // Alarm kontrolü
  checkAlarms();
  
  // Serial çıktı (her 500ms)
  if (millis() - lastSerialPrint > 500) {
    printSensorData();
    lastSerialPrint = millis();
  }
  
  // ThingSpeak'e veri gönder (her 15 saniye)
  if (millis() - lastThingSpeakUpdate > THINGSPEAK_DELAY) {
    sendToThingSpeak();
    lastThingSpeakUpdate = millis();
  }
  
  delay(100);
}

// ==================== WiFi BAĞLANTI ====================
void connectWiFi() {
  Serial.print("📡 WiFi'ye bağlanılıyor");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" ✅ Bağlandı!");
    Serial.print("📍 IP Adresi: ");
    Serial.println(WiFi.localIP());
    Serial.print("📶 Sinyal Gücü: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println(" ❌ HATA!");
    Serial.println("⚠  WiFi bağlantısı kurulamadı!");
  }
}

// ==================== TELNET KURULUMU ====================
void setupTelnet() {
  telnetServer.begin();
  telnetServer.setNoDelay(true);
  
  Serial.println("📞 Telnet Server başlatıldı! (Port: 23)");
  Serial.println("\n========================================");
  Serial.println("  WINDOWS CMD İLE BAĞLANMA:");
  Serial.println("========================================");
  Serial.println("1. Windows tuşuna basın");
  Serial.println("2. 'cmd' yazıp Enter'a basın");
  Serial.println("3. Şu komutu yazın:\n");
  Serial.print("   telnet ");
  Serial.println(WiFi.localIP());
  Serial.println("\n========================================\n");
}

// ==================== TELNET İŞLEYİCİ ====================
void handleTelnet() {
  // Yeni client bağlantısı kontrolü
  if (!telnetClient || !telnetClient.connected()) {
    if (telnetClient) {
      telnetClient.stop();
    }
    
    telnetClient = telnetServer.available();
    
    if (telnetClient) {
      Serial.println("\n[TELNET] Yeni client bağlandı!");
      Serial.print("[TELNET] IP: ");
      Serial.println(telnetClient.remoteIP());
      
      // Hoş geldin mesajı
      telnetClient.println("\n╔═══════════════════════════════════════╗");
      telnetClient.println("║   ESP32 İZLEME SİSTEMİ - TELNET      ║");
      telnetClient.println("╚═══════════════════════════════════════╝");
      telnetClient.println("\n✅ Bağlantı başarılı!");
      telnetClient.println("📊 Veri akışı başladı...\n");
      telnetClient.println("💡 Komutlar için 'help' yazın\n");
      
      // LED sinyali
      for(int i = 0; i < 3; i++) {
        digitalWrite(LED_NORMAL, LOW);
        delay(50);
        digitalWrite(LED_NORMAL, HIGH);
        delay(50);
      }
    }
  }
  
  // Client bağlıysa işle
  if (telnetClient && telnetClient.connected()) {
    
    // Periyodik veri gönderimi (her 1 saniye)
    if (millis() - lastTelnetSend > 1000) {
      sendTelnetData();
      lastTelnetSend = millis();
    }
    
    // Client'tan gelen komutları oku
    if (telnetClient.available()) {
      String command = telnetClient.readStringUntil('\n');
      command.trim();
      command.toLowerCase();
      
      handleTelnetCommand(command);
    }
    
    // Bağlantı kontrolü
    if (!telnetClient.connected()) {
      Serial.println("\n[TELNET] Client bağlantısı kesildi\n");
      telnetClient.stop();
    }
  }
}

// ==================== TELNET VERİ GÖNDERİMİ ====================
void sendTelnetData() {
  if (!telnetClient || !telnetClient.connected()) return;
  
  // Zaman bilgisi
  unsigned long saniye = millis() / 1000;
  unsigned long dakika = saniye / 60;
  unsigned long saat = dakika / 60;
  
  // Alarm durumu
  String durum = alarmActive ? "⚠ ALARM!" : "✅ Normal";
  
  // Ana veri satırı
  char buffer[256];
  sprintf(buffer, "[%02lu:%02lu:%02lu] X:%.2f Y:%.2f Z:%.2f | Toplam:%.2f m/s² | Ses:%d | %s",
          saat % 24, dakika % 60, saniye % 60,
          accelX, accelY, accelZ, totalAccel, soundLevel, durum.c_str());
  
  telnetClient.println(buffer);
}

// ==================== TELNET KOMUT İŞLEYİCİ ====================
void handleTelnetCommand(String cmd) {
  Serial.print("[TELNET] Komut alındı: ");
  Serial.println(cmd);
  
  if (cmd == "help" || cmd == "yardim") {
    telnetClient.println("\n╔══════════ KOMUTLAR ══════════╗");
    telnetClient.println("║ help      - Bu menü          ║");
    telnetClient.println("║ info      - Sistem bilgileri ║");
    telnetClient.println("║ sensor    - Sensör detayları ║");
    telnetClient.println("║ threshold - Eşik değerleri   ║");
    telnetClient.println("║ alarm on  - Alarmı aç        ║");
    telnetClient.println("║ alarm off - Alarmı kapat     ║");
    telnetClient.println("║ buzzer    - Buzzer test      ║");
    telnetClient.println("║ led       - LED test         ║");
    telnetClient.println("║ reset     - Sistemi resetle  ║");
    telnetClient.println("║ clear     - Ekranı temizle   ║");
    telnetClient.println("╚══════════════════════════════╝\n");
  }
  else if (cmd == "info") {
    telnetClient.println("\n╔═══════ SİSTEM BİLGİLERİ ═══════╗");
    telnetClient.print("║ IP Adresi  : ");
    telnetClient.println(WiFi.localIP());
    telnetClient.print("║ MAC Adresi : ");
    telnetClient.println(WiFi.macAddress());
    telnetClient.print("║ WiFi SSID  : ");
    telnetClient.println(ssid);
    telnetClient.print("║ WiFi RSSI  : ");
    telnetClient.print(WiFi.RSSI());
    telnetClient.println(" dBm");
    telnetClient.print("║ Uptime     : ");
    telnetClient.print(millis() / 1000);
    telnetClient.println(" saniye");
    telnetClient.print("║ Free Heap  : ");
    telnetClient.print(ESP.getFreeHeap());
    telnetClient.println(" byte");
    telnetClient.println("╚═════════════════════════════════╝\n");
  }
  else if (cmd == "sensor") {
    telnetClient.println("\n╔═══════ SENSÖR DETAYLARI ═══════╗");
    telnetClient.print("║ ADXL345 X : ");
    telnetClient.print(accelX, 2);
    telnetClient.println(" m/s²");
    telnetClient.print("║ ADXL345 Y : ");
    telnetClient.print(accelY, 2);
    telnetClient.println(" m/s²");
    telnetClient.print("║ ADXL345 Z : ");
    telnetClient.print(accelZ, 2);
    telnetClient.println(" m/s²");
    telnetClient.print("║ Toplam    : ");
    telnetClient.print(totalAccel, 2);
    telnetClient.println(" m/s²");
    telnetClient.print("║ MAX4466   : ");
    telnetClient.print(soundLevel);
    telnetClient.println(" / 4095");
    telnetClient.print("║ Durum     : ");
    telnetClient.println(alarmActive ? "⚠ ALARM AKTİF" : "✅ Normal");
    telnetClient.println("╚═════════════════════════════════╝\n");
  }
  else if (cmd == "threshold") {
    telnetClient.println("\n╔═══════ EŞIK DEĞERLERİ ═══════╗");
    telnetClient.print("║ Titreşim : ");
    telnetClient.print(VIBRATION_THRESHOLD);
    telnetClient.println(" m/s²");
    telnetClient.print("║ Ses      : ");
    telnetClient.print(SOUND_THRESHOLD);
    telnetClient.println(" / 4095");
    telnetClient.println("╚═══════════════════════════════╝\n");
  }
  else if (cmd == "alarm on") {
    alarmActive = true;
    telnetClient.println("\n✅ Alarm manuel olarak aktifleştirildi\n");
  }
  else if (cmd == "alarm off") {
    alarmActive = false;
    telnetClient.println("\n✅ Alarm manuel olarak kapatıldı\n");
  }
  else if (cmd == "buzzer") {
    telnetClient.println("\n🔊 Buzzer testi yapılıyor...\n");
    tone(BUZZER_PIN, 2000, 200);
    delay(300);
    tone(BUZZER_PIN, 2500, 200);
    telnetClient.println("✅ Test tamamlandı\n");
  }
  else if (cmd == "led") {
    telnetClient.println("\n💡 LED testi yapılıyor...\n");
    for(int i = 0; i < 5; i++) {
      digitalWrite(LED_NORMAL, LOW);
      digitalWrite(LED_ALARM, HIGH);
      delay(150);
      digitalWrite(LED_NORMAL, HIGH);
      digitalWrite(LED_ALARM, LOW);
      delay(150);
    }
    telnetClient.println("✅ Test tamamlandı\n");
  }
  else if (cmd == "reset") {
    telnetClient.println("\n⚠ ESP32 yeniden başlatılıyor...\n");
    telnetClient.flush();
    delay(1000);
    ESP.restart();
  }
  else if (cmd == "clear") {
    // ANSI escape code ile ekran temizleme
    telnetClient.print("\033[2J\033[H");
    telnetClient.println("✅ Ekran temizlendi\n");
  }
  else if (cmd.length() > 0) {
    telnetClient.print("\n❌ Bilinmeyen komut: ");
    telnetClient.println(cmd);
    telnetClient.println("'help' yazarak komut listesini görebilirsiniz.\n");
  }
}

// ==================== WEB SERVER KURULUMU ====================
void setupWebServer() {
  // Ana sayfa
  server.on("/", handleRoot);
  
  // JSON veri endpoint'i
  server.on("/data", handleData);
  
  // Sistem durumu
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("🌐 Web Server başlatıldı!");
  Serial.print("🔗 Tarayıcıdan erişim: http://");
  Serial.println(WiFi.localIP());
}

// ==================== WEB SAYFALARI ====================
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 İzleme Sistemi</title>";
  html += "<style>";
  html += "body{font-family:Arial;margin:0;padding:20px;background:#1a1a2e;color:#fff;}";
  html += ".container{max-width:800px;margin:0 auto;}";
  html += "h1{text-align:center;color:#00d9ff;}";
  html += ".card{background:#16213e;padding:20px;margin:15px 0;border-radius:10px;box-shadow:0 4px 6px rgba(0,0,0,0.3);}";
  html += ".sensor-value{font-size:2em;font-weight:bold;color:#00d9ff;margin:10px 0;}";
  html += ".status{padding:10px;border-radius:5px;text-align:center;font-weight:bold;}";
  html += ".normal{background:#27ae60;color:#fff;}";
  html += ".alarm{background:#e74c3c;color:#fff;animation:blink 1s infinite;}";
  html += "@keyframes blink{0%,100%{opacity:1;}50%{opacity:0.5;}}";
  html += ".info{color:#95a5a6;font-size:0.9em;}";
  html += "</style>";
  html += "<script>";
  html += "setInterval(function(){";
  html += "fetch('/data').then(r=>r.json()).then(d=>{";
  html += "document.getElementById('accelX').innerText=d.accelX.toFixed(2);";
  html += "document.getElementById('accelY').innerText=d.accelY.toFixed(2);";
  html += "document.getElementById('accelZ').innerText=d.accelZ.toFixed(2);";
  html += "document.getElementById('totalAccel').innerText=d.totalAccel.toFixed(2);";
  html += "document.getElementById('soundLevel').innerText=d.soundLevel;";
  html += "let status=document.getElementById('status');";
  html += "if(d.alarm){status.className='status alarm';status.innerText='⚠ ALARM AKTİF!';}";
  html += "else{status.className='status normal';status.innerText='✅ NORMAL DURUM';}";
  html += "});";
  html += "},1000);";
  html += "</script>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>🔧 ESP32 TAM İZLEME SİSTEMİ</h1>";
  
  html += "<div id='status' class='status normal'>✅ NORMAL DURUM</div>";
  
  html += "<div class='card'>";
  html += "<h2>📳 Titreşim Sensörü (ADXL345)</h2>";
  html += "<div class='info'>X Ekseni:</div>";
  html += "<div class='sensor-value'><span id='accelX'>0.00</span> m/s²</div>";
  html += "<div class='info'>Y Ekseni:</div>";
  html += "<div class='sensor-value'><span id='accelY'>0.00</span> m/s²</div>";
  html += "<div class='info'>Z Ekseni:</div>";
  html += "<div class='sensor-value'><span id='accelZ'>0.00</span> m/s²</div>";
  html += "<div class='info'>Toplam İvme:</div>";
  html += "<div class='sensor-value'><span id='totalAccel'>0.00</span> m/s²</div>";
  html += "</div>";
  
  html += "<div class='card'>";
  html += "<h2>🔊 Ses Sensörü (MAX4466)</h2>";
  html += "<div class='info'>Ses Seviyesi:</div>";
  html += "<div class='sensor-value'><span id='soundLevel'>0</span> / 4095</div>";
  html += "</div>";
  
  html += "<div class='card'>";
  html += "<h2>📞 Telnet Bağlantı</h2>";
  html += "<div class='info'>CMD'de bağlanmak için:</div>";
  html += "<div style='background:#000;padding:10px;margin:10px 0;border-radius:5px;font-family:monospace;'>";
  html += "telnet " + WiFi.localIP().toString() + " 23";
  html += "</div>";
  html += "</div>";
  
  html += "<div class='card'>";
  html += "<div class='info'>📡 WiFi: " + String(ssid) + "</div>";
  html += "<div class='info'>📶 RSSI: " + String(WiFi.RSSI()) + " dBm</div>";
  html += "<div class='info'>🕐 Çalışma Süresi: " + String(millis()/1000) + " saniye</div>";
  html += "</div>";
  
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"accelX\":" + String(accelX) + ",";
  json += "\"accelY\":" + String(accelY) + ",";
  json += "\"accelZ\":" + String(accelZ) + ",";
  json += "\"totalAccel\":" + String(totalAccel) + ",";
  json += "\"soundLevel\":" + String(soundLevel) + ",";
  json += "\"alarm\":" + String(alarmActive ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleStatus() {
  String status = "ESP32 TAM İZLEME SİSTEMİ\n\n";
  status += "WiFi: " + String(ssid) + "\n";
  status += "IP: " + WiFi.localIP().toString() + "\n";
  status += "RSSI: " + String(WiFi.RSSI()) + " dBm\n";
  status += "Uptime: " + String(millis()/1000) + " saniye\n";
  status += "Alarm: " + String(alarmActive ? "AKTİF" : "KAPALI") + "\n";
  
  server.send(200, "text/plain", status);
}

// ==================== SENSÖR OKUMA ====================
void readSensors() {
  // ADXL345 oku
  sensors_event_t event;
  accel.getEvent(&event);
  
  accelX = event.acceleration.x;
  accelY = event.acceleration.y;
  accelZ = event.acceleration.z;
  
  totalAccel = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
  
  // MAX4466 oku
  soundLevel = readSoundLevel();
}

int readSoundLevel() {
  unsigned long startMillis = millis();
  int peakToPeak = 0;
  int signalMax = 0;
  int signalMin = 4095;
  
  while (millis() - startMillis < SAMPLE_WINDOW) {
    int sample = analogRead(MIC_PIN);
    
    if (sample > signalMax) {
      signalMax = sample;
    }
    if (sample < signalMin) {
      signalMin = sample;
    }
  }
  
  peakToPeak = signalMax - signalMin;
  return peakToPeak;
}

// ==================== ALARM KONTROLÜ ====================
void checkAlarms() {
  bool vibrationAlarm = (totalAccel > VIBRATION_THRESHOLD);
  bool soundAlarm = (soundLevel > SOUND_THRESHOLD);
  
  alarmActive = vibrationAlarm || soundAlarm;
  
  if (alarmActive) {
    digitalWrite(LED_ALARM, HIGH);
    digitalWrite(LED_NORMAL, LOW);
    
    // Buzzer - kısa bip
    tone(BUZZER_PIN, 2500, 100);
  } else {
    digitalWrite(LED_ALARM, LOW);
    digitalWrite(LED_NORMAL, HIGH);
    noTone(BUZZER_PIN);
  }
}

// ==================== SERİAL ÇIKTI ====================
void printSensorData() {
  Serial.println("┌─────────────────────────────────────┐");
  Serial.print("│ Titreşim: X=");
  Serial.print(accelX, 2);
  Serial.print(" Y=");
  Serial.print(accelY, 2);
  Serial.print(" Z=");
  Serial.print(accelZ, 2);
  Serial.println(" m/s²");
  Serial.print("│ Toplam İvme: ");
  Serial.print(totalAccel, 2);
  Serial.println(" m/s²");
  Serial.print("│ Ses Seviyesi: ");
  Serial.print(soundLevel);
  Serial.println(" / 4095");
  Serial.print("│ Durum: ");
  Serial.println(alarmActive ? "⚠ ALARM!" : "✅ Normal");
  Serial.println("└─────────────────────────────────────┘\n");
}

// ==================== THINGSPEAK GÖNDERME ====================
void sendToThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi bağlantısı yok!");
    return;
  }
  
  HTTPClient http;
  
  String url = String(thingSpeakServer) + "?api_key=" + thingSpeakAPI;
  url += "&field1=" + String(accelX);
  url += "&field2=" + String(accelY);
  url += "&field3=" + String(accelZ);
  url += "&field4=" + String(totalAccel);
  url += "&field5=" + String(soundLevel);
  url += "&field6=" + String(alarmActive ? 1 : 0);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.println("📤 ThingSpeak'e veri gönderildi! Kod: " + String(httpCode));
  } else {
    Serial.println("❌ ThingSpeak hatası: " + String(httpCode));
  }
  
  http.end();
}
