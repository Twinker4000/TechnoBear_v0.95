#pragma once
#include <Arduino.h>

class Buzzer {
  byte _pin;
  byte _count=0;
  bool _status=0;
  long _timer=0;
public:
  Buzzer(byte new_pin)
    : _pin(new_pin) {}

  void init() {
    pinMode(_pin, OUTPUT);
    randomSeed(analogRead(0));
  }
  void addCount(int n = 1){
    _count += n;
  }

  const byte time_tick = 50;
  void tick(){
    if (!_count)return;

    if (_timer==0){
      _timer=millis()+time_tick;
      tone(_pin,2000);
      // digitalWrite(_pin, 1);
      _status=1;
      return;
    }

    if ((millis() > _timer) && _status){
      _timer=millis()+time_tick;
      noTone(_pin);
      // digitalWrite(_pin, 0);
      _status=0;
      return;
    }

    if ((millis() > _timer) && !_status){
      _timer=0;
      _count--;
    }


  }

  void time(long t) {
    if (t <= 0) return;
    // digitalWrite(_pin, 1);
    tone(_pin,500);
    delay(t);
    // digitalWrite(_pin, 0);
    noTone(_pin);
    delay(t);
  }

  void timeCount(long t, int c) {
    if ((t <= 0) || (c <= 0)) return;
    for (int i = 0; i < c; i++) time(t);
  }

  void r2d2() {
    int K = 2000;
    switch (random(1, 7)) {

      case 1: phrase1(); break;
      case 2: phrase2(); break;
      case 3:
        phrase1();
        phrase2();
        break;
      case 4:
        phrase1();
        phrase2();
        phrase1();
        break;
      case 5:
        phrase1();
        phrase2();
        phrase1();
        phrase2();
        phrase1();
        break;
      case 6:
        phrase2();
        phrase1();
        phrase2();
        break;
    }
    for (int i = 0; i <= random(3, 9); i++) {

      tone(_pin, K + random(-1700, 2000));
      delay(random(70, 170));
      noTone(_pin);
      delay(random(0, 30));
    }
    noTone(_pin);
  }

  void phrase1() {

    int k = random(1000, 2000);
    for (int i = 0; i <= random(100, 2000); i++) {

      tone(_pin, k + (-i * 2));
      delay(random(.9, 2));
    }
    for (int i = 0; i <= random(100, 1000); i++) {

      tone(_pin, k + (i * 10));
      delay(random(.9, 2));
    }
  }

  void phrase2() {

    int k = random(1000, 2000);
    for (int i = 0; i <= random(100, 2000); i++) {

      tone(_pin, k + (i * 2));
      delay(random(.9, 2));
    }
    for (int i = 0; i <= random(100, 1000); i++) {

      tone(_pin, k + (-i * 10));
      delay(random(.9, 2));
    }
  }
};
