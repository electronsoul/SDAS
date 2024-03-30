#include <HardwareSerial.h>
#include <config_sys.h>
#include <EasyLogger.h>
#include <string.h>
#include <stdio.h>

#include <init_sys.h>
#include <ec200_module.h>
#include <ec200_gnss.h>
#include "SDMEM.h"
#include "route.h"
#include <ICM_20948.h>
#include "icm20.h"

#include <ArduinoJson.h>
const size_t bufferSize = JSON_OBJECT_SIZE(50);
StaticJsonDocument<bufferSize> jsonBuffer;

ICM_20948_I2C myICM;

HardwareSerial Serial1(USART1_RX, USART1_TX);
HardwareSerial Serial2(USART2_RX, USART2_TX);
HardwareSerial Serial3(USART3_RX, USART3_TX);

_SYS_MODULES_t _sys_module = SYSTEM_M;


byte fd_display[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x04};
byte rt_display[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x05};
byte lt_display[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x06};
byte ut_display[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x07};

void setDisplay(int i) {
  if (i == 1) {
    DisplayUart.write(fd_display, sizeof(fd_display));
  }
  else if (i == 2) {
    DisplayUart.write(rt_display, sizeof(rt_display));
  }
  else if (i == 3) {
    DisplayUart.write(lt_display, sizeof(lt_display));
  }
  else if (i == 4) {
    DisplayUart.write(ut_display, sizeof(ut_display));
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

void displayText(byte spd, int i) {
  byte displaytext[] = {0x5A, 0xA5, 0x08, 0x82, 0x70, 0x00, 0x30, 0x30, 0x30, 0xFF, 0xFF};
  int digit1, digit2, digit3;
  extractDigits(spd, digit1, digit2, digit3);

  digit1 = integerToAsciiByte(digit1);
  digit2 = integerToAsciiByte(digit2);
  digit3 = integerToAsciiByte(digit3);

  if (i == 1) {
    displaytext[4] = 0x70; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 2) {
    displaytext[4] = 0x80; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 3) {
    displaytext[4] = 0x90; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }
  else if (i == 4) {
    displaytext[4] = 0xA0; //d1
    displaytext[6] = digit1; //spd
    displaytext[7] = digit2; //spd
    displaytext[8] = digit3; //spd
  }

  DisplayUart.write(displaytext, sizeof(displaytext));
  //delay(50);
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
  pinMode(PD12, OUTPUT);
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

  if (flagStatus == 1) {
    DebugUart.println("System Initialization Successful");
  }
  else if ( flagStatus == 0) {
    DebugUart.println("System Initialization Failed");
    DebugUart.println("System Resetting...");
    //NVIC_SystemReset();
  }

  while (digitalRead(PA0) == LOW); //add dispaly switch input
}

void loop() {
  String str = "", latz = "" , lonz = "" , spdz = "", mapstr = "";
  _sys_module = EC200_M;
  float mycompass = 0, compprev = 0;
  int routeindex = 0;
  int disSelect = 1;

  digitalWrite(PD12, HIGH);
  str = getLocationCoords();
  if (str.startsWith("AT+QGPSLOC")) { // != " ") {

    latz = str.substring(36);
    latz = latz.substring(0, 8);
    double mylat = latz.toDouble();
    //DebugUart.print(mylat);
    //DebugUart.print(" \t ");
    lonz = str.substring(45);
    lonz = lonz.substring(0, 8);
    double mylon = lonz.toDouble();
    //DebugUart.print(mylon, 6);
    //DebugUart.print(" \t ");
    spdz = str.substring(73);
    spdz = spdz.substring(0, 3);
    int myspd = spdz.toInt();
    //DebugUart.print(myspd);
    //DebugUart.print(" \t ");
    //appendLineToSDCard("log.txt", latz + String('\t') + lonz + String('\t') + spdz);
    /*if (myICM.dataReady())
      {
      myICM.getAGMT();
      compass(myICM.agmt);
      DebugUart.print(retarr[2]);
      // DebugUart.print(" \t ");
      }
    */

    deserializeJson(jsonBuffer, String(routedata[routeindex]));
    double file_lat = jsonBuffer["lat"].as<double>();
    double file_lon = jsonBuffer["lon"].as<double>();
    int file_spd = jsonBuffer["speed"].as<int>();
    if (myICM.dataReady())
    {
      myICM.getAGMT();
      compass(myICM.agmt);
      mycompass = retarr[1];  //taking gyro
    }
    if (mycompass > -1 and mycompass < 1) {
      disSelect = 1;
    } else if (mycompass > 2 and mycompass < 6) {
      disSelect = 3;
    } else if (mycompass > -6 and mycompass < -3) {
      disSelect = 2;
    } else if ((mycompass > 9 and mycompass < 13) or (mycompass > -13 and mycompass < -9)) {
      disSelect = 4;
    }

    setDisplay(disSelect);
    displayText(myspd, disSelect); //lt screen

    //compare lat lon, while changed to if
    if (mylat <= file_lat + 0.00005 && mylat >= file_lat - 0.00005) {
      setDisplay(1);

      if (myspd <= file_spd) {
        displayText(myspd, 1); //first screen
        //display_forward(spd, 1, 0); //screen 1, overspeed = 1
      }
      else {
        displayText(myspd, 1); //first screen
        //display_forward(spd, 1, 0); //screen 1, overspeed = 1
      }
      if (myICM.dataReady())
      {
        myICM.getAGMT();
        compass(myICM.agmt);
        mycompass = retarr[2];
      }

      if ((mycompass - compprev) > 40) { //check to see if only posite comparrison works
        setDisplay(2); //right
        if (myspd <= file_spd) {
          displayText(myspd, 2); //rt screen
          //display_right(spd, 0); //overspeed = 1
        }
        else {
          displayText(myspd, 2); //rt screen
          //display_right(spd, 1); //overspeed = 1
        }
      }

      else if ((mycompass - compprev) < 0) {
        setDisplay(3); // left
        if (myspd <= file_spd) {
          displayText(myspd, 3); //lt screen
          //display_left(spd, 0); //overspeed = 1
        }
        else {
          displayText(myspd, 3); //lt screen
          //display_leftt(spd, 1); //overspeed = 1
        }
      }

      compprev = mycompass;
      routeindex++;
    }

    //DebugUart.println();
    str = "";
  }
  delay(100); //prev it was taking 4-5 samples at 100ms, repaeating samples
}
