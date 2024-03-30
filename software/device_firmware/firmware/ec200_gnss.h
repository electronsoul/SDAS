#pragma once
#ifndef EC200_GNSS_H
#define EC200_GNSS_H

String ModemGNSSCommandList[] = {"AT+QGPSCFG=\"outport\",\"none\"", 
                                 "AT+QGPSCFG=\"autogps\",0",
                                 "AT+QGPSCFG=\"apflash\",1", 
                                 "AT+QGPS?", "AT+QGPS=1", "AT+QGPS=0",
                                 "AT+QGPSEND", 
                                 "AT+QAGPS?", "AT+QAGPS=0", "AT+QAGPS=1",
                                 "AT+QGPSLOC=2", "AT+QGPSINFO?", 
                                 "AT+QGPSCFG=\"nmeasrc\",1",
                                 "AT+QGPSGNMEA=\"RMC\"",
                                 "AT+QGPSCFG=\"outport\",\"uart1\"",
                                 "AT+QGPSCFG=\"apflash\",0"
                                };

int initModemGNSS(void);
int checkGNSSRunState(void);
String getLocationCoords(void);
String getSpeedFromGps();
#endif
