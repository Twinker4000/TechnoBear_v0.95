#pragma once
#include <Arduino.h>

#define USE_VL53LOX

#include <Wire.h>
#include <VL53L0X.h>


class RL0X {
  VL53L0X sensor;
  // Состояние дальномера
  bool is_init = false;
public:
  // Настройка 
  void init() {
#ifdef DEBUG
    Serial.print("Init_VL53L0X...");
#endif

    Wire.begin();
    sensor.setTimeout(500);
    is_init = sensor.init();

#ifdef DEBUG
    if (is_init) Serial.println("OK");
    else Serial.println("ERROR");
#endif
  }

  // Запрос измерения дистанции
  int dist() {
    // Проверка состояния
    if (!is_init) return 0;
    int dist = sensor.readRangeSingleMillimeters();
    if (sensor.timeoutOccurred()) return 0;
    if (dist > 8000) return 0;
    return dist;
  }
};