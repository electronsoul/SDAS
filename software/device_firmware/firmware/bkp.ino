
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
