#include <ec200_module.h>

void resetModem() {
  digitalWrite(MODEMRESET, HIGH);
  delay(500);
  digitalWrite(MODEMRESET, LOW);
  delay(10000);
}

void powerDownModem() {
  //delay(500);
  //ModemUart.println("AT+QPOWD");
  digitalWrite(MODEMPOWEREY, HIGH);
  delay(4000);
  digitalWrite(MODEMPOWEREY, LOW);
}

void powerUpModem() {
  digitalWrite(MODEMPOWEREY, HIGH);
  delay(2000);
  digitalWrite(MODEMPOWEREY, LOW);
  delay(2000);
}


int initModem(void) {
  int initStatus = 1;
  String str;
  //resetModem();

  DebugUart.println("Modem Initialization Begin");
  ModemUart.begin(115200);
  delay(500);

  for (int itr = 0; itr < ((sizeof(InitCommandList) / sizeof(InitCommandList[0]))); itr++) {
    ModemUart.println(InitCommandList[itr]);
    str = getMessageModem();
    if (str != "") {
      DebugUart.println(str);
      initStatus = 0;
      if (str.indexOf("OK") != -1) {
        initStatus = 1;
      }
    }
    delay(500);
  }
  DebugUart.println("Modem Initialization End");
  return initStatus;
}
