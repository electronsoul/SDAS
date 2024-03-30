//#pragma once
#ifndef ICM20_H
#define ICM20_H

float retarr[3];

//ICM_20948_I2C myICM;
float compass(ICM_20948_AGMT_t); // agmt = myICM.agmt, ICM_20948_I2C *sensor = &myICM);
void initICM20();

#endif
