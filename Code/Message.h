#pragma once
#include <Arduino.h>

// Максимальная длинны сообщения
#define Message_Max_Len 20

struct Message {
  // Номер группы получатля
  byte group = 0;
  // Номер получателя
  byte id = 0;
  // Код команды
  byte com = 0;
  // Данные команды
  unsigned long data = 0;
  // Наличие данных
  bool is_data = false;
  // Отправитель
  char sender;
  // Массив используется для:
  // 1) для формирования в нём сообщения и отправки его канал связи
  // 2) для получения в него сообщения из канала связи и послдующего разбора
  char massive[Message_Max_Len];
  // Текущая длина сообщения в массиве
  byte len = 0;

  // Конструктор сообщения без данных
  Message(byte in_group, byte in_id, byte in_com, char in_sender)
    : group(in_group), id(in_id), com(in_com), sender(in_sender) {}

  // Конструктор сообщения с данными
  Message(byte in_group, byte in_id, byte in_com, unsigned long in_data, char in_sender)
    : group(in_group), id(in_id), com(in_com), is_data(1), data(in_data), sender(in_sender) {}

  // Конструктор сообщения с данными (в формате int)
  Message(byte in_group, byte in_id, byte in_com, int in_data, char in_sender)
    : group(in_group), id(in_id), com(in_com), is_data(1), data((unsigned long)in_data), sender(in_sender) {}

  // Добавление данных к уже созданному сообщению
  void setData(unsigned long in_data) {
    is_data = true;
    data = in_data;
  }

  // Запись данных из массива байтов
  bool setMassive(byte* buffer, byte size) {
    if ((size > 20) || (size < 5)) return 0;
    len = size;
    for (byte i = 0; i < len; i++) massive[i] = (char)buffer[i];
    return 1;
  }

  // Создание массива по переменным
  void genMassive() {
    len = 0;
    massive[len++] = '0' + (group);
    massive[len++] = '0' + (id);
    massive[len++] = '0' + (com / 10);
    massive[len++] = '0' + (com % 10);
    if (is_data) {
      String sd = String(data);
      for (byte i = 0; i < sd.length(); i++)
        massive[len++] = sd.charAt(i);
    }
    massive[len++] = sender;
  }
  
  // Разбор массива в переменные
  bool parsMassive() {
    // Проверка длины
    if (len < 5) return 0;

    // Проверка символов сообщения
    for (byte i = 0; i < len; i++) {
      if ((i != len - 1) && (!is_digit(massive[i]))) return 0;
      if ((i == len - 1) && (massive[i] != '*') && (massive[i] != '#')) return 0;
    }
    // Разбор
    group = toInt(massive[0]);
    id = toInt(massive[1]);
    com = toInt(massive[2]) * 10 + toInt(massive[3]);
    // Разбор данных команды
    is_data = false;
    if ((massive[4] != '*') && (massive[4] != '#')) {
      is_data = true;
      data = toInt(massive[4]);
      for (byte i = 5; i < len; i++) {
        if ((massive[i] == '*') || (massive[i] == '#')) {
          break;
        } else data = data * 10 + toInt(massive[i]);
      }
    }
    sender=massive[len-1];
    return 1;
  }

  // Проверка символа
  int is_digit(char ch) {
    return ('0' <= ch && ch <= '9');
  }
  // Преобразованя символа в цифру
  int toInt(char ch) {
    return (ch - 48);
  }
};