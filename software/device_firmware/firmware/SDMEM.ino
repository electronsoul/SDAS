#include "SDMEM.h"
#include <SPI.h>
#include "SdFat.h"
#include <ArduinoJson.h>
#include <string.h>
#include "sdios.h"

#define SD_SPI_BUS_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(18), &SD_SPI_BUS)

SdFat SD;
File myFile;
const size_t bufSize = JSON_OBJECT_SIZE(50);
StaticJsonDocument<bufSize> jsonBuf;

//To detect sd card presence
int initSDCard(void) {

  SPI.setMOSI(SPI_MOSI);
  SPI.setMISO(SPI_MISO);
  SPI.setSCLK(SPI_SCLK);

  if (!SD.begin(SD_SPI_BUS_CONFIG)) {
    return 0;
  } else {
    return 1;
  }
}

//To log sd card information
void printSDCardSpecifications(void) {
  _sys_module = SDMEM_M;
  uint32_t cardSize = SD.card()->sectorCount();
  uint32_t volumesize;

  switch (SD.card()->type())
  {
    case SD_CARD_TYPE_SD1:
      LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "CARD TYPE: " << "SD1");
      break;
    case SD_CARD_TYPE_SD2:
      LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "CARD TYPE: " << "SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      if (cardSize < 70000000) {
        LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "CARD TYPE: " << "SDHC");
      } else {
        LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "CARD TYPE: " << "SDXC");
      }
      break;
    default:
      LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "CARD TYPE: " << "UNKNOWN");
  }

  LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "TYPE OF VOLUME: FAT" << int(SD.vol()->fatType()));
  //LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "SIZE (KB):  " << ((float) 0.000512 * cardSize));
  LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "TOTAL SIZE(KB): " << (0.000512 * SD.vol()->clusterCount()*SD.sectorsPerCluster()));
  //LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "FREE SIZE(KB): " << (0.000512 * SD.vol()->freeClusterCount()*SD.sectorsPerCluster()));

}

//
int appendLineToSDCard(String filename, String str) {
  myFile = SD.open(filename, FILE_WRITE);
  if (myFile) {
    myFile.seek(myFile.size());
    myFile.println(str);
    myFile.close();
    return 1;
  } else {
    return -1;
  }
}

//
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

//s
String getLineByIndex(String filename = "mapdata.txt", int indexID = 0) {
  String readData = "";
  while (1) { //Dangerous , can go infinite loop!!!
    readData = readLineFromSDCard(filename);
    if (readData != "")
    {
      parseJsonData(readData);
      if (jsonBuffer["index"] == indexID) {
        return readData;
      }
    }
  }
  //else {
  return "-1";
  //}
}

//
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

//Extract JSON Data From Line
int parseJsonData(String str) {
  String jsonString = str; //Serial.readStringUntil('\n');
  DeserializationError error = deserializeJson(jsonBuf, jsonString);
  if (!error) {
    return 1;
  } else {
    return -1;
  }
}

//To Form A JSON String
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

/*
  //To Print Directory On SD Card
  void printSDCardDirectory(File dir, int numTabs) {
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
  }*/
