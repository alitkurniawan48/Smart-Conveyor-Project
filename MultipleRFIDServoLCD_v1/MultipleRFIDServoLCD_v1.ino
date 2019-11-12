#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>

#define LED_PIN1 A0
#define LED_PIN2 A1
#define LED_PIN3 A2
#define BUZZER_PIN A3

LiquidCrystal_I2C lcd(0x3F, 16, 2);

Servo servo1;
Servo servo2;

String txData = "";

EthernetClient client;  //ETHERNET INSTANCE
EthernetClient client2;  //ETHERNET INSTANCE

constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
constexpr uint8_t SS_1_PIN = 4;   // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 2
constexpr uint8_t SS_2_PIN = 8;    // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 1

constexpr uint8_t NR_OF_READERS = 2;

byte ssPins[] = {SS_1_PIN, SS_2_PIN};

MFRC522 mfrc522[NR_OF_READERS];   // Create MFRC522 instance.

/**
   Initialize.
*/
void setup() {
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  // ETHERNET MODULE INITIAL
  SPI.begin();        // Init SPI bus

  // SERVO
  servo1.attach(5);
  servo2.attach(6);

  // LED (GREEN, ORANGE, BLUE)
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);

  // BUZZER
  pinMode(BUZZER_PIN, OUTPUT);

  byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
  };     //MAC = 000102030405

  // ETHERNET IP
  byte ip[] = {
    192, 168, 8, 98
  };
  
  //IPAddress ip(192,168,8,98);
  Ethernet.begin(mac, ip);      //CONNECT USING ABOVE
  Serial.println(Ethernet.localIP());
  Serial.println("SUCCESSFUL CONNECTION!");

  // LCD MODULE INITIAL
  lcd.begin();
  lcd.begin();

  // MUNCULKAN DI LCD
  lcd.backlight();
  lcd.setCursor(4, 0); // BARIS PERTAMA
  lcd.print("WELCOME");
  lcd.setCursor(3, 1); // BARIS KEDUA
  lcd.print("PROFIT-WMS");

  // POSISI DEFAULT SERVO
  servo1.write(0);
  servo2.write(0);

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
}

/**
   Main loop.
*/
void loop() {

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    // Look for new cards

    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F(": Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));

      // DEKLARASI RFID KE VARIABEL
      String strID = "";
      for (byte i = 0; i < 4; i++) {
        strID +=
          (mfrc522[reader].uid.uidByte[i] < 0x10 ? "0" : "") +
          String(mfrc522[reader].uid.uidByte[i], HEX) +
          (i != 3 ? ":" : "");
      }
      strID.toUpperCase();
      strID.replace(":", "");

      // MENGKONEKSIKAN KE JARINGAN SERVER DAN MENGIRIM DATA
      //CEK KONEKSI SERVER
      if (client.connect("192.168.8.99", 80) && client2.connect("192.168.8.99", 80)) {
        // MEMANGGIL DATA RFID
        const String ID = strID;
        txData = "barang_id=" + (ID) ;

        // MENGIRIM DATA
        Serial.println("Connected");
        Serial.print(txData);
        
        client.println("GET /app_wms/inboundc/fetch_data?kode=" + (ID) + "");
        client2.println("GET /app_wms/inboundc/get_jenis?kode=" + (ID));

//        client.println("GET /tugas_akhir/index.php?mod=barang_masuk&submod=barang_masuk_check_arduino&barang_id=" + (ID) + "");
//        client2.println("GET /tugas_akhir/index.php?mod=barang_masuk&submod=barang_masuk_check_arduino_jenis&barang_id=" + (ID));        

        // DELAY 1 DETIK
        delay(500);

        if (client.available() && client2.available())
        {
          Serial.println("Avaliable");
          int c = client.read();
          Serial.println(c);
          int c2 = client2.read();
          Serial.println(c2);
          
            if (c < 49)
            {
              LEDOrange();
              
              LCDTidakTerdeteksi();

              delay(2000);

              LCDReset();
            } else {
              LEDBlue();

              // MENGGERAKKAN SERVO
              if (c2 < 49) {
                LCDPecahBelah();

                ServoSatuGerak();
              } else {
                LCDBukanPecahBelah();

//                ServoDuaGerak();
              }
            }
          } else {
            Serial.println("Not Available");
            LEDBlue();
          }

        // MEMATIKAN KONEKSI
        client.stop();
        client2.stop();
      } else {
        Serial.println("Connection Failed.");

        LEDOrange();
      }

      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader
}

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void ServoSatuGerak() {
  servo1.write(0);
  delay(15000);
  LCDReset();
  servo1.write(90);
}

//void ServoDuaGerak() {
//  servo2.write(0);
//  delay(12000);
//  LCDReset();
//  servo2.write(135);
//}

void LCDReset() {
  lcd.clear();
  lcd.setCursor(4, 0); //baris pertama
  lcd.print("WELCOME");
  lcd.setCursor(3, 1); //baris pertama
  lcd.print("PROFIT-WMS");
}

void LCDPecahBelah() {
  lcd.clear();
  lcd.setCursor(0, 0); //baris pertama
  lcd.print("JENIS BARANG :");
  lcd.setCursor(0, 1); //baris pertama
  lcd.print("PECAH BELAH");
}

void LCDBukanPecahBelah() {
  lcd.clear();
  lcd.setCursor(0, 0); //baris pertama
  lcd.print("JENIS BARANG :");
  lcd.setCursor(0, 1); //baris pertama
  lcd.print("BUKAN PECAH BELAH");
}

void LCDTidakTerdeteksi() {
  lcd.clear();
  lcd.setCursor(0, 0); //baris pertama
  lcd.print("JENIS BARANG :");
  lcd.setCursor(0, 1); //baris pertama
  lcd.print("TIDAK TERDETEKSI");
}

void LEDBlue() {
  // MENGHIDUPKAN LAMPU BIRU DAN BUZZER
  digitalWrite(LED_PIN1, HIGH);
  digitalWrite(LED_PIN2, LOW);
  tone(BUZZER_PIN, 1500);

  // DELAY 0.5 DETIK
  delay(300);

  // MEMATIKAN LAMPU BIRU DAN BUZZER
  digitalWrite(LED_PIN1, LOW);
  noTone(BUZZER_PIN);
}

void LEDOrange() {
  // MENGHIDUPKAN LAMPU ORANGE DAN BUZZER
  digitalWrite(LED_PIN2, HIGH);
  digitalWrite(LED_PIN1, LOW);
  tone(BUZZER_PIN, 1000);

  // DELAY 1 DETIK
  delay(300);

  // MEMATIKAN LAMPU ORANGE DAN BUZZER
  digitalWrite(LED_PIN2, LOW);
  noTone(BUZZER_PIN);
}

void LEDGreen() {
  // MENGHIDUPKAN LAMPU HIJAU DAN BUZZER
  digitalWrite(LED_PIN3, HIGH);
  tone(BUZZER_PIN, 1500);

  // DELAY 1 DETIK
  delay(300);

  // MEMATIKAN LAMPU HIJAU DAN BUZZER
  digitalWrite(LED_PIN3, LOW);
  noTone(BUZZER_PIN);
}
