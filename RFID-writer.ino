#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 7

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

byte insertData[16] = {"ocean_man"};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // Prepare the default key (all 0xFF)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("Scan a MIFARE 1K card to write and read data..."));
}

void loop() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("Card detected."));

  // Select a block to write to (Block 4 is safe; avoid Sector Trailers like block 3, 7, etc.)
  byte blockAddr = 1;
  MFRC522::StatusCode status;

  // Authenticate using Key A
  status = (MFRC522::StatusCode) rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }

  // Write data to the block
  status = (MFRC522::StatusCode) rfid.MIFARE_Write(blockAddr, insertData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  } else {
    Serial.println(F("Data written successfully!"));
  }

  // Read data back from the block
  byte buffer[18];
  byte size = sizeof(buffer);
  status = (MFRC522::StatusCode) rfid.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
  } else {
    Serial.print(F("Data in block "));
    Serial.print(blockAddr);
    Serial.print(F(": "));
    for (byte i = 0; i < 16; i++) {
      Serial.write(buffer[i]);
    }
    Serial.println();
  }

  // Halt PICC and stop encryption on PCD
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  delay(3000);
}

