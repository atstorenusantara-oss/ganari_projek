#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

struct SensorData {
  int co2;
  float o2;
  int pm25;
  float temp;
  float VOC;
};

void updateSensorDataRandom(SensorData& data);

#endif
