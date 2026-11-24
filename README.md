# Tarian Servo --- ESP32 IoT Project

Proyek simulasi **robot penari 12-DOF berbasis ESP32** di Wokwi. Dua
belas servo bergerak dalam sepuluh rangkaian koreografi, dilengkapi efek
warna pada NeoPixel Ring, iringan nada melalui buzzer, serta informasi
babak dan tempo pada OLED.

> **Simulasi Wokwi:** [Tarian Servo
](https://wokwi.com/projects/480930266021257217)

## Daftar Isi

-   [Gambaran Umum](#gambaran-umum)
-   [Fitur](#fitur)
-   [Komponen](#komponen)
-   [Pemetaan Pin ESP32](#pemetaan-pin-esp32)
-   [Library](#library)
-   [Cara Menjalankan Simulasi](#cara-menjalankan-simulasi)
-   [Cara Kerja Program](#cara-kerja-program)
-   [Urutan Koreografi](#urutan-koreografi)
-   [Pengaturan Kecepatan](#pengaturan-kecepatan)
-   [Catatan Pengembangan dan Perangkat
    Keras](#catatan-pengembangan-dan-perangkat-keras)

## Gambaran Umum

Sistem ini menggerakkan **6 pasang servo** (12 servo total). Setiap
pasangan merepresentasikan dua sisi gerakan. Program mengatur sudut
servo secara simetris atau independen untuk menghasilkan pola gerak yang
berbeda.

Selama pertunjukan berlangsung:

-   NeoPixel Ring menampilkan warna yang berubah mengikuti tema babak
    dan sudut servo.
-   Buzzer memainkan nada pentatonik sebagai iringan.
-   OLED menampilkan nomor babak, nama koreografi, tempo, dan progress
    bar.
-   Slide potentiometer mengatur kecepatan keseluruhan gerakan.

Program berjalan otomatis dan mengulang urutan koreografi dari awal
setelah babak kesepuluh selesai.

## Fitur

-   Kontrol 12 servo menggunakan library `ESP32Servo`.
-   10 pola koreografi dengan variasi gerak sinusoidal, gelombang, dan
    transisi halus.
-   Pengaturan tempo secara langsung dari potentiometer, sekitar **0,25×
    sampai 3,00×**.
-   Efek warna pada 12 LED NeoPixel yang dipetakan terhadap sudut servo.
-   Iringan buzzer dengan delapan frekuensi nada pentatonik.
-   Tampilan status melalui OLED SSD1306 128 × 64 piksel.
-   Transisi kembali ke posisi netral (90°) di antara babak.

## Komponen

    No. Komponen                          Jumlah Fungsi
  ----- --------------------------- ------------ ---------------------------
      1 ESP32 DevKit V1                        1 Mikrokontroler utama
      2 Servo motor                           12 Penggerak mekanisme tari
      3 NeoPixel Ring (WS2812)        1 (12 LED) Efek pencahayaan
      4 OLED SSD1306 I2C 128 × 64              1 Informasi babak dan tempo
      5 Slide potentiometer                    1 Pengatur kecepatan
      6 Passive buzzer                         1 Iringan nada
      7 Kabel penghubung              Secukupnya Koneksi antarkomponen

Daftar di atas merangkum perangkat yang digunakan oleh program. Periksa
`diagram.json` pada project Wokwi untuk melihat susunan rangkaian
simulasi secara visual.

## Pemetaan Pin ESP32

### Servo motor

  ----------------------------------------------------------------------------------
  Kelompok            Servo 1    Servo 2    Servo 3    Servo 4    Servo 5    Servo 6
  ---------------- ---------- ---------- ---------- ---------- ---------- ----------
  Sisi kanan          GPIO 32    GPIO 33    GPIO 25    GPIO 26    GPIO 27    GPIO 14
  (`servoKanan`)                                                          

  Sisi kiri           GPIO 23    GPIO 22    GPIO 21    GPIO 19    GPIO 18    GPIO 17
  (`servoKiri`)                                                           
  ----------------------------------------------------------------------------------

### Perangkat lainnya

  Perangkat               Pin ESP32 Keterangan
  --------------------- ----------- -----------------
  Slide potentiometer       GPIO 34 Input ADC
  NeoPixel Ring             GPIO 13 Data LED
  Passive buzzer            GPIO 15 Output nada/PWM
  OLED SDA                  GPIO 16 Jalur I2C SDA
  OLED SCL                   GPIO 4 Jalur I2C SCL

OLED menggunakan alamat I2C `0x3C`. Resolusi layar yang dikonfigurasi
adalah 128 × 64 piksel.

## Library

Library yang digunakan pada sketch:

  Library               Kegunaan
  --------------------- ----------------------------------
  `ESP32Servo`          Mengendalikan servo pada ESP32
  `Wire`                Komunikasi I2C dengan OLED
  `Adafruit GFX`        Fungsi grafis dasar OLED
  `Adafruit SSD1306`    Mengendalikan layar OLED SSD1306
  `Adafruit NeoPixel`   Mengendalikan LED NeoPixel

Header `math.h` juga digunakan untuk perhitungan fungsi matematika
seperti `sin()`, `cos()`, dan `pow()`.

Di Wokwi, dependensi library eksternal dapat dikelola melalui **Library
Manager** atau file `libraries.txt`. Pastikan library yang diperlukan
tersedia saat menjalankan simulasi.

## Cara Menjalankan Simulasi

1.  Buka [project Tarian Servo 2 di
    Wokwi](https://wokwi.com/projects/480930266021257217).
2.  Tunggu hingga diagram rangkaian dan sketch selesai dimuat.
3.  Klik tombol **Start Simulation**.
4.  Amati inisialisasi OLED, LED, dan posisi awal servo.
5.  Perhatikan sepuluh babak koreografi yang berjalan berurutan.
6.  Geser slide potentiometer untuk mengubah kecepatan gerakan.
7.  Buka **Serial Monitor** dengan baud rate `115200` untuk melihat
    informasi babak dan tempo.

Jika ingin mengembangkan project, ubah kode pada `sketch.ino`, susunan
komponen pada `diagram.json`, atau dependensi pada `libraries.txt`.

## Cara Kerja Program

### 1. Inisialisasi (`setup()`)

Saat ESP32 dinyalakan, program:

-   Memulai Serial Monitor pada 115200 baud.
-   Mengatur pin potentiometer dan buzzer.
-   Menginisialisasi komunikasi I2C OLED pada SDA GPIO 16 dan SCL GPIO
    4.
-   Menginisialisasi NeoPixel Ring.
-   Menghubungkan 12 servo ke pin masing-masing.
-   Mengatur seluruh servo ke posisi awal 90°.

### 2. Pengaturan tempo

Fungsi `bacaKecepatanGlobal()` membaca nilai ADC potentiometer
(0--4095), lalu memetakannya ke rentang 0,25×--3,00×. Nilai ini
digunakan sebagai pengali kecepatan gerak dan durasi jeda.

### 3. Pengendalian servo

-   `setSimetris()` menggerakkan sepasang servo dengan sudut berlawanan.
-   `setBebas()` mengatur sudut servo kanan dan kiri secara terpisah.
-   `kembaliNetral()` mengembalikan servo ke 90° dengan interpolasi
    halus (*smoothstep easing*).

Sudut gerak dibatasi pada rentang 10°--170°.

### 4. Efek LED, audio, dan OLED

-   `updateNeoPixel()` membaca sudut servo dan menggunakannya untuk
    menentukan warna serta kecerahan LED.
-   `updateAudioIrama()` menghasilkan nada berdasarkan ketukan dan pola
    irama.
-   `updateOLED()` memperbarui informasi babak, tempo, dan progress bar.
    Pembaruan layar dibatasi sekitar setiap 120 ms.

### 5. Pengulangan koreografi (`loop()`)

Fungsi `loop()` menjalankan babak 1 sampai 10 secara berurutan. Setelah
setiap babak, servo dikembalikan ke posisi netral sebelum melanjutkan ke
babak berikutnya. Setelah babak penutup selesai, urutan dimulai kembali.

## Urutan Koreografi

  -------------------------------------------------------------------------
                  Babak Nama             Karakter gerakan  Tema warna
  --------------------- ---------------- ----------------- ----------------
                      1 Kepak Kupu-Kupu  Gerakan berombak  Biru--cyan
                                         seperti kepakan   
                                         sayap             

                      2 Gelombang Pita   Gerakan gelombang Magenta--ungu
                        Sutra            dengan kombinasi  
                                         sinus dan cosinus 

                      3 Mekar Bunga      Gerakan membuka   Merah muda--emas
                        Teratai          dan mengembang    
                                         secara bertahap   

                      4 Spiral DNA       Gerakan           Hijau zamrud
                                         berlawanan antara 
                                         sisi kanan dan    
                                         kiri              

                      5 Formasi Kipas    Gerakan           Toska--biru
                        Merak            membuka-menutup   
                                         menyerupai kipas  

                      6 Tari Saman Aceh  Gerakan ritmis    Merah--oranye
                                         bergantian dengan 
                                         tempo cepat       

                      7 Pusaran Vortex   Gelombang         Ungu elektrik
                                         berputar dengan   
                                         amplitudo berubah 

                      8 Pendulum Wave    Gerakan ayunan    Kuning emas
                                         dengan frekuensi  
                                         berbeda           

                      9 Riak Ombak       Gerakan gelombang Biru laut
                        Diagonal         diagonal yang     
                                         tidak serempak    

                     10 Penutup Hormat   Gerakan penutup   Emas hangat
                        Agung            dan transisi      
                                         kembali ke posisi 
                                         netral            
  -------------------------------------------------------------------------

## Pengaturan Kecepatan

Kecepatan dihitung dengan rumus:

``` cpp
0.25 + (nilaiADC / 4095.0) * 2.75
```

    Nilai potentiometer (perkiraan)               Kecepatan
  --------------------------------- -----------------------
                                  0   0,25× (paling lambat)
                               2048                   1,63×
                               4095    3,00× (paling cepat)

Nilai aktual dapat sedikit berbeda bergantung pada pembacaan ADC dan
posisi slider.

## Catatan Pengembangan dan Perangkat Keras

-   Project ini merupakan simulasi berbasis Wokwi. Perilaku pada
    perangkat fisik dapat berbeda karena karakteristik servo, catu daya,
    dan rangkaian.
-   Untuk implementasi nyata, **jangan memasok daya 12 servo langsung
    dari pin 5V ESP32**. Gunakan catu daya eksternal yang memadai sesuai
    spesifikasi servo.
-   Satukan GND catu daya servo dengan GND ESP32 agar sinyal kontrol
    memiliki referensi yang sama.
-   Pastikan regulator, kabel, dan distribusi daya mampu menangani arus
    awal (*stall current*) seluruh servo.
-   Periksa kompatibilitas level tegangan dan koneksi data NeoPixel
    serta buzzer.
-   Sesuaikan mekanisme fisik dan batas sudut servo agar tidak terjadi
    benturan atau beban berlebih.

## Struktur File Wokwi

  File              Keterangan
  ----------------- --------------------------------------------------
  `sketch.ino`      Program utama ESP32 dan algoritma koreografi
  `diagram.json`    Konfigurasi komponen dan koneksi rangkaian Wokwi
  `libraries.txt`   Daftar library tambahan yang dibutuhkan

------------------------------------------------------------------------

**Project:** Tarian Servo 2\
**Platform simulasi:**
[Wokwi](https://wokwi.com/projects/480930266021257217)\
**Board:** ESP32 DevKit V1
