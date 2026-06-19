// =============================================================
//  Lengan Robot - Kontrol Servo via Bluetooth Low Energy (BLE)
//  Tidak menggunakan class — murni prosedural
// =============================================================
#include <ESP32Servo.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// --- PIN Servo ---
#define PIN_MAJU   23
#define PIN_ATAS   22
#define PIN_KIRI   19
#define PIN_CAPIT  18

// --- UUID BLE (harus sama persis dengan HTML) ---
#define SERVICE_UUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// --- Variabel Global ---
Servo servo1, servo2, servo3, servo4;

BLEServer*         bleServer = NULL;
BLECharacteristic* bleChar   = NULL;

bool   sudahTerhubung = false;
String pesanLama      = "";


// =============================================================
//  Fungsi: gerakkan servo berdasarkan pesan yang diterima
//  Format pesan : "1,90"  →  servo 1, sudut 90 derajat
// =============================================================
void prosesPerintah(String pesan) {
  int posKoma = pesan.indexOf(',');
  if (posKoma == -1) {
    return;  // format salah, abaikan
  }

  int id    = pesan.substring(0, posKoma).toInt();
  int sudut = pesan.substring(posKoma + 1).toInt();

  // Batasi sudut agar servo tidak rusak
  sudut = constrain(sudut, 0, 180);

  if (id == 1) {
    servo1.write(sudut);
  }
  else if (id == 2) {
    servo2.write(sudut);
  }
  else if (id == 3) {
    servo3.write(sudut);
  }
  else if (id == 4) {
    servo4.write(sudut);
  }

  Serial.print("Servo ");
  Serial.print(id);
  Serial.print(" -> ");
  Serial.print(sudut);
  Serial.println(" derajat");
}


// =============================================================
//  Fungsi: inisialisasi semua servo ke posisi tengah (90°)
// =============================================================
void setupServo() {
  ESP32PWM::allocateTimer(0);

  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);
  servo3.setPeriodHertz(50);
  servo4.setPeriodHertz(50);

  servo1.attach(PIN_MAJU);
  servo2.attach(PIN_ATAS);
  servo3.attach(PIN_KIRI);
  servo4.attach(PIN_CAPIT);

  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);

  Serial.println("Servo siap.");
}


// =============================================================
//  Fungsi: inisialisasi BLE
// =============================================================
void setupBLE() {
  BLEDevice::init("LenganRobot");
  bleServer = BLEDevice::createServer();
  BLEService* bleService = bleServer->createService(SERVICE_UUID);
  // Buat karakteristik yang bisa menerima tulisan dari HTML
  bleChar = bleService->createCharacteristic(
    CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );

  bleService->start();

  // Mulai iklan BLE agar HP/browser bisa menemukan perangkat ini
  BLEAdvertising* iklan = BLEDevice::getAdvertising();
  iklan->addServiceUUID(SERVICE_UUID);
  iklan->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("BLE aktif  —  nama perangkat: LenganRobot");
}


// =============================================================
//  setup() dipanggil sekali saat ESP32 menyala
// =============================================================
void setup() {
  Serial.begin(115200);
  setupServo();
  setupBLE();
}


// =============================================================
//  loop() dipanggil terus-menerus selama ESP32 hidup
// =============================================================
void loop() {

  // --- 1. Cek apakah ada HP/browser yang sedang terhubung ---
  bool terhubungSekarang = (bleServer->getConnectedCount() > 0);

  if (terhubungSekarang && !sudahTerhubung) {
    Serial.println("Client terhubung!");
    sudahTerhubung = true;
  }

  if (!terhubungSekarang && sudahTerhubung) {
    Serial.println("Client terputus. Mulai iklan lagi...");
    sudahTerhubung = false;
    BLEDevice::startAdvertising();  // supaya bisa ditemukan kembali
  }

  // --- 2. Baca pesan BLE yang masuk ---
  if (terhubungSekarang) {
    String pesan = bleChar->getValue();

    // Proses hanya jika ada pesan baru (beda dari pesan sebelumnya)
    if (pesan.length() > 0 && pesan != pesanLama) {
      pesanLama = pesan;
      prosesPerintah(pesan);
    }
  }

  delay(20);
}
