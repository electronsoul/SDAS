#include <SPI.h>
#include "SdFat.h"
#include <ArduinoJson.h>
#include <string.h>

#define SD_CS_PIN PD6
static SPIClass mySPI2(PB5, PB4, PB3, SD_CS_PIN);
#define SD2_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(18), &SPI)
SdFat SD;

void printDirectory(File dir, int numTabs);
File myFile;
int lineNumber = 0; // Variable to keep track of the line number

// Define the size of the JSON buffer
const size_t bufferSize = JSON_OBJECT_SIZE(50);

// Create a buffer for JSON data
StaticJsonDocument<bufferSize> jsonBuffer;

int writeLineToSDCard(String, String);
String getLineByIndex(String, int);
int parseJsonData(String);
String readLineFromSDCard(String);
String formJsonData(int);

void setup() {
  Serial.setTx(PB6);
  Serial.setRx(PB7);
  Serial.begin(115200);

  delay(1000);
  Serial.print("\nInitializing SD card...");
  SPI.setMOSI(PB5);
  SPI.setMISO(PB4);
  SPI.setSCLK(PB3);
  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!SD.begin(SD2_CONFIG)) {
    //if (!SD.begin(SD_CS_PIN)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  uint32_t cardSize = SD.card()->sectorCount();

  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (SD.card()->type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println(F("SD1"));
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println(F("SD2"));
      break;

    case SD_CARD_TYPE_SDHC:
      if (cardSize < 70000000) {
        Serial.println(F("SDHC"));
      } else {
        Serial.println(F("SDXC"));
      }
      break;

    default:
      Serial.println(F("Unknown"));
  }

  /*
    //  print the type and size of the first FAT-type volume
    uint32_t volumesize;
    Serial.print("Volume type is:    FAT");
    Serial.println(int(SD.vol()->fatType()), DEC);

    Serial.print("Card size:  ");
    Serial.println((float) 0.000512 * cardSize);

    Serial.print("Total bytes: ");
    Serial.println(0.000512 * SD.vol()->clusterCount()*SD.sectorsPerCluster());

    Serial.print("Free bytes: ");
    Serial.println(0.000512 * SD.vol()->freeClusterCount()*SD.sectorsPerCluster());
  */

  /*
    File dir =  SD.open("/");
    printDirectory(dir, 0);

    File myFile = SD.open("test.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (myFile) {
      Serial.print("Writing to test.txt...");
      myFile.println("testing 1, 2, 3.");
      // close the file:
      myFile.close();
      Serial.println("done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }

    // re-open the file for reading:
    myFile = SD.open("test.txt");
    if (myFile) {
      Serial.println("test.txt:");

      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        Serial.write(myFile.read());
      }
      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }

      printDirectory(dir, 0);*/

}

int writeLineToSDCard(String filename, String str) {
  myFile = SD.open(filename, FILE_WRITE);
  if (myFile) {
    myFile.println(str);
    myFile.close();
    return 1;
  } else {
    return -1;
  }
}

String getLineByIndex(String filename = "mapdata.txt", int indexID = 0) {
  String readData = "";
  readData = readLineFromSDCard(filename);
  if (readData != "")
  {
    parseJsonData(readData);
    if (jsonBuffer["index"] == indexID) {
      return readData;
    }
  }
  else {
    return "-1";
  }
}

String readLineFromSDCard(String filename = "mapdata.txt") {
  String readData = "";
  static int filepos = 0;

  myFile = SD.open(filename);
  myFile.seek(filepos);
  if (myFile)
  {
    while (myFile.available())
    {
      char charRead = myFile.read();
      readData = readData + String(charRead);
      if (charRead == '\n')break;
    }
    filepos = myFile.position();
    myFile.close();
    return readData;
  }
  else
  {
    return "-1"; // Error opening file
  }
}

int parseJsonData(String str) {
  String jsonString = str; //Serial.readStringUntil('\n');
  DeserializationError error = deserializeJson(jsonBuffer, jsonString);
  if (!error) {
    return 1;
  } else {
    return -1;
  }
}

String formJsonData(int dataToSerialize[]) {
  
  jsonBuffer["index"] = dataToSerialize[0];
  jsonBuffer["lat"] = dataToSerialize[1];
  jsonBuffer["lon"] = dataToSerialize[2];
  jsonBuffer["type"] = dataToSerialize[3];
  jsonBuffer["speed"] = dataToSerialize[4];
  
  // Serialize JSON data into a string
  String jsonString;
  serializeJson(jsonBuffer, jsonString);
  return jsonString;
}

void loop() {
  String str = "";
  //getJsonData();
  //str = makeJsonData();
  //writeSDCard(str);
  str = getLineByIndex("test.txt", 13);
  Serial.println(str);
  delay(2000);
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    entry.printName(&Serial);

    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      uint16_t pdate;
      uint16_t ptime;
      entry.getModifyDateTime(&pdate, &ptime);

      Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", FS_YEAR(pdate), FS_MONTH(pdate), FS_DAY(pdate), FS_HOUR(ptime), FS_MINUTE(ptime), FS_SECOND(ptime));
    }
    entry.close();
  }
}
