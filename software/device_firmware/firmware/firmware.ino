#include <HardwareSerial.h>
#include <config_sys.h>
#include <EasyLogger.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>

#include <init_sys.h>
#include <ec200_module.h>
#include <ec200_gnss.h>
#include "SDMEM.h"
#include "route.h"
#include <ICM_20948.h>
#include "icm20.h"

#include <ArduinoJson.h>
const size_t bufferSize = JSON_OBJECT_SIZE(100);
StaticJsonDocument<bufferSize> jsonBuffer;

ICM_20948_I2C myICM;

HardwareSerial Serial1(USART1_RX, USART1_TX);
HardwareSerial Serial2(USART2_RX, USART2_TX);
HardwareSerial Serial3(USART3_RX, USART3_TX);

_SYS_MODULES_t _sys_module = SYSTEM_M;
//int routeindex = 0;
int myCompass = 0;
double lat1, lon1, lat2, lon2;
double d1 = 0, d2 = 0;

byte fd_display[10]      = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x02};
byte fd_display_R[10]    = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x03};
byte lt_display[10]      = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x04};
byte lt_display_R[10]    = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x05};
byte rt_display[10]      = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x06};
byte rt_display_R[10]    = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x07};
byte fw_lt_display[10]   = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x08};
byte fw_lt_display_R[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x09};
byte fw_rt_display[10]   = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x0A};
byte fw_rt_display_R[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x0B};
byte round_display[10]   = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x0D};
byte round_display_R[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x0E};
byte cross_display[10]   = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x0F};
byte cross_display_R[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x10};
byte warning_display[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x11};
byte buz[] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0xA0, 0x00, 0x7D};

double toRadians(double degrees) {
  return (degrees * PI ) / 180.0;
}

bool isInRange(double value, double rangeStart, double rangeEnd) {
  // Ensure that rangeStart is smaller than rangeEnd
  if (rangeStart > rangeEnd)
    std::swap(rangeStart, rangeEnd);

  return (value >= rangeStart && value <= rangeEnd);
}

String getDirection(int d) { //current , previous
  String dir = "Forward";

  if (d == 2) {
    dir = "Right";
  } else if (d == 3) {
    dir = "Left";
  }

  /*if ((d1 - d2) > 85) {
    dir = "Right";
    } else if ((d1 - d2)  <  -85) {
    dir = "Left";
    }*/

  return dir;
}

double calculateInitialCompassBearing(double lat1, double lon1, double lat2, double lon2) {
  // Convert decimal degrees to radians
  lat1 = toRadians(lat1);
  lon1 = toRadians(lon1);
  lat2 = toRadians(lat2);
  lon2 = toRadians(lon2);

  // Compute difference in longitudes
  double diff_lon = lon2 - lon1;

  // Compute the bearing using trigonometry
  double y = sin(diff_lon) * cos(lat2);
  double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(diff_lon);
  double bearing = atan2(y, x);

  // Convert bearing from radians to degrees
  bearing = bearing * 180.0 / PI;

  // Normalize to compass bearing (0 to 360 degrees)
  double compass_bearing = fmod((bearing + 360), 360);

  return compass_bearing;
}

void setDisplay(int i) {
  if (i == 1) {
    DisplayUart.write(fd_display, sizeof(fd_display));
  }
  else if (i == 2) {
    DisplayUart.write(fd_display_R, sizeof(fd_display_R));
  }
  else if (i == 3) {
    DisplayUart.write(lt_display, sizeof(lt_display));
  }
  else if (i == 4) {
    DisplayUart.write(lt_display_R, sizeof(lt_display_R));
  }
  else if (i == 5) {
    DisplayUart.write(rt_display, sizeof(rt_display));
  }
  else if (i == 6) {
    DisplayUart.write(rt_display_R, sizeof(rt_display_R));
  }
  else if (i == 7) {
    DisplayUart.write(fw_lt_display, sizeof(fw_lt_display));
  }
  else if (i == 8) {
    DisplayUart.write(fw_lt_display_R, sizeof(fw_lt_display_R));
  }
  else if (i == 9) {
    DisplayUart.write(fw_rt_display, sizeof(fw_rt_display));
  }
  else if (i == 10) {
    DisplayUart.write(fw_rt_display_R, sizeof(fw_rt_display_R));
  }
  else if (i == 12) {
    DisplayUart.write(round_display, sizeof(round_display));
  }
  else if (i == 13) {
    DisplayUart.write(round_display_R, sizeof(round_display_R));
  }
  else if (i == 14) {
    DisplayUart.write(cross_display, sizeof(cross_display));
  }
  else if (i == 15) {
    DisplayUart.write(cross_display_R, sizeof(cross_display_R));
  }
  else if (i == 16) {
    DisplayUart.write(warning_display, sizeof(warning_display));
  }

  //delay(10);
}

byte integerToAsciiByte(int number) {
  char buffer[10]; // Buffer to hold the ASCII representation of the number
  itoa(number, buffer, 10); // Convert integer to ASCII string

  // Assuming the integer fits into a byte, we return the first character of the ASCII string
  return (byte)buffer[0];
}

void extractDigits(int number, int &digit1, int &digit2, int &digit3) {
  digit1 = number / 100;         // Extract hundreds digit
  digit2 = (number / 10) % 10;    // Extract tens digit
  digit3 = number % 10;           // Extract ones digit
}

int displayText(byte spd, int i) {
  byte displaytext[] = {0x5A, 0xA5, 0x08, 0x82, 0x10, 0x20, 0x30, 0x30, 0x30, 0xFF, 0xFF};
  int digit1, digit2, digit3;
  extractDigits(spd, digit1, digit2, digit3);

  digit1 = integerToAsciiByte(digit1);
  digit2 = integerToAsciiByte(digit2);
  digit3 = integerToAsciiByte(digit3);

  if (i == 1) {
    displaytext[4] = 0x10; //d1
    displaytext[5] = 0x20; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 2) {
    displaytext[4] = 0x10; //d1
    displaytext[5] = 0x40; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 3) {
    displaytext[4] = 0x10; //d1
    displaytext[5] = 0x60; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 4) {
    displaytext[4] = 0x10; //d1
    displaytext[5] = 0x80; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 5) {
    displaytext[4] = 0x11; //d1
    displaytext[5] = 0x00; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 6) {
    displaytext[4] = 0x11; //d1
    displaytext[5] = 0x20; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 7) {
    displaytext[4] = 0x11; //d1
    displaytext[5] = 0x40; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 8) {
    displaytext[4] = 0x10; //d1
    displaytext[5] = 0x60; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 9) {
    displaytext[4] = 0x11; //d1
    displaytext[5] = 0x80; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 10) {
    displaytext[4] = 0x12; //d1
    displaytext[5] = 0x00; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 12) {
    displaytext[4] = 0x12; //d1
    displaytext[5] = 0x40; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 13) {
    displaytext[4] = 0x12; //d1
    displaytext[5] = 0x60; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 14) {
    displaytext[4] = 0x12; //d1
    displaytext[5] = 0x80; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 15) {
    displaytext[4] = 0x13; //d1
    displaytext[5] = 0x00; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 16) {
    displaytext[4] = 0x13; //d1
    displaytext[5] = 0x20; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  DisplayUart.write(displaytext, sizeof(displaytext));
  //delay(50);
  return 1;
}

int flag = 0;
String getMessageModem() {
  String str = "";
  //DebugUart.printf("Size %d N", ModemUart.available());
  while (ModemUart.available()) {
    char ch = ModemUart.read();
    str = str + String(ch);
  }
  //DebugUart.printf("Size %d N", ModemUart.available());
  //str = str + "\0";
  return str;
}

String getMessageDisplay() {
  String str = "";
  while (DisplayUart.available()) {
    int ch = DisplayUart.read();
    str = str + String(ch);
    delay(50);
  }
  return str;
}

String getMessageDebug() {
  String str = "";
  while (DebugUart.available()) {
    char ch = DebugUart.read();
    str = str + String(ch);
    //delay(50);
  }
  return str;
}

void setup() {
  int flagStatus = 1;
  SystemClock_Config();

  DebugUart.begin(115200);
  while (!DebugUart);
  LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "SYSTEM BOOTING");
  LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "FIRMWARE REVISION: " << _SOFTWARE_REV_);

  _sys_module = DISPLAY_M;
  DisplayUart.begin(115200);
  while (!DisplayUart);
  LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "DISPLAY BOOTING");
  //powerUpModem();
  //delay(2000);
  //powerDownModem();
  pinMode(PD12, OUTPUT); //GREENLED
  pinMode(PD15, OUTPUT); //BLUELED
  pinMode(PD14, OUTPUT); //REDLED
  pinMode(PA0, INPUT); //push button , push to high
  digitalWrite(PD12, LOW);
  //pinMode(MODEMPOWEREY, OUTPUT);
  //pinMode(MODEMRESET, OUTPUT);
  //digitalWrite(MODEMPOWEREY, LOW);
  //digitalWrite(MODEMRESET, LOW);

  _sys_module = EC200_M;
  LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "EC200 BOOTING");

  //powerDownModem();
  //powerUpModem();

  flagStatus &= initModem();
  flagStatus &= initModemGNSS();
  initICM20();

  _sys_module = SDMEM_M;
  if (initSDCard()) {
    LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "SD CARD DETECTED");
    printSDCardSpecifications();
  }

  int tmp = millis();
  while ((millis() - tmp) < 10000) {
    DebugUart.println("Qcectel ColdBoot Calibration in Progress...");
    digitalWrite(PD15, HIGH);
    digitalWrite(PD14, LOW);
    delay(250);
    digitalWrite(PD15, LOW);
    digitalWrite(PD14, HIGH);
    delay(250);
  }
  if (flagStatus == 1) {
    DebugUart.println("System Initialization Successful");
  }
  else if ( flagStatus == 0) {
    DebugUart.println("System Initialization Failed");
    DebugUart.println("System Resetting...");
    //NVIC_SystemReset();
  }
  //deserializeJson(jsonBuffer, String(routedata_sec85[0]));
  //lat1 = jsonBuffer["lat"].as<double>();
  //lon1 = jsonBuffer["lon"].as<double>();

  int state = 1;
  String str = "";

  while (state && !(digitalRead(PA0)))//; //add dispaly switch input
  {
    str = getMessageDisplay();
    if (str != "") {
      str = str.substring(15);
      if (str == "1") {
        state = 0;
      }
      //DebugUart.println(str);
    }
  }
  setDisplay(16);
  displayText(0, 16);

}

String str = "", latz = "" , lonz = "" , spdz = "", mapstr = "";
float mycompass = 0, compprev = 0;
int disSelect = 16;
int file_dir = 1, file_spd;
int myspd = 0;
int routeindex = 0;
double mylat, mylon, file_lat1, file_lon1, file_lat2, file_lon2;

void loop() {
  _sys_module = EC200_M;
  digitalWrite(PD12, HIGH);
  str = getLocationCoords();

  if (str.startsWith("AT+QGPSLOC"))
  {
    //DebugUart.print(str); DebugUart.println('\t');
    latz = str.substring(36);
    latz = latz.substring(0, 8);
    mylat = latz.toDouble();
    DebugUart.print(mylat, 6); DebugUart.print('\t');
    lonz = str.substring(45);
    lonz = lonz.substring(0, 8);
    mylon = lonz.toDouble();
    DebugUart.print(mylon, 6); DebugUart.print('\t');
    spdz = str.substring(73);
    spdz = spdz.substring(0, 3);
    myspd = spdz.toInt();
    //DebugUart.print(myspd); DebugUart.print(" \t ");

    if (routeindex < route_size) {
      deserializeJson(jsonBuffer, String(routedata_sec85[routeindex]));
      file_lat1 = jsonBuffer["lat1"].as<double>();
      file_lon1 = jsonBuffer["lon1"].as<double>();
      file_lat2 = jsonBuffer["lat2"].as<double>();
      file_lon2 = jsonBuffer["lon2"].as<double>();
      file_spd = jsonBuffer["speed"].as<int>();
      file_dir = jsonBuffer["dir"].as<int>();

      String tmp = "";
      tmp = getMessageDebug();
      if (tmp != "") {
        mylat = tmp.toDouble();
        DebugUart.print(mylat); DebugUart.print('\t');
      }

      if (isInRange(mylat, file_lat1, file_lat2) || isInRange(mylon, file_lon1, file_lon2))
      {
        DisplayUart.write(buz, sizeof(buz));
        disSelect = file_dir;
        routeindex++;
        if (myspd > file_spd) {
          disSelect++;
        }
      }
      setDisplay(disSelect);
      displayText(myspd, disSelect);

    }
    else {
      setDisplay(16);
      displayText(myspd, 16);
    }
    DebugUart.print(routeindex); DebugUart.print('\t');
    DebugUart.println();
    str = "";
  } else {
    //ModemUart.println("AT+QGPSCFG=\"autogps\",1");
    //if (str != "")
    DebugUart.println("No Data From Qcectel");
  }
  DebugUart.flush();
  ModemUart.flush();
  delay(250); //prev it was taking 4-5 samples at 100ms, repaeating samples
}
