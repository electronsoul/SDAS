
/*for (int i = 0; i < 4; i++) {
  DebugUart.write(Forward, sizeof(Forward));
  DisplayUart.write(Forward, sizeof(Forward));
  Forward[9]++;
  delay(1000); // Delay between each value for better readability
  }*/

/*
  DisplayUart.write(Forward, sizeof(Forward));
  DisplayUart.write(displaytext, sizeof(displaytext));
  DisplayUart.write(displaytext2, sizeof(displaytext2));
*/

/*
  _sys_module = SDMEM_M;
  char _buffer[100];
  if (initSDCard()) {
  LOG_INFO(_SYSTEM_MODULE_LIST[_sys_module], "SD CARD DETECTED");
  printSDCardSpecifications();
  //String str = getLineByIndex("mapdata.txt", 13);
  strcpy_P(_buffer, (char *)pgm_read_ptr(&(routedata[600])));
  DebugUart.println(_buffer);
  }
*/

/*
  DebugUart.println("Powering Down");
  powerDownModem();
  delay(2000);
  DebugUart.println("Powering Up");
  powerUpModem();
*/

/*
  if (myICM.dataReady())
  {
    myICM.getAGMT();
    compass(myICM.agmt);
    DebugUart.print(retarr[0]);
    DebugUart.print(" \t ");
    DebugUart.print(retarr[1]);
    DebugUart.print(" \t ");
    DebugUart.println(retarr[2]);
    appendLineToSDCard("log.txt", latz + String('\t') + lonz + String('\t') + String('\t') + spdz + String('\t') + String(retarr[0]) + String('\t') + String(retarr[1]) + String('\t') + String(retarr[2]));
  }
*/

//appendLineToSDCard("log.txt", latz + String('\t') + lonz + String('\t') + spdz);

/*if (myICM.dataReady())
  {
  myICM.getAGMT();
  compass(myICM.agmt);
  DebugUart.print(retarr[2]);
  // DebugUart.print(" \t ");
  }
*/

/*if (myICM.dataReady())
  {
  myICM.getAGMT();
  compass(myICM.agmt);
  mycompass = retarr[1];  //taking gyro
  }
  if (mycompass > -1 and mycompass < 1) {
  disSelect = 1;
  } else if (mycompass > 3 and mycompass < 6) {
  disSelect = 3;
  } else if (mycompass > -6 and mycompass < -3) {
  disSelect = 2;
  } else if ((mycompass > 9 and mycompass < 13) or (mycompass > -13 and mycompass < -9)) {
  disSelect = 4;
  }
*/

/*
        if (myICM.dataReady())
        {
        myICM.getAGMT();
        compass(myICM.agmt);
        mycompass = retarr[1];  //taking gyro
        }

        if (mycompass > -1 and mycompass < 1) {
        disSelect = 1; DebugUart.print("FWD"); DebugUart.print('\t');
        } else if (mycompass > 3 and mycompass < 6) {
        disSelect = 3; DebugUart.print("RIGHT"); DebugUart.print('\t');
        } else if (mycompass > -6 and mycompass < -3) {
        disSelect = 2; DebugUart.print("LEFT"); DebugUart.print('\t');
        } else if ((mycompass > 9 and mycompass < 13) or (mycompass > -13 and mycompass < -9)) {
        disSelect = 4;
        }
*/
