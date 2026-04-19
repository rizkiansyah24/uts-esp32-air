# Smart Water Tank Dashboard berbasis ESP32 + MQTT + HTTP

## Profil pembuat
* Nama         : Aldi Rizkiansyah
* NIM          : 23552011130
* Jurusan      : Teknik Informatika
* Semester     : 6
* Mata Kuliah  : Sistem Mikrokontroller
* Kampus       : Universitas Teknologi Bandung

## Deskripsi Proyek

Proyek ini merupakan sistem monitoring ketinggian air berbasis mikrokontroler ESP32 R32 yang dapat:

* Membaca jarak permukaan air menggunakan sensor HC-SR04
* Menentukan status ketinggian air
* Menyalakan LED indikator sesuai level air
* Mengirim data secara realtime menggunakan MQTT melalui Shiftr.io
* Menampilkan data pada dashboard web
* Menyimpan histori monitoring ke MockAPI menggunakan HTTP POST
* Mengontrol LED secara manual melalui website

Dashboard dibuat menggunakan HTML, CSS, dan JavaScript di VS Code, sedangkan program ESP32 dibuat menggunakan Arduino IDE.

---

# Arsitektur Sistem

```text
HC-SR04 --> ESP32 --> MQTT (Shiftr.io) --> Dashboard Web
                 --> HTTP POST --> MockAPI --> Histori Dashboard
```

Penjelasan:

1. Sensor HC-SR04 membaca jarak permukaan air
2. ESP32 memproses jarak tersebut menjadi status air
3. ESP32 mengirim data ke:

   * Shiftr.io melalui MQTT untuk realtime monitoring
   * MockAPI melalui HTTP POST untuk penyimpanan histori
4. Dashboard web menerima data MQTT dan menampilkan:

   * Jarak air
   * Status air
   * Status LED
   * Tombol kontrol LED
5. Dashboard juga mengambil histori data dari MockAPI

---

# Tools dan Software yang Digunakan

## Hardware

* ESP32 R32
* Breadboard
* Sensor HC-SR04
* 4 LED
* 4 resistor 220 ohm
* Kabel jumper
* Kabel USB ESP32

## Software

* Arduino IDE
* VS Code
* Laragon
* Shiftr.io
* MockAPI
* Browser

---

# Fungsi Masing-Masing Komponen

## ESP32 R32

ESP32 digunakan sebagai pusat pengendali sistem.

Fungsi:

* Membaca data sensor HC-SR04
* Menyalakan LED
* Mengirim data MQTT
* Mengirim data HTTP
* Menerima perintah dari dashboard

## HC-SR04

Sensor ultrasonik HC-SR04 digunakan untuk membaca jarak antara sensor dengan permukaan air.

Semakin kecil jarak, berarti air semakin penuh.

## LED

LED digunakan sebagai indikator level air:

| Kondisi | LED Menyala           |
| ------- | --------------------- |
| Kosong  | Tidak ada             |
| Rendah  | LED 1                 |
| Sedang  | LED 1 + LED 2         |
| Tinggi  | LED 1 + LED 2 + LED 3 |
| Penuh   | Semua LED             |

---

# Mapping Pin ESP32

| Komponen     | Pin ESP32 |
| ------------ | --------- |
| LED 1        | GPIO 18   |
| LED 2        | GPIO 19   |
| LED 3        | GPIO 23   |
| LED 4        | GPIO 5    |
| HC-SR04 Trig | GPIO 13   |
| HC-SR04 Echo | GPIO 12   |
| HC-SR04 VCC  | 5V / VIN  |
| HC-SR04 GND  | GND       |

---

# Rangkaian Breadboard

## Ground

Semua ground disatukan ke jalur negatif breadboard:

```text
ESP32 GND -> Rail (-) Breadboard
HC-SR04 GND -> Rail (-)
Semua kaki negatif LED -> resistor -> Rail (-)
```

## Positif LED

```text
GPIO18 -> LED1 -> resistor -> GND
GPIO19 -> LED2 -> resistor -> GND
GPIO23 -> LED3 -> resistor -> GND
GPIO5  -> LED4 -> resistor -> GND
```

## Sensor HC-SR04

```text
VCC  -> 5V / VIN ESP32
GND  -> GND
TRIG -> GPIO13
ECHO -> GPIO12
```

---

# Cara Membuat Project Web

## 1. Membuat Folder Project di Laragon

Buka Terminal Laragon:

```bash
cd C:\laragon\www
mkdir smart-water-dashboard
cd smart-water-dashboard
code .
```

---

## 2. Struktur Folder

```text
smart-water-dashboard/
│
├── index.html
├── script.js
└── README.md
```

---

# Konfigurasi Shiftr.io

## Membuat Broker MQTT

1. Login ke Shiftr.io
2. Buat namespace baru
3. Catat:

   * Host
   * Username
   * Password

Contoh:

```text
Host      : uts-esp32-air.cloud.shiftr.io
Username  : uts-esp32-air
Password  : uts-esp32-air
```

Koneksi MQTT web menggunakan:

```javascript
mqtt.connect("wss://uts-esp32-air.cloud.shiftr.io:443", {
    username: "uts-esp32-air",
    password: "uts-esp32-air"
});
```

---

# Topic MQTT yang Digunakan

| Topic            | Fungsi                |
| ---------------- | --------------------- |
| uts/water/level  | Mengirim jarak air    |
| uts/water/status | Mengirim status air   |
| uts/led/status   | Mengirim status LED   |
| uts/led/control  | Menerima perintah LED |

---

# Logika Status Air

Program menentukan status air berdasarkan jarak:

```text
> 30 cm      = KOSONG
20 - 30 cm   = RENDAH
10 - 20 cm   = SEDANG
5 - 10 cm    = TINGGI
<= 5 cm      = PENUH
```

---

# Cara Kerja Program ESP32

## 1. ESP32 Membaca Sensor

ESP32 membaca HC-SR04 menggunakan trigger dan echo.

```cpp
float distance = readDistance();
```

## 2. Menentukan Status Air

Berdasarkan nilai distance, ESP32 menentukan:

* KOSONG
* RENDAH
* SEDANG
* TINGGI
* PENUH

## 3. Menyalakan LED

Setiap status menyalakan jumlah LED yang berbeda.

## 4. Mengirim MQTT

```cpp
client.publish(topicWaterLevel, jarak.c_str());
client.publish(topicWaterStatus, statusAir.c_str());
```

## 5. Mengirim HTTP POST ke MockAPI

ESP32 mengirim data JSON seperti berikut:

```json
{
  "distance": 12.45,
  "status": "SEDANG",
  "led1": true,
  "led2": true,
  "led3": false,
  "led4": false
}
```

---

# Konfigurasi MockAPI

Buat endpoint:

```text
https://xxxxxxxx.mockapi.io/api/v1/water-monitor
```

Data dikirim dengan metode POST.

---

# Tampilan Dashboard

Dashboard memiliki 5 bagian:

1. Status koneksi MQTT
2. Jarak air realtime
3. Status air realtime
4. Kontrol LED manual
5. Histori monitoring

---

# Cara Kerja Dashboard

## Realtime MQTT

Dashboard subscribe ke:

```javascript
client.subscribe(topicWaterLevel);
client.subscribe(topicWaterStatus);
client.subscribe(topicLedStatus);
```

Saat data diterima:

* Jarak diperbarui
* Status air diperbarui
* Warna indikator berubah
* Switch LED ikut berubah

---

## Histori Monitoring

Dashboard mengambil histori dari MockAPI:

```javascript
fetch(apiUrl)
```

Kemudian ditampilkan seperti terminal log.

---

# Fitur yang Sudah Berhasil

* Realtime monitoring sensor air
* Kontrol LED dari website
* Sinkronisasi LED antara ESP32 dan dashboard
* Penyimpanan histori ke MockAPI
* Histori tampil di dashboard
* MQTT connect/disconnect indicator
* Log aktivitas MQTT

---

# Kemungkinan Pengembangan

Beberapa fitur tambahan agar project lebih bagus:

* Menambahkan buzzer saat status PENUH
* Menambahkan grafik histori
* Menambahkan mode otomatis dan manual
* Menambahkan login dashboard
* Menambahkan notifikasi Telegram
* Menambahkan database asli seperti MySQL atau Firebase
* Menambahkan sensor lain seperti suhu atau kelembaban

---

# Cara Menjalankan Project

## ESP32

1. Buka Arduino IDE
2. Install board ESP32
3. Install library:

   * WiFi.h
   * PubSubClient
   * HTTPClient
4. Upload program ke ESP32
5. Buka Serial Monitor

## Website

1. Jalankan Laragon
2. Buka folder project di browser
3. Pastikan internet aktif
4. Dashboard akan otomatis connect ke Shiftr.io

---

# Screenshot yang Disarankan untuk README

Tambahkan screenshot berikut:

* Tampilan dashboard
* Serial Monitor Arduino IDE
* Data di MockAPI
* Foto breadboard dan rangkaian
* Tampilan saat air penuh

Contoh:

```markdown
![Dashboard](images/dashboard.png)
![Serial Monitor](images/serial-monitor.png)
![MockAPI](images/mockapi.png)
![Rangkaian](images/rangkaian.jpg)
```

---

# Kesimpulan

Project ini berhasil menggabungkan:

* ESP32
* Sensor ultrasonik
* MQTT
* HTTP API
* Dashboard web

Sehingga tercipta sistem monitoring air yang realtime, dapat dikontrol dari web, dan memiliki histori data.

Project ini cocok digunakan sebagai tugas UTS Sistem Mikrokontroler karena mencakup:

* Sensor
* Aktuator
* Internet of Things
* Web Dashboard
* API
* MQTT
* Penyimpanan Data
