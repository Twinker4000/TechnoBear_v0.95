#pragma once
#include <Arduino.h>

// Подключение библиотеки для работы с серво
#include <Servo.h>

//---глобальные переменные

// Период смены фаз шагов
// Период шага в 2 раза больше
#define MotorTimer 2

bool DIST = false;             // Включение измерения дистанции после комплексных движений
bool Move_dist = false;        // Измерения дистанции после завершения движения
bool dist_after_move = false;  // Команда на измерение дистанции

bool HOLD = true;  // Включение торможения после движений
//------------------------
// MOTOR (пин включения, пин шагов, пин направления, инверсия) - класс для управления шаговикамом через драйвер
class MOTOR {
  byte _PIN_EN;    // Пин включения питания мотора
  byte _PIN_STEP;  // Пин шагов мотора
  byte _PIN_DIR;   // Пин направления вращения
  bool _Invert;    // Инверсия мотора
  bool _NEXT = 0;  // Начала нового движения без завершения текущего
  long _Steps;     // Отображает сколько шагов осталось сделать (в движении)
  bool _InStep;    // Подача импульсов на драйвер
  int _PassStep = 0;
public:
  // Конструктор с присвоением пинов и возможностью програмной инверсии мотор
  MOTOR(byte PIN_EN, byte PIN_STEP, byte PIN_DIR, bool Invert = 0)
    : _PIN_EN(PIN_EN), _PIN_STEP(PIN_STEP), _PIN_DIR(PIN_DIR), _Invert(Invert) {
    // Настройка пинов
    pinMode(_PIN_STEP, OUTPUT);
    pinMode(_PIN_DIR, OUTPUT);
    pinMode(_PIN_EN, OUTPUT);
    // Выключение питания мотора
    OFF();
  }

  // Включение питания мотора
  void ON() {
    digitalWrite(_PIN_EN, 0);
  }
  // Выключение питания мотора и заверршение движения
  void OFF() {
    _Steps = 0;
    digitalWrite(_PIN_EN, 1);
  }
  // Завершение движения с возвратом оставшегося количества шагов
  long STOP() {
    long t = _Steps;
    OFF();
    return t;
  }
  void ENDMOVE(){
    _Steps = 0;
  }
  // Запрос оставшегося количества шагов
  long getSteps() {
    return _Steps;
  }
  // Запрос состояния мотора
  bool isActive() {
    return (_Steps != 0);
  }
  // Установлние разрешения на исполнение следующей команды без завершения предыдущей
  void setNEXT(bool t) {
    _NEXT = t;
  }
  // Движение относительно текущей позиции
  bool Move(long Target) {
    if (Target == 0) return 1;
    // Проверка на незавершённость предыдущей команды
    if (isActive() && (!_NEXT)) return 0;
    // Включение питания
    ON();
    // Выброр направления вращения
    bool new_dir = (Target > 0) ? _Invert : (!_Invert);
    digitalWrite(_PIN_DIR, new_dir);
    // Установка требуемого количества шагов
    _Steps = abs(Target);
    // Установка фазы шага
    _InStep = 0;
    return 1;
  }
  // Данная функция должна вызывается с фиксированной частотой для смены фаз шагов
  // Частота работы шаговика будет в 2 раза меньше частоты вызова функции
  void Go() {
    if (_Steps == 0) return;
    if (_Steps > 0) {
      _InStep = !_InStep;
      if (!_PassStep)digitalWrite(_PIN_STEP, _InStep);
      if (!_InStep) {
        _Steps--;
        if(_PassStep)_PassStep--;
      }
    }
  }

  void GoOne(bool dir) {
    ON();
    bool new_dir = dir ? _Invert : (!_Invert);
    digitalWrite(_PIN_DIR, new_dir);
    for (byte i = 0; i < 2; i++) {
      delay(MotorTimer);
      _InStep = !_InStep;
      digitalWrite(_PIN_STEP, _InStep);
    }
    delay(MotorTimer);
    if (!isActive())OFF();
  }

  void PassStep(int n){
    _PassStep = n;
  }

  void AddSteps(int n){
    _Steps += n;
  }
  void setSteps(int n){
    _Steps = n;
  }
};


MOTOR M1(PIN_en, PIN_Xstep, PIN_Xdir, 0);  //Подключаем правый мотор
MOTOR M2(PIN_en, PIN_Zstep, PIN_Zdir, 1);  //Подключаем левый мотор
Servo S1;
Servo S2;

// Настройка
void init_Motors() {
  M1.OFF();
  M2.OFF();
  S1.attach(PIN_SERVO1);
  S2.attach(PIN_SERVO2);
}



void MotorsGo() {
  if ((!M1.isActive()) && (!M2.isActive())) return;
  // Время последней смены фаз
  static long last_millis = 0;
  // Смена фаз с определённым периодом
  if ((last_millis == 0) || (millis() - last_millis > MotorTimer)) {
    M1.Go();
    M2.Go();
    last_millis = millis();
  }
  // При достижении конечной точки обоими моторами
  if ((!M1.isActive()) && (!M2.isActive())) {
    last_millis = 0;
    // Пауза с включёнными моторами позволет погасить инерцию после движения (торможение)
    if (HOLD == true && !MotorAzimuthHold) delay(300);
    if (MotorAzimuthHold) delay(100);
    // Выключение питания моторов
    M1.OFF();
    M2.OFF();

    HoldingMove = 's';
    // Выполнение инструкций для последующего измерения дистанции
    if (Move_dist) {
      Move_dist = false;
      dist_after_move = true;
    }
  }
}

// Совместная отправка команды на оба мотора
void MotorsMove(long l, long r) {
  M1.Move(r);
  M2.Move(l);
}
