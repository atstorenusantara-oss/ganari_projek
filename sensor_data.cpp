#include <Arduino.h>
#include "sensor_data.h"

void updateSensorDataRandom(SensorData& data) {
  data.co2 = random(400, 1200);
  data.o2 = random(195, 210) / 10.0;
  data.pm25 = random(5, 80);
  data.temp = random(220, 320) / 10.0;
  data.VOC = random(220, 320) / 10.0;
}
