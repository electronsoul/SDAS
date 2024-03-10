#include <ec200_gnss.h>

String getLocationCoords() {
  String str = "";
  ModemUart.println(ModemGNSSCommandList[10]);
  delay(500);
  str = getMessageModem();
  return str;
}

int checkGNSSRunState(void) {
  int isRunning = 0;
  String str = "";

  delay(500);
  ModemUart.println(ModemGNSSCommandList[3]); //Check GPS Running Status

  delay(500);
  str = getMessageModem();
  if (str != "") {
    DebugUart.println(str);
    if (str.indexOf("+QGPS: 1") != -1) {
      isRunning = 1;
    }
  }
  delay(500);

  return isRunning;
}

int initModemGNSS(void) {
  int initStatus = 1;
  String str;
  int commandListLength = sizeof(ModemGNSSCommandList) / sizeof(ModemGNSSCommandList[0]);

  DebugUart.println("Modem GNSS Initialization Begin");

  int isRunning = checkGNSSRunState();
  DebugUart.println(isRunning);
  if (isRunning == 0) {
    DebugUart.println("GNSS is Off... Turning On");
    delay(500);
    ModemUart.println(ModemGNSSCommandList[4]);
    delay(500);
  } else if (isRunning == 1) {
    DebugUart.println("GNSS is already On");
    delay(500);
  }

  for (int itr = 11; itr > 10 and itr < 15; itr++) {
    ModemUart.println(ModemGNSSCommandList[itr]);
    delay(500);
    str = getMessageModem();
    if (str != "") {
      DebugUart.println(str);
    }
    delay(500);
  }

  DebugUart.println("Modem GNSS Initialization End");

  return initStatus;
}
