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

#define OOR_DISPLAY 16
#define SPEED_LIMIT 40

byte fd_display[]      = {0x5A, 0xA5, 0x05, 0x82, 0x10, 0x12, 0x00, 0x00};
byte rt_display[]      = {0x5A, 0xA5, 0x05, 0x82, 0x10, 0x12, 0x00, 0x01};
byte lt_display[]      = {0x5A, 0xA5, 0x05, 0x82, 0x10, 0x12, 0x00, 0x02};
byte fw_rt_display[]   = {0x5A, 0xA5, 0x05, 0x82, 0x10, 0x12, 0x00, 0x03};
byte fw_lt_display[]   = {0x5A, 0xA5, 0x05, 0x82, 0x10, 0x12, 0x00, 0x04};
byte cross_display[]   = {0x5A, 0xA5, 0x05, 0x82, 0x10, 0x12, 0x00, 0x05};
byte round_display[]   = {0x5A, 0xA5, 0x05, 0x82, 0x10, 0x12, 0x00, 0x06};
byte hos_display[]     = {0x5A, 0xA5, 0x05, 0x82, 0x10, 0x12, 0x00, 0x07};
byte sch_display[]     = {0x5A, 0xA5, 0x05, 0x82, 0x10, 0x12, 0x00, 0x08};

byte ul_display[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x02};
byte ol_display[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x03};

byte out_of_range_display[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x04};
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

void setDisplay(int i, int overlimit = 0) {

  if (i == OOR_DISPLAY) {
    DisplayUart.write(out_of_range_display, sizeof(out_of_range_display));
    return;
  }
  if (overlimit == 1) {
    DisplayUart.write(ol_display, sizeof(ol_display));
    if (i == 0) {
      fd_display[5] = 0x14;
      DisplayUart.write(fd_display, sizeof(fd_display));
    }
    if (i == 1) {
      rt_display[5] = 0x14;
      DisplayUart.write(rt_display, sizeof(rt_display));
    }
    if (i == 2) {
      lt_display[5] = 0x14;
      DisplayUart.write(lt_display, sizeof(lt_display));
    }
    if (i == 3) {
      fw_rt_display[5] = 0x14;
      DisplayUart.write(fw_rt_display, sizeof(fw_rt_display));
    }
    if (i == 4) {
      fw_lt_display[5] = 0x14;
      DisplayUart.write(fw_lt_display, sizeof(fw_lt_display));
    }
    if (i == 5) {
      cross_display[5] = 0x14;
      DisplayUart.write(cross_display, sizeof(cross_display));
    }
    if (i == 6) {
      round_display[5] = 0x14;
      DisplayUart.write(round_display, sizeof(round_display));
    }
    if (i == 7) {
      hos_display[5] = 0x14;
      DisplayUart.write(hos_display, sizeof(hos_display));
    }
    if (i == 8) {
      sch_display[5] = 0x14;
      DisplayUart.write(sch_display, sizeof(sch_display));
    }
  }
  else if (overlimit == 0) {
    DisplayUart.write(ul_display, sizeof(ul_display));
    if (i == 0) {
      DisplayUart.write(fd_display, sizeof(fd_display));
    }
    if (i == 1) {
      DisplayUart.write(rt_display, sizeof(rt_display));
    }
    if (i == 2) {
      DisplayUart.write(lt_display, sizeof(lt_display));
    }
    if (i == 3) {
      DisplayUart.write(fw_rt_display, sizeof(fw_rt_display));
    }
    if (i == 4) {
      DisplayUart.write(fw_lt_display, sizeof(fw_lt_display));
    }
    if (i == 5) {
      DisplayUart.write(cross_display, sizeof(cross_display));
    }
    if (i == 6) {
      DisplayUart.write(round_display, sizeof(round_display));
    }
    if (i == 7) {
      DisplayUart.write(hos_display, sizeof(hos_display));
    }
    if (i == 8) {
      DisplayUart.write(sch_display, sizeof(sch_display));
    }
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

int displayText(byte spd, int i, int speedlimit = 0, int overlimit = 0) {
  byte displaytext[]      = {0x5A, 0xA5, 0x08, 0x82, 0x11, 0x50, 0x30, 0x30, 0x30, 0xFF, 0xFF};
  byte limitdisplaytext[] = {0x5A, 0xA5, 0x08, 0x82, 0x11, 0x60, 0x30, 0x30, 0x30, 0xFF, 0xFF};

  int digit1, digit2, digit3;
  extractDigits(spd, digit1, digit2, digit3);

  digit1 = integerToAsciiByte(digit1);
  digit2 = integerToAsciiByte(digit2);
  digit3 = integerToAsciiByte(digit3);

  int limit_digit1, limit_digit2, limit_digit3;
  extractDigits(speedlimit, limit_digit1, limit_digit2, limit_digit3);

  limit_digit1 = integerToAsciiByte(limit_digit1);
  limit_digit2 = integerToAsciiByte(limit_digit2);
  limit_digit3 = integerToAsciiByte(limit_digit3);

  if (overlimit == 0) {
    displaytext[4] = 0x11; //d1
    displaytext[5] = 0x10; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd

    limitdisplaytext[4] = 0x11; //d1
    limitdisplaytext[5] = 0x20; //d1
    limitdisplaytext[6] = limit_digit1; //spd
    limitdisplaytext[7] = limit_digit2; //spd
    limitdisplaytext[8] = limit_digit3; //spd
  } else if (overlimit == 1) {
    displaytext[4] = 0x11; //d1
    displaytext[5] = 0x30; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd

    limitdisplaytext[4] = 0x11; //d1
    limitdisplaytext[5] = 0x40; //d1
    limitdisplaytext[6] = limit_digit1; //spd
    limitdisplaytext[7] = limit_digit2; //spd
    limitdisplaytext[8] = limit_digit3; //spd
  }

  if (i == OOR_DISPLAY) {
    displaytext[4] = 0x11; //d1
    displaytext[5] = 0x50; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd

    limitdisplaytext[4] = 0x11; //d1
    limitdisplaytext[5] = 0x60; //d1
    limitdisplaytext[6] = limit_digit1; //spd
    limitdisplaytext[7] = limit_digit2; //spd
    limitdisplaytext[8] = limit_digit3; //spd
  }
  DisplayUart.write(displaytext, sizeof(displaytext));
  delay(5);
  DisplayUart.write(limitdisplaytext, sizeof(limitdisplaytext));
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
  setDisplay(OOR_DISPLAY);
  displayText(0, OOR_DISPLAY, 40);

}

String str = "", latz = "" , lonz = "" , spdz = "", mapstr = "";
float mycompass = 0, compprev = 0;
int disSelect = OOR_DISPLAY;
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
    //mylat = 0;

    lonz = str.substring(45);
    lonz = lonz.substring(0, 8);
    mylon = lonz.toDouble();
    DebugUart.print(mylon, 6); DebugUart.print('\t');
    //mylon = 0;

    spdz = str.substring(73);
    spdz = spdz.substring(0, 3);
    myspd = spdz.toInt();
    DebugUart.print(myspd); DebugUart.print(" \t ");

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
        //todo
        DisplayUart.write(buz, sizeof(buz));
        disSelect = file_dir;
        routeindex++;
      }

      if (myspd > file_spd) {
        DisplayUart.write(buz, sizeof(buz));
        setDisplay(disSelect, 1);
        displayText(myspd, disSelect, file_spd, 1);
      }
      else {
        setDisplay(disSelect, 0);
        displayText(myspd, disSelect, file_spd, 0);
      }
    }
    else {
      setDisplay(OOR_DISPLAY);
      if (myspd > SPEED_LIMIT) {
        DisplayUart.write(buz, sizeof(buz));
      }
      displayText(myspd, OOR_DISPLAY, SPEED_LIMIT);
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
