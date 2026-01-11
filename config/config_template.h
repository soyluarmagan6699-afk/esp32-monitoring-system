/*
 * YAPILANDIRMA DOSYASI
 * Bu dosyayı kopyalayıp 'config.h' olarak kaydedin
 * ve kendi bilgilerinizi girin
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== WiFi AYARLARI ====================
// Kendi WiFi bilgilerinizi girin
const char* WIFI_SSID = "Armağan iPhone’u";
const char* WIFI_PASSWORD = "soner2002";

// ==================== THINGSPEAK AYARLARI ====================
// ThingSpeak Write API Key'inizi girin
// https://thingspeak.com/channels/your_channel_id/api_keys
const char* THINGSPEAK_API_KEY = "08UBRYXLN9NGLUCH";
const char* THINGSPEAK_CHANNEL_ID = "3201694";

// ==================== SENSÖR PİN AYARLARI ====================
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

#endif
