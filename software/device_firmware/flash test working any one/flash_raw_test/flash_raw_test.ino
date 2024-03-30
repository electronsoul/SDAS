#include <SPI.h>

#define FLASH_CS_PIN PD7 // Chip select pin connected to SPI flash memory

void setup() {
  Serial.setTx(PB6);
  Serial.setRx(PB7);
  Serial.begin(115200);
  pinMode(FLASH_CS_PIN, OUTPUT);
  // Initialize SPI communication
  SPI.setMOSI(PB5);
  SPI.setMISO(PB4);
  SPI.setSCLK(PB3);
  SPI.begin();
}

void loop() {
  // Select SPI flash memory
  digitalWrite(FLASH_CS_PIN, LOW);

  // Send read ID command to SPI flash memory
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(0x90); // Read ID command
  SPI.transfer(0x00); // Address bytes
  SPI.transfer(0x00);
  SPI.transfer(0x00);

  // Read device ID
  byte manufacturerID = SPI.transfer(0x00);
  byte memoryType = SPI.transfer(0x00);
  byte capacityID = SPI.transfer(0x00);

  // Deselect SPI flash memory
  digitalWrite(FLASH_CS_PIN, HIGH);

  // Print device ID
  Serial.print("Manufacturer ID: 0x");
  Serial.println(manufacturerID, HEX);
  Serial.print("Memory Type: 0x");
  Serial.println(memoryType, HEX);
  Serial.print("Capacity ID: 0x");
  Serial.println(capacityID, HEX);

  delay(1000); // Wait before reading again
}
