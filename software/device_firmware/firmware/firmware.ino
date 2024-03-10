#include <HardwareSerial.h>
#include <config_sys.h>
#include <init_sys.h>
#include <ec200_module.h>
#include <ec200_gnss.h>

HardwareSerial Serial1(USART1_RX, USART1_TX);
HardwareSerial Serial2(USART2_RX, USART2_TX);

String getMessageModem(void) {
  String str = "";
  while (ModemUart.available()) {
    char ch = ModemUart.read();
    str = str + String(ch);
    //delay(10);
  }
  str = str + "\0";
  return str;
}

void setup() {

  int flagStatus = 1;
  SystemClock_Config();

  DebugUart.begin(115200);
  delay(500);
  DebugUart.println("System Initialization Begin");

  pinMode(MODEMPOWEREY, OUTPUT);
  pinMode(MODEMRESET, OUTPUT);
  digitalWrite(MODEMPOWEREY, LOW);
  digitalWrite(MODEMRESET, LOW);

  flagStatus &= initModem();
  flagStatus &= initModemGNSS();

  if (flagStatus == 1) {
    DebugUart.println("System Initialization Successful");
  }
  else if ( flagStatus == 0) {
    DebugUart.println("System Initialization Failed");
    DebugUart.println("System Resetting...");
    NVIC_SystemReset();
  }

  /*
    DebugUart.println("Powering Down");
    powerDownModem();
    delay(2000);
    DebugUart.println("Powering Up");
    powerUpModem();
  */
}

void loop() {
  String str;
  str = getLocationCoords();
  if (str != "") {
    DebugUart.println(str);
  }
  delay(500);
}
