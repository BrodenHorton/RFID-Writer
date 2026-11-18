#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 7

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
byte targetBlock = 1;

byte insertionData[16];

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // Prepare the default key (all 0xFF)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  printCommandMenuPrompt();
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {
      updateInsertionData(input);
    }
  }

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("Card detected."));
  Serial.println();

  if(!isAuthenticated()) {
    return;
  }

  // Write data to the block
  MFRC522::StatusCode status = (MFRC522::StatusCode) rfid.MIFARE_Write(targetBlock, insertionData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }
  else {
    Serial.println(F("Data written successfully!"));
  }

  printBlockAsString();

  // Halt PICC and stop encryption on PCD
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  delay(3000);
}

void printCommandMenuPrompt() {
  Serial.println("========================================");
  Serial.println("RFID Data Block Writer");
  Serial.println("========================================");
  Serial.println();
  Serial.println("Enter the data you want inserted into the RFID card.");
  Serial.println();
}

void updateInsertionData(String input) {
  if(input.length() > 16) {
    Serial.println("Invalid string length. The inserted string must be 16 characters or less.");
    Serial.println();
    return;
  }

  // Clear the insertionData buffer before copying the input string into it
  size_t insertionDataLength = sizeof(insertionData);
  for(byte i = 0; i < insertionDataLength; i++) {
    insertionData[i] = 0;
  }

  memcpy(insertionData, input.c_str(), input.length());
  Serial.print("Insertion data changed to: ");
  Serial.println(input);
  Serial.println();
}

bool isAuthenticated() {
  MFRC522::StatusCode status = (MFRC522::StatusCode) rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, targetBlock, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }

  return true;
}

void printBlockAsString() {
  byte buffer[18];
  byte size = sizeof(buffer);
  MFRC522::StatusCode status = (MFRC522::StatusCode) rfid.MIFARE_Read(targetBlock, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
  } else {
    Serial.print(F("Data in block "));
    Serial.print(targetBlock);
    Serial.print(F(": "));
    for (byte i = 0; i < 16; i++) {
      Serial.write(buffer[i]);
    }
    Serial.println();
  }
  Serial.println();
}





