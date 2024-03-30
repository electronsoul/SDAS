#pragma once
#ifndef SDMEM_H
#define SDMEM_H


int initSDCard(void);
void printSDCardSpecifications(void);
String getLineByIndex(String, int);
String readLineFromSDCard(String);
int parseJsonData(String);
String formJsonData(int []);
int writeLineToSDCard(String, String );
int appendLineToSDCard(String , String );


#endif
