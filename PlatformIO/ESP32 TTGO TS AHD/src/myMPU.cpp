/*
Advanced_I2C.ino
Brian R Taylor
brian.taylor@bolderflight.com
Copyright (c) 2017 Bolder Flight Systems
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, 
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or 
substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <Arduino.h>
#include <Wire.h>
#include "MPU9250.h"
extern TwoWire Wire;

// an MPU9250 object with the MPU-9250 sensor on I2C bus 0 with address 0x68
MPU9250 IMU(Wire,0x69);
int status;
float pitch,roll;

void setupIMU() {
  // serial to display data
  //Serial.begin(115200);
  //while(!Serial) {}

  // start communication with IMU 
  status = IMU.begin();
  if (status < 0) {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(status);
    while(1) {}
  }
  // setting the accelerometer full scale range to +/-8G 
  IMU.setAccelRange(MPU9250::ACCEL_RANGE_8G);
  // setting the gyroscope full scale range to +/-500 deg/s
  IMU.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
  // setting DLPF bandwidth to 20 Hz
  IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
  // setting SRD to 19 for a 50 Hz update rate
  IMU.setSrd(19);
}

void loopIMU() {
  char buf[128];  
  //float pitch,roll;
  // read the sensor
  IMU.readSensor();

  //calculate pitch/roll
  float x = IMU.getAccelX_mss();
  float y = IMU.getAccelY_mss();
  float z = IMU.getAccelZ_mss();
  pitch = atan(x / sqrt((y * y) + (z * z)));
  roll = atan(y / sqrt((x * x) + (z * z)));
  //convert radians into degrees
  pitch = pitch * (180.0 / 3.14159);
  roll = roll * (180.0 / 3.14159) ;

  sprintf(buf, "%+6.3f %+6.3f %+6.3f, %+6.3f %+6.3f %+6.3f, %+6.3f %+6.3f %+6.3f, %5.2f°C \tpitch:%+6.2f \troll:%+6.2f\n",
  IMU.getAccelX_mss(),IMU.getAccelY_mss(),IMU.getAccelZ_mss(),
  IMU.getGyroX_rads(),IMU.getGyroY_rads(),IMU.getGyroZ_rads(),
  IMU.getMagX_uT(),IMU.getMagY_uT(),IMU.getMagZ_uT(),
  IMU.getTemperature_C(), pitch, roll
  );  
  Serial.print(buf);
  /*
  // display the data
  Serial.print(IMU.getAccelX_mss(),5);
  Serial.print(" ");
  Serial.print(IMU.getAccelY_mss(),5);
  Serial.print(" ");
  Serial.print(IMU.getAccelZ_mss(),5);
  Serial.print("\t");
  Serial.print(IMU.getGyroX_rads(),5);
  Serial.print(" ");
  Serial.print(IMU.getGyroY_rads(),5);
  Serial.print(" ");
  Serial.print(IMU.getGyroZ_rads(),5);
  Serial.print("\t");
  Serial.print(IMU.getMagX_uT(),5);
  Serial.print(" ");
  Serial.print(IMU.getMagY_uT(),5);
  Serial.print(" ");
  Serial.print(IMU.getMagZ_uT(),5);
  Serial.print("\t");
  Serial.print(IMU.getTemperature_C(),2);
  Serial.println("°C");
  */
  delay(20);
}