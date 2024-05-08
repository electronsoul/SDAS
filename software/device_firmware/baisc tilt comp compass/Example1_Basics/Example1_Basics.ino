#include "ICM_20948.h"
#include <cmath>
#include "route.h"
#include <ArduinoJson.h>

const size_t bufferSize = JSON_OBJECT_SIZE(50);
StaticJsonDocument<bufferSize> jsonBuffer;

#define SERIAL_PORT Serial
#define WIRE_PORT Wire
ICM_20948_I2C myICM;


#define Frequency 125                                                   // 8mS sample interval 
#define Sensitivity 62.5                                                // Gyro sensitivity (see data sheet)
#define Sensor_to_deg 1/(Sensitivity*Frequency)                         // Convert sensor reading to degrees
#define Sensor_to_rad Sensor_to_deg*DEG_TO_RAD                          // Convert sensor reading to radians

int dir = 1;
unsigned long prevTime = 0;
float prevSpeed = 0.0;
float totalDistance = 0.0;
int     Gyro_x,     Gyro_y,     Gyro_z;
long    Gyro_x_cal, Gyro_y_cal, Gyro_z_cal;
float   Gyro_pitch, Gyro_roll, Gyro_yaw;
float   Gyro_pitch_output, Gyro_roll_output;
long    Accel_x,      Accel_y,      Accel_z,    Accel_total_vector;
float   Accel_pitch,  Accel_roll;
int Gyro_synchronised = false;
float   declinationAngle = +1.41666;                                             //  Degrees ... replace this declination with yours
int     headingDegrees;
float distance = 0;
int     Mag_x,                Mag_y,                Mag_z;                  // Raw magnetometer readings
float   Mag_x_dampened,       Mag_y_dampened,       Mag_z_dampened;
float   Mag_x_hor, Mag_y_hor;
float   Mag_pitch, Mag_roll;

int routeindex = 0;
int myCompass = 0;
double lat1, lon1, lat2, lon2;
double d1 = 0, d2 = 0;

double toRadians(double degrees) {
  return (degrees * PI ) / 180.0;
}

String getDirection(int d) { //current , previous
  // Define compass directions
  //const String directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
  // Divide the compass into sectors
  //const int num_sectors = sizeof(directions) / sizeof(directions[0]);
  //const double sector_size = 360.0 / num_sectors;
  // Determine the sector
  //int sector = static_cast<int>((bearing + sector_size / 2) / sector_size) % num_sectors;
  // Assign directions based on the sector
  /*if (sector == 0 || sector == 7) return "Forward";
    else if (sector == 1 || sector == 2) return "Right";
    else if (sector == 3 || sector == 4) return "Backward or U-Turn";
    else if (sector == 5 || sector == 6) return "Left";
    else return "Unknown";*/
  String dir = "Forward";

  if (d == 2) {
    dir = "Right";
  } else if (d == 3) {
    dir = "Left";
  }

  return dir;
}

double calculateInitialCompassBearing(double lat1, double lon1, double lat2, double lon2) {
  // Convert decimal degrees to radians
  lat1 = toRadians(lat1);
  lon1 = toRadians(lon1);
  lat2 = toRadians(lat2);
  lon2 = toRadians(lon2);

  // Compute difference in longitudes
  double diff_lon = lon2 - lon1;

  // Compute the bearing using trigonometry
  double y = sin(diff_lon) * cos(lat2);
  double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(diff_lon);
  double bearing = atan2(y, x);

  // Convert bearing from radians to degrees
  bearing = bearing * 180.0 / PI;

  // Normalize to compass bearing (0 to 360 degrees)
  double compass_bearing = fmod((bearing + 360), 360);

  return compass_bearing;
}

void compass(ICM_20948_AGMT_t agmt, ICM_20948_I2C *sensor, int &myCompass) {
  Accel_x = agmt.acc.axes.x;
  Accel_y = agmt.acc.axes.y;
  Accel_z = agmt.acc.axes.z;

  Gyro_x = agmt.gyr.axes.x;
  Gyro_y = agmt.gyr.axes.y;
  Gyro_z = agmt.gyr.axes.z;

  Mag_x = agmt.mag.axes.x;
  Mag_y = agmt.mag.axes.y;
  Mag_z = agmt.mag.axes.z;
  // ----- Calculate travelled angles
  /*
    ---------------------------
    Adjust Gyro_xyz signs for:
    ---------------------------
    Pitch (Nose - up) = +ve reading
    Roll (Right - wing down) = +ve reading
    Yaw (Clock - wise rotation)  = +ve reading
  */
  Gyro_pitch += -Gyro_y * Sensor_to_deg;                            // Integrate the raw Gyro_y readings
  Gyro_roll += Gyro_x * Sensor_to_deg;                              // Integrate the raw Gyro_x readings
  Gyro_yaw += -Gyro_z * Sensor_to_deg;                              // Integrate the raw Gyro_x readings

  // ----- Compensate pitch and roll for gyro yaw
  Gyro_pitch += Gyro_roll * sin(Gyro_z * Sensor_to_rad);            // Transfer the roll angle to the pitch angle if the Z-axis has yawed
  Gyro_roll -= Gyro_pitch * sin(Gyro_z * Sensor_to_rad);            // Transfer the pitch angle to the roll angle if the Z-axis has yawed

  // ----- Accelerometer angle calculations
  Accel_total_vector = sqrt((Accel_x * Accel_x) + (Accel_y * Accel_y) + (Accel_z * Accel_z));   // Calculate the total (3D) vector
  Accel_pitch = asin((float)Accel_x / Accel_total_vector) * RAD_TO_DEG;                         //Calculate the pitch angle
  Accel_roll = asin((float)Accel_y / Accel_total_vector) * RAD_TO_DEG;                         //Calculate the roll angle

  // ----- Zero any residual accelerometer readings
  /*
     Place the accelerometer on a level surface
     Adjust the following two values until the pitch and roll readings are zero
  */
  Accel_pitch -= -0.2f;                                             //Accelerometer calibration value for pitch
  Accel_roll -= 1.1f;                                               //Accelerometer calibration value for roll


  // ----- Correct for any gyro drift
  if (Gyro_synchronised)
  {
    // ----- Gyro & accel have been synchronised
    Gyro_pitch = Gyro_pitch * 0.9996 + Accel_pitch * 0.0004;        //Correct the drift of the gyro pitch angle with the accelerometer pitch angle
    Gyro_roll = Gyro_roll * 0.9996 + Accel_roll * 0.0004;           //Correct the drift of the gyro roll angle with the accelerometer roll angle
  }
  else
  {
    // ----- Synchronise gyro & accel
    Gyro_pitch = Accel_pitch;                                       //Set the gyro pitch angle equal to the accelerometer pitch angle
    Gyro_roll = Accel_roll;                                         //Set the gyro roll angle equal to the accelerometer roll angle
    Gyro_synchronised = true;                                             //Set the IMU started flag
  }

  // ----- Dampen the pitch and roll angles
  Gyro_pitch_output = Gyro_pitch_output * 0.9 + Gyro_pitch * 0.1;   //Take 90% of the output pitch value and add 10% of the raw pitch value
  Gyro_roll_output = Gyro_roll_output * 0.9 + Gyro_roll * 0.1;      //Take 90% of the output roll value and add 10% of the raw roll value



  // ----- Fix the pitch, roll, & signs
  /*
     MPU-9250 gyro and AK8963 magnetometer XY axes are orientated 90 degrees to each other
     which means that Mag_pitch equates to the Gyro_roll and Mag_roll equates to the Gryro_pitch

     The MPU-9520 and AK8963 Z axes point in opposite directions
     which means that the sign for Mag_pitch must be negative to compensate.
  */
  Mag_pitch = -1 * Gyro_pitch_output * DEG_TO_RAD;
  Mag_roll = Gyro_roll_output * DEG_TO_RAD;

  // ----- Apply the standard tilt formulas
  Mag_x_hor = Mag_x * cos(Mag_pitch) + Mag_y * sin(Mag_roll) * sin(Mag_pitch) - Mag_z * cos(Mag_roll) * sin(Mag_pitch);
  Mag_y_hor = Mag_y * cos(Mag_roll) + Mag_z * sin(Mag_roll);

  // ----- Dampen any data fluctuations
  Mag_x_dampened = Mag_x_dampened * 0.9 + Mag_x_hor * 0.1;
  Mag_y_dampened = Mag_y_dampened * 0.9 + Mag_y_hor * 0.1;

  // ----- Calculate the heading
  headingDegrees = atan2(-1 * Mag_y_dampened, Mag_x_dampened) * RAD_TO_DEG; // Magnetic North

  // ----- Correct for True North
  declinationAngle = +5;
  headingDegrees += declinationAngle;                                   // Geographic North

  // ----- Allow for under/overflow
  if (headingDegrees < 0) headingDegrees += 360;
  if (headingDegrees >= 360) headingDegrees -= 360;

  unsigned long currentTime = millis();
  float prevacX = 0, curacX = 0, dt = (currentTime - prevTime) / 1000.0; // Convert milliseconds to second
  int tmp = millis();
  prevacX = (((0.98 * agmt.acc.axes.x + 0.02 * agmt.acc.axes.z) / 16384 ) - 0.04);
  while ((millis() - tmp) < 100) {
    curacX = (((0.98 * agmt.acc.axes.x + 0.02 * agmt.acc.axes.z) / 16384 ) - 0.04);
    curacX = (prevacX + curacX) / 2;
    prevacX = curacX;
    //float acX = sensor->accZ();
  }
  if (curacX < 0.03 ) { //offset factor for stationary calib
    curacX = 0;
  }
  //Serial.print(curacX);

  float currentSpeed = prevSpeed + (curacX) * dt;
  if ((currentSpeed != prevSpeed) ) {
    distance = (prevSpeed + currentSpeed) / 2.0 * dt;
    totalDistance += distance;
  }
  // Update previous values
  prevSpeed = currentSpeed;
  prevTime = currentTime;

  //Serial.print(" \t");
  //Serial.print(currentSpeed); // Print speed with 4 decimal places
  //Serial.print(" \t");
  //Serial.print(totalDistance); // Print distance with 4 decimal places
  //Serial.print(" \t");
  //Serial.println(headingDegrees);
  myCompass = headingDegrees;
}

void setup()
{
  SERIAL_PORT.setTx(PB6);
  SERIAL_PORT.setRx(PB7);
  SERIAL_PORT.begin(115200);

  WIRE_PORT.setSCL(PA8);
  WIRE_PORT.setSDA(PC9);
  WIRE_PORT.begin();
  WIRE_PORT.setClock(400000);

  bool initialized = false;
  while (!initialized)
  {
    myICM.begin(WIRE_PORT, 1);
    SERIAL_PORT.print(F("Initialization of the sensor returned: "));
    SERIAL_PORT.println(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok)
    {
      SERIAL_PORT.println("Trying again...");
      delay(500);
    }
    else
    {
      initialized = true;
    }
  }
  //deserializeJson(jsonBuffer, String(routedata_sec85[0]));
  //lat1 = jsonBuffer["lat"].as<double>();
  //lon1 = jsonBuffer["lon"].as<double>();
}

void loop()
{
  /*if ( routeindex < route_size) {
    deserializeJson(jsonBuffer, String(routedata_sec85[routeindex]));
    lat1 = jsonBuffer["lat"].as<double>();
    lon1 = jsonBuffer["lon"].as<double>();
    dir = jsonBuffer["dir"].as<int>();

    String direction_str = getDirection(dir);
    Serial.print(direction_str);
    lat1 = lat2;
    lon1 = lon2;

    routeindex++;
    Serial.println();
    }*/

  if (myICM.dataReady())
  {
    myICM.getAGMT();
    ICM_20948_I2C *sensor = &myICM;
    ICM_20948_AGMT_t agmt = myICM.agmt;

    //Serial.print(agmt.gyr.axes.x); Serial.print('\t');
    //Serial.print(agmt.gyr.axes.y); Serial.print('\t');
    //Serial.print(agmt.gyr.axes.z); Serial.print('\t');

    Serial.print(sensor->gyrX()); Serial.print('\t');
    //Serial.print(sensor->gyrY()); Serial.print('\t');
    //Serial.print(sensor->gyrZ()); Serial.print('\t');
    Serial.println();
  }
  delay(50);
}
