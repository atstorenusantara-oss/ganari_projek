#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

struct SensorData {
  int co2;
  int voc;
  float humidity;
  int pm25;
  float temp;
};

void initSensorReader();
bool updateSensorDataFromSensor(SensorData& data);

#endif
