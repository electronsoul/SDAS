#include <ec200_module.h>

String getMessageQuectel(void) {
  String str = "";
  while (QuectelUart.available()) {
    char ch = QuectelUart.read();
    str = str + String(ch);
  }
  str = str + "\0";
  return str;
}

int initQuectel(void) {
  int initStatus = 1;
  String str;

  DebugUart.println("Quectel Modem Initialization Begin");
  QuectelUart.begin(115200);

  for (int itr = 0; itr < (sizeof(InitCommandList) / sizeof(InitCommandList[0])); itr++) {
    QuectelUart.println(InitCommandList[itr]);
    str = getMessageQuectel();
    if (str != "") {
      DebugUart.println(str);
    }
  }

  QuectelUart.println("AT+QGPSCFG=\"outport\",\"none\"");
  //delay(5000);

  //DebugUart.println("AT+QGPSLOC=2");
  //QuectelUart.println("AT+QGPSLOC=2");
  //delay(100);
  //DebugUart.println("AT+QGPSCFG=\"nmeasrc\",1");
  //QuectelUart.println("AT+QGPSCFG=\"nmeasrc\",1");
  //delay(100);
  //DebugUart.println("AT+QGPSGNMEA=\"GGA\"");
  //QuectelUart.println("AT+QGPSGNMEA=\"GGA\"");
  //delay(100);

  return initStatus;
}
