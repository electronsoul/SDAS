/*
 routeindex = 1;
 deserializeJson(jsonBuffer, String(routedata_sec85[0]));
  lat1 = jsonBuffer["lat"].as<double>();
  lon1 = jsonBuffer["lon"].as<double>();
  dir = jsonBuffer["dir"].as<int>();
}

void loop()
{
  if ( routeindex < route_size) {
    deserializeJson(jsonBuffer, String(routedata_sec85[routeindex]));
    lat2 = jsonBuffer["lat"].as<double>();
    lon2 = jsonBuffer["lon"].as<double>();
    routeindex++;
   if (1) //myICM.dataReady())
    {
      //myICM.getAGMT();
      //compass(myICM.agmt, &myICM, myCompass);
      d1 = calculateInitialCompassBearing(lat1, lon1, lat2, lon2);
      String direction_str = getDirection(d1, d2);
      Serial.print(direction_str);
      d2 = d1; // to keep track of prev bearing
      lat1 = lat2;
      lon1 = lon2;
    }
    Serial.println();
  }
  delay(50);
 */
