# 🔧 ESP32 Tam İzleme Sistemi

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![ThingSpeak](https://img.shields.io/badge/IoT-ThingSpeak-orange.svg)](https://thingspeak.com/)

ESP32 tabanlı gerçek zamanlı titreşim ve ses izleme sistemi. ADXL345 ivmeölçer ve MAX4466 mikrofon sensörleri ile donatılmış, ThingSpeak IoT entegrasyonlu profesyonel izleme çözümü.

## ✨ Özellikler

- 📳 **Titreşim İzleme**: ADXL345 ivmeölçer ile 3 eksenli titreşim ölçümü
- 🔊 **Ses İzleme**: MAX4466 mikrofon ile yüksek hassasiyetli ses seviyesi tespiti
- 🌐 **Web Arayüzü**: Gerçek zamanlı veri görüntüleme ve izleme
- 📞 **Telnet Desteği**: CMD/Terminal üzerinden uzaktan yönetim
- ☁️ **ThingSpeak IoT**: Bulut tabanlı veri kaydı ve analiz
- 🚨 **Akıllı Alarm**: Özelleştirilebilir eşik değerleri ile otomatik uyarı
- 💡 **LED & Buzzer**: Görsel ve işitsel uyarı sistemi

## 🛠️ Donanım Gereksinimleri

### Elektronik Bileşenler
- ESP32 Development Board
- ADXL345 İvmeölçer Sensörü (I2C)
- MAX4466 Mikrofon Modülü
- LED x2 (Yeşil + Kırmızı)
- Buzzer (Aktif veya Pasif)
- Breadboard ve Jumper Kablolar

### Bağlantı Şeması

```
ESP32          ADXL345
-----------------------
GPIO 21 (SDA)  → SDA
GPIO 22 (SCL)  → SCL
3.3V           → VCC
GND            → GND

ESP32          MAX4466
-----------------------
GPIO 34        → OUT
3.3V           → VCC
GND            → GND

ESP32          LED/Buzzer
--------------------------
GPIO 17        → LED Yeşil (+)
GPIO 16        → LED Kırmızı (+)
GPIO 18        → Buzzer (+)
GND            → LED/Buzzer (-)
```

## 📦 Kurulum

### 1. Arduino IDE Hazırlığı

ESP32 board desteği ekleyin:
```
File > Preferences > Additional Board Manager URLs:
https://dl.espressif.com/dl/package_esp32_index.json
```

### 2. Gerekli Kütüphaneler

Arduino IDE > Tools > Manage Libraries:
- `Adafruit ADXL345` (by Adafruit)
- `Adafruit Unified Sensor` (by Adafruit)

### 3. Projeyi İndirme

```bash
# GitHub'dan indir
git clone https://github.com/KULLANICI_ADIN/esp32-izleme-sistemi.git
cd esp32-izleme-sistemi
```

Veya:
- GitHub sayfasında **Code** > **Download ZIP** tıklayın
- ZIP'i çıkarın

### 4. Yapılandırma Dosyası Oluşturma

1. `config` klasöründeki `config_template.h` dosyasını kopyalayın
2. `src` klasörüne `config.h` adıyla yapıştırın
3. `config.h` dosyasını açın ve bilgilerinizi girin:

```cpp
const char* WIFI_SSID = "SenindWiFiAdin";
const char* WIFI_PASSWORD = "SenindWiFiSifren";
const char* THINGSPEAK_API_KEY = "SenindThingSpeakAPIKeyin";
```

### 5. ESP32'ye Yükleme

1. Arduino IDE'de `src/esp32_monitoring.ino` dosyasını açın
2. **Board**: `ESP32 Dev Module` seçin
3. **Port**: ESP32'nizin bağlı olduğu portu seçin
4. **Upload** butonuna tıklayın
5. Seri monitörü açın (115200 baud)
6. ESP32'nin IP adresini not edin

## 🚀 Kullanım

### Web Arayüzü

Tarayıcıda ESP32'nin IP adresini açın:
```
http://192.168.X.X
```

### Telnet ile Bağlanma

**Windows CMD:**
```cmd
telnet 192.168.X.X 23
```

**Linux/Mac Terminal:**
```bash
telnet 192.168.X.X 23
```

### Telnet Komutları

| Komut | Açıklama |
|-------|----------|
| `help` | Komut listesini göster |
| `info` | Sistem bilgilerini göster |
| `sensor` | Sensör değerlerini göster |
| `threshold` | Eşik değerlerini göster |
| `alarm on` | Alarmı manuel aç |
| `alarm off` | Alarmı manuel kapat |
| `buzzer` | Buzzer testi yap |
| `led` | LED testi yap |
| `reset` | ESP32'yi yeniden başlat |
| `clear` | Ekranı temizle |

## ☁️ ThingSpeak Kurulumu

### 1. Hesap Oluşturma
- [ThingSpeak.com](https://thingspeak.com) adresine gidin
- **Sign Up** ile ücretsiz hesap oluşturun

### 2. Kanal Oluşturma
- **Channels** > **New Channel** tıklayın
- Kanal bilgileri:
  - **Name**: ESP32 Monitoring
  - **Field 1**: Accel X (m/s²)
  - **Field 2**: Accel Y (m/s²)
  - **Field 3**: Accel Z (m/s²)
  - **Field 4**: Total Accel (m/s²)
  - **Field 5**: Sound Level
  - **Field 6**: Alarm Status
- **Save Channel** tıklayın

### 3. API Key Alma
- **API Keys** sekmesine tıklayın
- **Write API Key**'i kopyalayın
- `config.h` dosyasına yapıştırın

## 📊 Dashboard Kullanımı

### Canlı Dashboard

Projenin canlı dashboard'unu görüntülemek için:
```
https://KULLANICI_ADIN.github.io/esp32-izleme-sistemi/dashboard/thingspeak_dashboard.html
```

### Dashboard'da Yapılacaklar:

1. **Channel ID** girin (ThingSpeak kanalınızdan)
2. **Read API Key** girin (ThingSpeak > API Keys)
3. **Veri Yükle** butonuna tıklayın
4. Veriler otomatik olarak her 16 saniyede güncellenecek

### Dashboard Özellikleri:
- ✅ Gerçek zamanlı grafik görüntüleme
- ✅ 3 eksenli titreşim analizi
- ✅ Ses seviyesi trendleri
- ✅ Alarm durumu göstergesi
- ✅ Otomatik veri güncelleme
- ✅ Responsive tasarım (mobil uyumlu)

## 🔧 Yapılandırma

### Eşik Değerlerini Ayarlama

`config.h` dosyasında:

```cpp
#define SOUND_THRESHOLD 2000         // Ses eşiği (0-4095)
#define VIBRATION_THRESHOLD 10.0     // Titreşim eşiği (m/s²)
```

**Öneriler:**
- Hassas izleme için değerleri düşürün
- Yanlış alarm azaltmak için değerleri yükseltin
- Test ederek optimal değerleri bulun

### Veri Gönderim Aralığı

```cpp
#define THINGSPEAK_DELAY 15000       // 15 saniye (minimum)
```

⚠️ **Not**: ThingSpeak ücretsiz hesaplarda minimum 15 saniye aralık gereklidir.

## 📖 Proje Yapısı

```
esp32-izleme-sistemi/
├── README.md                    # Bu dosya
├── LICENSE                      # MIT Lisansı
├── src/
│   └── esp32_monitoring.ino    # Ana Arduino kodu
├── config/
│   └── config_template.h       # Yapılandırma şablonu
├── dashboard/
│   └── thingspeak_dashboard.html  # Web dashboard
└── docs/
    └── README.md               # Ek dokümantasyon
```

## 🎯 Kullanım Senaryoları

- 🏭 **Sanayi**: Makine titreşim izleme
- 🏠 **Ev Güvenliği**: Kapı/pencere izleme
- 🔔 **Alarm Sistemi**: Ses ve hareket tespiti
- 📊 **Veri Toplama**: IoT projeleri için veri analizi
- 🎓 **Eğitim**: ESP32 ve sensör öğrenimi

## 🤝 Katkıda Bulunma

Katkılarınızı bekliyoruz! 

1. **Fork** yapın
2. Feature branch oluşturun: `git checkout -b yeni-ozellik`
3. Değişikliklerinizi commit edin: `git commit -m 'Yeni özellik eklendi'`
4. Branch'e push yapın: `git push origin yeni-ozellik`
5. **Pull Request** oluşturun

## 🐛 Sorun Bildirme

Bir sorun mu buldunuz? [Issue açın](https://github.com/KULLANICI_ADIN/esp32-izleme-sistemi/issues)

## 📝 Lisans

Bu proje MIT lisansı altında lisanslanmıştır - detaylar için [LICENSE](LICENSE) dosyasına bakın.

## 👤 Yazar

**Armağan SOYLU**
- GitHub: [@soyluarmagan6699-afk](https://github.com/KULLANICI_ADIN)
- Email: soyluarmagan6699@gmail.com

## 🙏 Teşekkürler

- **Adafruit** - ADXL345 kütüphanesi için
- **Espressif** - ESP32 platformu için
- **ThingSpeak** - IoT altyapısı için
- **Açık kaynak topluluğu** - Sürekli destek için

## 📞 İletişim ve Destek

- 💬 Sorularınız için: [Issue](https://github.com/KULLANICI_ADIN/esp32-izleme-sistemi/issues) açın
- 📧 Email: email@example.com
- 🌟 Projeyi beğendiyseniz yıldız vermeyi unutmayın!

---

## 🚀 Hızlı Başlangıç Özeti

```bash
# 1. Projeyi indir
git clone https://github.com/KULLANICI_ADIN/esp32-izleme-sistemi.git

# 2. config.h oluştur ve bilgilerini doldur
cp config/config_template.h src/config.h

# 3. Arduino IDE'de aç ve yükle
# src/esp32_monitoring.ino

# 4. Web arayüzünü aç
# http://[ESP32_IP_ADRESI]

# 5. Dashboard'u kullan
# https://KULLANICI_ADIN.github.io/esp32-izleme-sistemi/dashboard/thingspeak_dashboard.html
```

---

⭐ **Bu projeyi beğendiyseniz yıldız vermeyi unutmayın!**

🔔 **Watch** yaparak güncellemelerden haberdar olun!

🍴 **Fork** yaparak kendi versiyonunuzu oluşturun!
