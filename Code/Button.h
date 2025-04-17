#pragma once
#include <Arduino.h>

// Класс для кнопок
class Button {
  // Время последнего опроса опроса для исключения дребезга контактов
  long _btnTimer = 0;
  // Текущее состояние кнопки 
  bool _flag = 0;
  // Пин кнопки
  byte _pin;
public:
  // Присвоени пина в конструкторе  
  Button(byte new_pin): _pin(new_pin) {}

  // Настройка пина
  void init(){
    pinMode(_pin, INPUT_PULLUP);
  }
  // Обновление состояния кнопки и вывод информации о событиии
  int test() {
    // Чтение состояния пина
    bool btnState = !digitalRead(_pin);
    // Событие - кнопка нажата
    if (btnState && !_flag && millis() - _btnTimer > 100) {
      // Изменение текущего состояния
      _flag = true;
      // Запись текущего времени
      _btnTimer = millis();
      // Код состояния 1
      return 1;
    }
    // Событие - кнопка отпущена
    if (!btnState && _flag && millis() - _btnTimer > 100) {
      // Изменение текущего состояния
      _flag = false;
      // Запись текущего времени
      _btnTimer = millis();
      // Код состояния -1
      return -1;
    }
    return 0;
  }
  // Чтение текущего состояния
  bool Flag(){ return _flag; }
};

