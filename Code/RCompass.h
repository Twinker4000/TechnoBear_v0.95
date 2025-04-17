#pragma once
#include <Arduino.h>

#include <Wire.h>
#include <LSM303.h>


#define CompassHeading_Z \
  (LSM303::vector<int>) { \
    0, 0, 1 \
  }
#define CompassHeading_mZ \
  (LSM303::vector<int>) { \
    0, 0, -1 \
  }
#define CompassHeading_Y \
  (LSM303::vector<int>) { \
    0, 1, 0 \
  }
#define CompassHeading_mY \
  (LSM303::vector<int>) { \
    0, -1, 0 \
  }
#define CompassHeading_X \
  (LSM303::vector<int>) { \
    1, 0, 0 \
  }
#define CompassHeading_mX \
  (LSM303::vector<int>) { \
    -1, 0, 0 \
  }


class RCompass {
  LSM303 _compass;
  LSM303::vector<int16_t> _useHeading = CompassHeading_Z;
  bool _isInit = false;
  long _calibration = 0;
  LSM303::vector<int16_t> running_min = { 32767, 32767, 32767 };
  LSM303::vector<int16_t> running_max = { -32768, -32768, -32768 };
public:
  // RCompass(){}

  bool init() {

#ifdef DEBUG
    Serial.print("Init_Compass...");
#endif

    Wire.begin();
    _isInit = _compass.init();
    _compass.enableDefault();
    //      {  -773,   +727,  -1007}    max: {   +99,   +766,   -385}
    // min: {  -420,   +937,   -813}    max: {   +72,   +967,   -465}
    _compass.m_min = (LSM303::vector<int16_t>){ -1053,   +436,   +118};
    _compass.m_max = (LSM303::vector<int16_t>){  -369,   +526,   +591};

#ifdef DEBUG
    if (_isInit) Serial.println("OK");
    else Serial.println("ERROR");
#endif
    return _isInit;
  }

  int AzimutCount(byte n) {
    if (!_isInit) return 999;
    float azimut = 0;
    for (byte i = 0; i < n; i++) {
      _compass.read();
      azimut += _compass.heading(_useHeading);
    }
    return int(azimut / n);
  }
  int Azimut() {
    if (!_isInit) return 999;
    _compass.read();
    return int(_compass.heading(_useHeading));
  }
  void startCalibration() {
    if (!_isInit) return;
    running_min = { 32767, 32767, 32767 };
    running_max = { -32768, -32768, -32768 };
    _calibration = millis() + 10000;
  }
  bool doingCalibration() {
    if (!_isInit) return 0;
    if (_calibration == 0) return 0;
    _compass.read();

    running_min.x = min(running_min.x, _compass.m.x);
    running_min.y = min(running_min.y, _compass.m.y);
    running_min.z = min(running_min.z, _compass.m.z);

    running_max.x = max(running_max.x, _compass.m.x);
    running_max.y = max(running_max.y, _compass.m.y);
    running_max.z = max(running_max.z, _compass.m.z);

    if (millis() < _calibration) return 0;
#ifdef DEBUG
    char report[80];
    snprintf(report, sizeof(report), "min: {%+6d, %+6d, %+6d}    max: {%+6d, %+6d, %+6d}",
             running_min.x, running_min.y, running_min.z,
             running_max.x, running_max.y, running_max.z);
    Serial.println(report);
#endif
    _compass.m_min = running_min;
    _compass.m_max = running_max;
    _calibration = 0;
    return 1;
  }

  /*

Значения функции Degree:
       90
        y    
        |
180 ----|----x 0
        |
       270
    
*/
  float Degree(float x, float y) {
    float a = degrees(asin(y / sqrt(x * x + y * y)));
    if (x <= 0) a = 180 - a;
    else if (y < 0) a += 360;
    return a;
  }

  int getAzimut() {
    if (!_isInit) return 999;
    _compass.read();

    float X = _compass.m.x - (_compass.m_min.x + _compass.m_max.x) / 2;
    float Y = _compass.m.y - (_compass.m_min.y + _compass.m_max.y) / 2;
    float Z = _compass.m.z - (_compass.m_min.z + _compass.m_max.z) / 2;
    return int(Degree(Z, -X));
  }

  int getAzimut2() {
    if (!_isInit) return 999;
    _compass.read();

    float X = float(_compass.m.x+156) / 2.3;
    float Z = float(_compass.m.z+644) / 1.6;
    // if (sqrt(X * X + Z * Z) > 130) return 888;
    // if (sqrt(X * X + Z * Z) < 80) return 777;
    int deg = int(Degree(Z, -X));
    int delta =-(deg/10-7)*(deg/10-34)/20;
    if (delta > 0) deg += delta;
    return deg;
  }
};