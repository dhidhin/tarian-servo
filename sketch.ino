#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// =========================================================================
// KONFIGURASI PIN & PERANGKAT
// =========================================================================
const int JUMLAH_PASANG      = 6;
const int PIN_POT_KECEPATAN  = 34; // Input ADC1 Slide Potentiometer
const int PIN_NEOPIXEL       = 13; // Output RMT NeoPixel Ring (12 LED)
const int JUMLAH_LED         = 12;
const int PIN_BUZZER         = 15; // Output PWM Buzzer Pasif
const int PIN_OLED_SDA       = 16; // I2C SDA
const int PIN_OLED_SCL       = 4;  // I2C SCL

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_NeoPixel strip(JUMLAH_LED, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

Servo servoKanan[JUMLAH_PASANG]; // 6 servo hadap kanan (sisi kiri kanvas)
Servo servoKiri[JUMLAH_PASANG];  // 6 servo hadap kiri (sisi kanan kanvas)

const int pinHadapKanan[JUMLAH_PASANG] = {32, 33, 25, 26, 27, 14};
const int pinHadapKiri[JUMLAH_PASANG]  = {23, 22, 21, 19, 18, 17};

// Tangga nada pentatonik Gamelan Pelog/Slendro (Hz) untuk iringan tarian
const int NADA_PENTATONIK[8] = {262, 294, 330, 392, 440, 523, 587, 659};

// State global untuk tampilan OLED & iringan audio/LED
int babakAktif = 1;
const char* namaBabakAktif = "Inisialisasi...";
uint16_t warnaTemaHue = 0;
int langkahKetukanTerakhir = -1;
unsigned long waktuUpdateOLED = 0;

// =========================================================================
// FUNGSI KONTROL KECEPATAN, AUDIO, LED & OLED
// =========================================================================
float bacaKecepatanGlobal() {
  int nilaiADC = analogRead(PIN_POT_KECEPATAN);
  return 0.25 + (nilaiADC / 4095.0) * 2.75; // 0.25x s/d 3.00x
}

// Memainkan iringan nada pentatonik non-blocking mengikuti tempo virtual (t)
void updateAudioIrama(float t, int polaIrama) {
  int ketukan = (int)(t * 4.0); // 4 ketukan per detik virtual
  if (ketukan != langkahKetukanTerakhir) {
    langkahKetukanTerakhir = ketukan;
    if (polaIrama == 0) {
      noTone(PIN_BUZZER);
      return;
    }
    int idxNada = (ketukan * polaIrama + babakAktif) % 8;
    // Aksen ketukan ringan (durasi singkat 45ms agar tidak mengganggu loop)
    if (ketukan % 2 == 0 || polaIrama >= 2) {
      tone(PIN_BUZZER, NADA_PENTATONIK[idxNada], 45);
    }
  }
}

// Memperbarui warna 12 LED NeoPixel sesuai sudut masing-masing servo
void updateNeoPixel() {
  for (int i = 0; i < JUMLAH_PASANG; i++) {
    int sudutKanan = servoKanan[i].read();
    int sudutKiri  = servoKiri[i].read();

    // LED 0..5 mewakili 6 servo kiri (atas ke bawah)
    uint16_t hueKanan = warnaTemaHue + map(sudutKanan, 10, 170, 0, 18000);
    uint8_t valKanan  = map(sudutKanan, 10, 170, 60, 255);
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hueKanan, 240, valKanan)));

    // LED 11..6 mewakili 6 servo kanan (atas ke bawah)
    uint16_t hueKiri = warnaTemaHue + map(sudutKiri, 10, 170, 0, 18000);
    uint8_t valKiri  = map(sudutKiri, 10, 170, 60, 255);
    strip.setPixelColor(11 - i, strip.gamma32(strip.ColorHSV(hueKiri, 240, valKiri)));
  }
  strip.show();
}

// Memperbarui layar OLED setiap ~120ms agar tidak membebani frame rate servo
void updateOLED(float progres01) {
  if (millis() - waktuUpdateOLED < 120) return;
  waktuUpdateOLED = millis();

  float kec = bacaKecepatanGlobal();
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Header Babak
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("TARIAN SERVO [");
  display.print(babakAktif);
  display.print("/10]");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Nama Tarian
  display.setCursor(0, 16);
  display.println(namaBabakAktif);

  // Kecepatan Global dari Slide Potentiometer
  display.setCursor(0, 32);
  display.print("Tempo: ");
  display.print(kec, 2);
  display.print("x");

  // Progress Bar Babak
  progres01 = constrain(progres01, 0.0, 1.0);
  display.drawRect(0, 48, 128, 12, SSD1306_WHITE);
  display.fillRect(2, 50, (int)(124 * progres01), 8, SSD1306_WHITE);

  display.display();
}

void mulaiBabak(int nomor, const char* nama, uint16_t hueTema) {
  babakAktif = nomor;
  namaBabakAktif = nama;
  warnaTemaHue = hueTema;
  langkahKetukanTerakhir = -1;
  waktuUpdateOLED = 0;

  Serial.print("Babak ");
  Serial.print(nomor);
  Serial.print("/10: ");
  Serial.print(nama);
  Serial.print(" | Kecepatan: ");
  Serial.print(bacaKecepatanGlobal(), 2);
  Serial.println("x");

  updateOLED(0.0);
}

void delayDinamis(int durasiMs) {
  float progres = 0.0;
  while (progres < durasiMs) {
    float kec = bacaKecepatanGlobal();
    progres += 20.0 * kec;
    updateNeoPixel();
    delay(20);
  }
}

// =========================================================================
// KONTROL GERAK SERVO
// =========================================================================
void setSimetris(int baris, float sudut) {
  sudut = constrain(sudut, 10, 170);
  servoKanan[baris].write((int)sudut);
  servoKiri[baris].write((int)(180.0 - sudut));
}

void setBebas(int baris, float sudutKanan, float sudutKiri) {
  servoKanan[baris].write((int)constrain(sudutKanan, 10, 170));
  servoKiri[baris].write((int)constrain(sudutKiri, 10, 170));
}

void kembaliNetral(int durasiMs) {
  noTone(PIN_BUZZER);
  float awalKanan[JUMLAH_PASANG];
  float awalKiri[JUMLAH_PASANG];

  for (int i = 0; i < JUMLAH_PASANG; i++) {
    awalKanan[i] = servoKanan[i].read();
    awalKiri[i]  = servoKiri[i].read();
  }

  float t = 0.0;
  while (t < 1.0) {
    t += (20.0 / durasiMs) * bacaKecepatanGlobal();
    if (t > 1.0) t = 1.0;

    float ease = t * t * (3.0 - 2.0 * t);
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      setBebas(i,
        awalKanan[i] + (90.0 - awalKanan[i]) * ease,
        awalKiri[i]  + (90.0 - awalKiri[i])  * ease
      );
    }
    updateNeoPixel();
    delay(20);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_POT_KECEPATAN, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // Inisialisasi I2C OLED pada pin khusus (SDA=16, SCL=4)
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED SSD1306 gagal diinisialisasi!");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(12, 24);
  display.println("TARIAN SERVO 12-DOF");
  display.display();

  // Inisialisasi NeoPixel
  strip.begin();
  strip.setBrightness(200);
  strip.show();

  // Inisialisasi 12 Servo
  for (int i = 0; i < JUMLAH_PASANG; i++) {
    servoKanan[i].attach(pinHadapKanan[i]);
    servoKiri[i].attach(pinHadapKiri[i]);
    setSimetris(i, 90);
  }
  updateNeoPixel();
  delay(800);
}

// =========================================================================
// 10 BABAK KOREOGRAFI TARIAN
// =========================================================================
void tarianKupuKupu(float durasi) {
  float t = 0.0;
  while (t < durasi) {
    t += 0.02 * bacaKecepatanGlobal();
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      setSimetris(i, 90.0 + 55.0 * sin(t * 3.2 - i * 0.35));
    }
    updateNeoPixel();
    updateAudioIrama(t, 1);
    updateOLED(t / durasi);
    delay(20);
  }
}

void tarianPitaSutra(float durasi) {
  float t = 0.0;
  while (t < durasi) {
    t += 0.02 * bacaKecepatanGlobal();
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      float sudut = 90.0 + sin(t * 4.0 - i * 0.75) * 40.0 + cos(t * 2.0 + i * 0.45) * 20.0;
      setSimetris(i, sudut);
    }
    updateNeoPixel();
    updateAudioIrama(t, 1);
    updateOLED(t / durasi);
    delay(20);
  }
}

void tarianBungaTeratai() {
  float t = 0.0;
  while (t < 1.0) {
    t += (1.0 / 60.0) * bacaKecepatanGlobal();
    if (t > 1.0) t = 1.0;
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      float d = constrain((t * 1.4) - (i * 0.08), 0.0, 1.0);
      float smooth = d * d * (3.0 - 2.0 * d);
      setSimetris(i, 90.0 - 65.0 * smooth);
    }
    updateNeoPixel();
    updateAudioIrama(t * 2.0, 1);
    updateOLED(t * 0.45);
    delay(20);
  }
  delayDinamis(250);

  t = 0.0;
  while (t < 1.0) {
    t += (1.0 / 80.0) * bacaKecepatanGlobal();
    if (t > 1.0) t = 1.0;
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      float d = constrain((t * 1.5) - (i * 0.1), 0.0, 1.0);
      float smooth = d * d * (3.0 - 2.0 * d);
      setSimetris(i, 25.0 + 130.0 * smooth);
    }
    updateNeoPixel();
    updateAudioIrama(2.0 + t * 2.5, 2);
    updateOLED(0.45 + t * 0.55);
    delay(20);
  }
  delayDinamis(350);
}

void tarianSpiralDNA(float durasi) {
  float t = 0.0;
  while (t < durasi) {
    t += 0.02 * bacaKecepatanGlobal();
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      float fase = t * 3.5 - i * 0.65;
      setBebas(i, 90.0 + 50.0 * sin(fase), 90.0 + 50.0 * sin(fase + PI));
    }
    updateNeoPixel();
    updateAudioIrama(t, 2);
    updateOLED(t / durasi);
    delay(20);
  }
}

void tarianKipasMerak(float durasi) {
  float t = 0.0;
  while (t < durasi) {
    t += 0.02 * bacaKecepatanGlobal();
    float napas = sin(t * 2.5);
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      setSimetris(i, 90.0 + (JUMLAH_PASANG - i) * 10.0 * napas);
    }
    updateNeoPixel();
    updateAudioIrama(t, 1);
    updateOLED(t / durasi);
    delay(20);
  }
}

void tarianSaman(float durasi) {
  float t = 0.0;
  while (t < durasi) {
    t += 0.02 * bacaKecepatanGlobal();
    float ketukan = sin(t * 7.5);
    float aksen = (ketukan >= 0) ? pow(ketukan, 0.5) : -pow(-ketukan, 0.5);
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      float arah = (i % 2 == 0) ? 1.0 : -1.0;
      setSimetris(i, 90.0 + 45.0 * aksen * arah);
    }
    updateNeoPixel();
    updateAudioIrama(t * 1.5, 3); // Ketukan cepat untuk Tari Saman
    updateOLED(t / durasi);
    delay(20);
  }
}

void tarianVortex(float durasi) {
  float t = 0.0;
  while (t < durasi) {
    t += 0.02 * bacaKecepatanGlobal();
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      float pos = (float)i / (JUMLAH_PASANG - 1);
      float ampl = 25.0 + 35.0 * sin(t * 1.5);
      setSimetris(i, 90.0 + sin(t * 4.5 + pos * PI * 2.0) * ampl);
    }
    updateNeoPixel();
    updateAudioIrama(t, 2);
    updateOLED(t / durasi);
    delay(20);
  }
}

void tarianPendulumWave(float durasi) {
  float t = 0.0;
  while (t < durasi) {
    t += 0.02 * bacaKecepatanGlobal();
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      float freq = 2.6 + i * 0.38;
      setSimetris(i, 90.0 + 55.0 * sin(t * freq));
    }
    updateNeoPixel();
    updateAudioIrama(t, 2);
    updateOLED(t / durasi);
    delay(20);
  }
}

void tarianOmbakDiagonal(float durasi) {
  float t = 0.0;
  while (t < durasi) {
    t += 0.02 * bacaKecepatanGlobal();
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      float kanan = 90.0 + 55.0 * sin(t * 4.2 - i * 0.55);
      float kiri  = 90.0 - 55.0 * sin(t * 4.2 - i * 0.55 - 0.85);
      setBebas(i, kanan, kiri);
    }
    updateNeoPixel();
    updateAudioIrama(t, 2);
    updateOLED(t / durasi);
    delay(20);
  }
}

void tarianPenutup() {
  float t = 0.0;
  while (t < 1.0) {
    t += (1.0 / 40.0) * bacaKecepatanGlobal();
    if (t > 1.0) t = 1.0;
    float ease = t * t * (3.0 - 2.0 * t);
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      setSimetris(i, 90.0 + 65.0 * ease);
    }
    updateNeoPixel();
    updateAudioIrama(t * 2.0, 2);
    updateOLED(t * 0.4);
    delay(20);
  }
  delayDinamis(350);

  t = 0.0;
  while (t < 1.0) {
    t += (1.0 / 75.0) * bacaKecepatanGlobal();
    if (t > 1.0) t = 1.0;
    for (int i = 0; i < JUMLAH_PASANG; i++) {
      float p = constrain((t * 1.5) - (i * 0.09), 0.0, 1.0);
      float ease = p * p * (3.0 - 2.0 * p);
      setSimetris(i, 155.0 - 120.0 * ease);
    }
    updateNeoPixel();
    updateAudioIrama(2.0 + t * 2.0, 1);
    updateOLED(0.4 + t * 0.6);
    delay(20);
  }
  tone(PIN_BUZZER, 523, 350); // Nada penutup panjang
  delayDinamis(850);
}

void loop() {
  mulaiBabak(1, "Kepak Kupu-Kupu", 42000);   // Biru-Cyan
  tarianKupuKupu(4.5);
  kembaliNetral(500);

  mulaiBabak(2, "Gelombang Pita Sutra", 52000); // Magenta-Ungu
  tarianPitaSutra(4.5);
  kembaliNetral(500);

  mulaiBabak(3, "Mekar Bunga Teratai", 60000); // Merah Muda-Emas
  tarianBungaTeratai();
  kembaliNetral(600);

  mulaiBabak(4, "Spiral DNA", 22000);          // Hijau Zamrud
  tarianSpiralDNA(4.5);
  kembaliNetral(500);

  mulaiBabak(5, "Formasi Kipas Merak", 32000); // Toska-Biru
  tarianKipasMerak(4.5);
  kembaliNetral(500);

  mulaiBabak(6, "Tari Saman Aceh", 4000);      // Merah-Oranye
  tarianSaman(4.5);
  kembaliNetral(500);

  mulaiBabak(7, "Pusaran Vortex", 46000);      // Ungu Elektrik
  tarianVortex(4.5);
  kembaliNetral(500);

  mulaiBabak(8, "Pendulum Wave", 10000);       // Kuning Emas
  tarianPendulumWave(5.5);
  kembaliNetral(500);

  mulaiBabak(9, "Riak Ombak Diagonal", 36000); // Biru Laut
  tarianOmbakDiagonal(4.5);
  kembaliNetral(500);

  mulaiBabak(10, "Penutup Hormat Agung", 8000); // Emas Hangat
  tarianPenutup();
  kembaliNetral(1100);
}
