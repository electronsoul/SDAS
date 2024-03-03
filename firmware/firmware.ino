#include <HardwareSerial.h>
#include <config_sys.h>
#include <init_sys.h>
#include <ec200_module.h>

HardwareSerial Serial1(PB7, PB6); //RX, TX
HardwareSerial Serial2(PA3, PA2);

void setup() {

  int flagStatus = 1;
  SystemClock_Config();

  pinMode(PA1, OUTPUT); //pwrkey
  pinMode(PA0, OUTPUT); //rst
  digitalWrite(PA1, LOW);
  delay(4000);
  digitalWrite(PA0, LOW);
  
  DebugUart.begin(115200);
  DebugUart.println("System Initialization Begin");
  
  flagStatus &= initQuectel();

  if (flagStatus == 1) {
    DebugUart.println("System Initialization Successful");
  }
  else if ( flagStatus == 0) {
    DebugUart.println("System Initialization Failed");
    DebugUart.println("System Resetting...");
    NVIC_SystemReset();
  }
}

void loop() {
  /*String str;
    DebugUart.println("AT+QGPSLOC=2");
    QuectelUart.println("AT+QGPSLOC=2");

    delay(1000);
    str = getMessageQuectel();
    if (str != "") {
    DebugUart.println(str);
    }
    delay(500);*/
}
