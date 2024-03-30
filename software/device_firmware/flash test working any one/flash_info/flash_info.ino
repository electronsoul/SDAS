/*
    Retrieve basic core info of Flash SPI
    Add a custom device w25x80
    library Adafruit_SPIFlash and SdFat - AdafruitFork

    by Mischianti Renzo <https://mischianti.org>

    https://mischianti.org/

    SPIFlash connected via SPI standard check wiring on the article

*/

#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include <SPI.h>
//            MOSI  MISO  SCLK   SSEL
SPIClass SPI_1(PA7, PA6, PA5, PC5);


Adafruit_FlashTransport_SPI flashTransport(10, SPI_1); // Set CS and SPI interface
Adafruit_SPIFlash flash(&flashTransport);

void setup()
{
  Serial.setTx(PB6);
  Serial.setRx(PB7);
  Serial.begin(115200);
  delay(1000);   // wait for native usb
  Serial.println(F("start"));

  if (flash.begin()) {
    Serial.println(F("Device finded and supported!"));
  } else {
    Serial.println(F("Problem to discover and configure device, check wiring also!"));
  }
  Serial.println(F("detected or not end"));
  // Set 4Mhz SPI speed
  flashTransport.setClockSpeed(4000000, 4000000); // added to prevent speed problem

  Serial.println();

  Serial.println("Adafruit Serial Flash get basic info: ");
  Serial.print("JEDEC ID (FFFFFF for unknown): "); Serial.println(flash.getJEDECID(), HEX);
  Serial.print("Flash size: "); Serial.println(flash.size());

  Serial.println(); Serial.println();

  uint8_t jedec_ids[4];
  flashTransport.readCommand(SFLASH_CMD_READ_JEDEC_ID, jedec_ids, 4);

  // For simplicity with commonly used device, we only check for continuation
  // code at 2nd byte (e.g Fujitsu FRAM devices)
  if (jedec_ids[1] == 0x7F) {
    // Shift and skip continuation code in 2nd byte
    jedec_ids[1] = jedec_ids[2];
    jedec_ids[2] = jedec_ids[3];
  }

  Serial.println("Retrieve JDEC_ID");

  Serial.print("Manufacturer ID: 0x");
  Serial.println(jedec_ids[0], HEX);

  Serial.print("Memory Type: 0x");
  Serial.println(jedec_ids[1], HEX);

  Serial.print("Capacity: 0x");
  Serial.println(jedec_ids[2], HEX);
  Serial.print("Capacity DEC: ");
  Serial.println(jedec_ids[2], DEC);
}

void loop()
{
  // nothing to do
}
