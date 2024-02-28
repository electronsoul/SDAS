#include <HardwareSerial.h>
#include <config_sys.h>
#include <init_sys.h>

String getMessageQuectel() {
  String str = "";
  while (QuectelUart.available()) {
    char ch = QuectelUart.read();
    str = str + String(ch);
  }
  str = str + "\0";
  return str;
}

HardwareSerial Serial1(PB7, PB6); //RX, TX
HardwareSerial Serial2(PA3, PA2);

void setup() {
  SystemClock_Config();

  DebugUart.begin(115200);
  DebugUart.println("SETUP");

  QuectelUart.begin(115200);

  pinMode(PA13, OUTPUT);
  digitalWrite(PA13, HIGH);

  delay(5000);

  //DebugUart.println("AT+QGPSLOC=2");
  //QuectelUart.println("AT+QGPSLOC=2");
  //delay(100);
  //DebugUart.println("AT+QGPSCFG=\"nmeasrc\",1");
  //QuectelUart.println("AT+QGPSCFG=\"nmeasrc\",1");
  //delay(100);
  //DebugUart.println("AT+QGPSGNMEA=\"GGA\"");
  //QuectelUart.println("AT+QGPSGNMEA=\"GGA\"");
  //delay(100);
}

void loop() {
  String str;
  DebugUart.println("AT+QGPSLOC=2");
  QuectelUart.println("AT+QGPSLOC=2");
  delay(100);
  str = getMessageQuectel();
  if (str != "") {
    DebugUart.println(str);
  }
  delay(500);
}
