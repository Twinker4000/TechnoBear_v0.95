#pragma once
#include <Arduino.h>

// Подключение класса для работы с соообщениями
#include "Message.h"

// Класс для приёма и передачи сообщений по Serial 
class RSerial {
  // Массив-буффер для сохраниения символов входящих в сообщение из потока
  char _buf[20];
  // Длинна последовательности символов в буфере
  int _size = 0;
public:
  // Проверка потока Serial на наличие данных
  bool test() {
    return Serial.available();
  }
  // Отправка сообщения в Serial
  void send(Message* outputMessage) {
    // Формирование массива по переменным
    outputMessage->genMassive();
    // Отправка массива в Serial
    Serial.write(outputMessage->massive, outputMessage->len);
    Serial.println();
  }
  // Попытка чтения символа из Serial 
  bool read(Message* inputMessage) {
    // Проверка на наличие данных
    if (!test()) return 0;
    // Проверка символа и добавление в буфер
    if (addChar(Serial.read())) {
      // Копирование буфера в массив сообщения
      memcpy(inputMessage->massive, _buf, _size);
      inputMessage->len = _size;
      _size = 0;
      // Разбор сообщения по переменным
      return inputMessage->parsMassive();
    }
    return 0;
  }
  bool addChar(char ch) {
    // Ограничение на макс длину сообщения
    if (_size > 20) _size = 0;
    // Проверка на соответствие протоколу
    if (is_digit(ch)) _buf[_size++] = ch;
    else if ((ch == '*') || (ch == '#')) {
      _buf[_size++] = ch;
      // Проверка на минимальную длинну 
      if (_size > 4) return 1;
      _size = 0;
    } else _size = 0;

    return 0;
  }
  // Проверка символа
  int is_digit(char ch) {
    if ('0' <= ch && ch <= '9') return 1;
    return 0;
  }
};