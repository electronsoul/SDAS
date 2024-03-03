#pragma once
#ifndef EC200_MODULE_H
#define EC200_MODULE_H

String InitCommandList[] = {"AT", "ATI", "ATV1", "ATE1", "AT+CMEE=2"
                            "AT+IPR?", "AT+GSN", "AT+QURCCFG=\"URCPORT","uart1", 
                            "AT+CPIN?", "AT+CIMI", "AT+QCCID", "AT+CSQ", 
                            "AT+CREG?", "AT+CGREG?", "AT+COPS?", "AT+CEREG?"};

String getMessageQuectel(void);
int initQuectel(void);

#endif
