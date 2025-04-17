//---------------------------------------------------------------------
// Глобальные переменные

// Номер группы и номер ромота
byte my_id_group = 0, my_id = 8;
// Резрешение на смену номера робота и группы
// Выдаётся на следующую команду после нажатия кнопки
bool ToChangeID = false;

//---------------------------------------------------------------------
// Разкомментировать для вывода отладочной информации
// #define DEBUG

#include "PinDefine.h"

// Подключение класса для работы с соообщениями
#include "Message.h"
// Подключение класса для отправки и приёма сообщений по Serial     
#include "RSerial.h"
RSerial R_Serial;

// Подключение класса для опроса кнопок (замукающей пин на землю)
#include "Button.h"
// Создание кнопки на пине A0
Button R_button(PIN_BUTTON);

// Подключение класса для работы с зуммером
#include "Buzzer.h"
// Создание зуммера на пине A1
Buzzer R_buzzer(PIN_BUZZER);

// Подключение файла цифрового дальномера на I2C
#include "RL0X.h"
RL0X R_lox;

#include "RCompass.h"
RCompass R_compass;
bool MotorAzimuthHold = false;
char HoldingMove = 's';
/* 
's' - stop
'f' - forward
'b' - back
'l' - left
'r' - right
*/
int HoldingAzimut = 0;
long HoldingAzimutTimer = 0;

// Подключение файла для работы с моторами и серво
#include "Motor.h"

//---------------------------------------------------------------------
// Настройка компонентов
void setup() {
  // Запуска Serial со скоростью 9600
  Serial.begin(9600);

#ifdef DEBUG
  Serial.println("setup");
#endif

  // Настройка моторов и серв
  init_Motors();
  // Настройка кнопки
  R_buzzer.init();
  // Настройка зуммера
  R_button.init();
  // Настройка дальномера
  R_lox.init();

  R_compass.init();


#ifdef DEBUG
  Serial.println("---------------");
#endif

  // Оповещение о выполнении настройки
  // R_buzzer.time(100);
  R_buzzer.addCount();
}
//---------------------------------------------------------------------
// Основная циклическая программа
// В ней постоянно опрашиваются компоненты и выполняются действия в зависимости от тригеров
void loop() {
  // Статический объект сообщения в который будут записываться принятые сообщения
  static Message M_input(0, 0, 0, '*');

  // Проверка Serial на наличие данных в буфере и их считывание
  if (R_Serial.read(&M_input)) {
    // Оповещение о успешном считывании сообщения
    // R_buzzer.time(100);
    // Добавлние в очерелдь одного писка
    // R_buzzer.addCount();
    // Исполнение команды в сообщении
    CommandDoing(&M_input);
  }

  // Проверка кнопки на нажатие
  if (R_button.test() == 1) {
    // Создание сообщения о нажатии кнопки
    Message out_M(my_id_group, my_id, (byte)11, 1, '#');
    // Отправка сообщения
    R_Serial.send(&out_M);
    // Разрешение на смену номера и группы
    ToChangeID = true;
  }

  // Работа с пищалкой по таймеру
  R_buzzer.tick();
  // Подача импульсов на моторы во время движения
  MotorsGo();
  
  if(R_compass.doingCalibration()){
    // Создание сообщения
    Message out_M(my_id_group, my_id, (byte)49, '#');
    // Отправка сообщения
    R_Serial.send(&out_M);
  }

  azimuthHold();


  // Отправка расстояния с дальномера если эта опция включена
  if (dist_after_move) {
    dist_after_move = false;
    // Формирование команды на дальномер
    Message do_M(my_id_group, my_id, (byte)12, '*');
    // Исполнение команды
    CommandDoing(&do_M);
  }
}

//---------------------------------------------------------------------

//Выбор действий в зависимости от входных данных
void CommandDoing(Message *M_input) {
  // Вывод информации о команде во время отладки
#ifdef DEBUG
  String s = String(M_input->group) + ":" + String(M_input->id) + " " + String(M_input->com);
  if (M_input->is_data) s += " " + String(M_input->data);
  s += " " + String(M_input->sender);
  Serial.println(s);
#endif

  // Проверка отправителя 
  if (M_input->sender != '*') return;
  // Проверка получатля
  if (M_input->group != my_id_group || M_input->id != my_id)
    if (M_input->group != 0 || M_input->id != 0)
      if (M_input->group != my_id_group || M_input->id != 0)
        return;


  // Оповищение о начале исполнения команды
  // R_buzzer.time(100);
  // Добавлние в очерелдь одного писка
  // R_buzzer.addCount();

  if (MotorAzimuthHold){
    if ((M_input->com < 10) || (M_input->com >= 30 && M_input->com < 40)){
      MotorAzimuthHold=false;
      if(M1.isActive() && M1.isActive()){
        M1.STOP();
        M2.STOP();
      }
    }
  }

  // Выбор действия
  switch (M_input->com) {
    //-----------------------------------------------------
    // Команды моторов и серв

    // 00 - полная остановка
    case 0:
      M1.STOP();
      M2.STOP();
      MotorAzimuthHold=false;
      break;
    // 01 - первый мотор вперёд
    case 1:
      M1.Move(M_input->data);
      break;
    // 02 - первый мотор назад
    case 2:
      M1.Move(-M_input->data);
      break;
    // 03 - второй мотор вперёд
    case 3:
      M2.Move(M_input->data);
      break;
    // 04 - второй мотор назад
    case 4:
      M2.Move(-M_input->data);
      break;
    // 05 - поворот 1 сервы на угол
    case 5:
      S1.write(M_input->data);
      break;
    // 06 - поворот 2 сервы на угол
    case 6:
      S2.write(M_input->data);
      break;

    //-----------------------------------------------------
    // Команды датчиков

    // 11 - запрос состояния кнопки
    case 11:
      {
        Message out_M(my_id_group, my_id, (byte)11, (int)R_button.Flag(), '#');
        R_Serial.send(&out_M);
      }
      break;
    // 12 - запрос измерения дистанции дальномером
    case 12:
      {
        Message out_M(my_id_group, my_id, (byte)12, R_lox.dist(), '#');
        R_Serial.send(&out_M);
      }
      break;

    case 14:
      {
        Message out_M(my_id_group, my_id, (byte)14, R_compass.getAzimut(), '#');
        // Message out_M(my_id_group, my_id, (byte)14, R_compass.getAzimut2(), '#');
        R_Serial.send(&out_M);
      }
      break;
    //-----------------------------------------------------
    // Косплексные команды

    // 30 - остановка робота
    case 30:
      M1.STOP();
      M2.STOP();
      MotorAzimuthHold=false;
      break;
    // 31 - движение робота вперёд
    case 31:
      MotorsMove(M_input->data, M_input->data);
      Move_dist = DIST;
      break;
    // 32 - движение робота назад
    case 32:
      MotorsMove(-M_input->data, -M_input->data);
      Move_dist = DIST;
      break;
    // 33 - поворот робота вокруг своей оси влево (против часовой стрелки)
    case 33:
      MotorsMove(-M_input->data, M_input->data);
      Move_dist = DIST;
      break;
    // 33 - поворот робота вокруг своей оси влево (по часовой стрелке)
    case 34:
      MotorsMove(M_input->data, -M_input->data);
      Move_dist = DIST;
      break;
    // 35 - поворот вправо левым колесом, вокруг оси правого колеса
    case 35:
      MotorsMove(M_input->data, 0);
      Move_dist = DIST;
      break;
    // 36 - поворот влево левым колесом, вокруг оси правого колеса
    case 36:
      MotorsMove(-M_input->data, 0);
      Move_dist = DIST;
      break;
    // 37 - поворот влево правым колесом, вокруг оси левого колеса
    case 37:
      MotorsMove(0, M_input->data);
      Move_dist = DIST;
      break;
    // 38 - поворот вправо правым колесом, вокруг оси левого колеса
    case 38:
      MotorsMove(0, -M_input->data);
      Move_dist = DIST;
      break;
    // 39 - поворот башни и последуещее измерение дистанции
    case 39:
      S1.write(M_input->data);
      // Пауза для ожидания поворота сервы
      delay(1000);
      {
        Message out_M(my_id_group, my_id, 12, R_lox.dist(), '#');
        R_Serial.send(&out_M);
      }
      break;

    //-----------------------------------------------------
    case 40:
      MotorAzimuthHold = true;
      if (M_input->is_data) {
        M_input->data %= 360;
        HoldingAzimut = M_input->data;
      //   // int dist = HoldingAzimut-R_compass.getAzimut();
      //   // if (dist < -180) dist += 360;
      //   // if (dist > 180) dist -= 360;

      //   // HoldingAzimutTimer = millis()+8*abs(dist)+500;
      }else
      // HoldingAzimut = R_compass.getAzimut2();
      HoldingAzimut = R_compass.getAzimut();
      
      HoldingMove = 's';
      break;
    case 41:
      MotorAzimuthHold = true;
      HoldingMove = 'f';
      // HoldingAzimut = R_compass.getAzimut2();
      HoldingAzimut = R_compass.getAzimut();
      MotorsMove(M_input->data, M_input->data);
      // HoldingAzimutTimer = millis()+8*M_input->data+200;
      break;
    case 42:
      MotorAzimuthHold = true;
      HoldingMove = 'b';
      // HoldingAzimut = R_compass.getAzimut2();
      HoldingAzimut = R_compass.getAzimut();
      MotorsMove(-M_input->data, -M_input->data);
      // HoldingAzimutTimer = millis()+8*M_input->data+200;
      break;
    case 43:
      MotorAzimuthHold = true;
      HoldingMove = 'l';
      M_input->data %= 360;
      // HoldingAzimut = R_compass.getAzimut2();
      HoldingAzimut = R_compass.getAzimut();
      HoldingAzimut -= M_input->data;
      if (HoldingAzimut < 0) HoldingAzimut += 360;
      
      M_input->data = int(float(M_input->data) * 1.3);
      MotorsMove(-M_input->data, M_input->data);
      // HoldingAzimutTimer = millis()+10*M_input->data;
      break;

    case 44:
      MotorAzimuthHold = true;
      HoldingMove = 'r';
      M_input->data %= 360;
      // HoldingAzimut = R_compass.getAzimut2();
      HoldingAzimut = R_compass.getAzimut();
      HoldingAzimut += M_input->data;
      if (HoldingAzimut >= 360) HoldingAzimut -= 360; 

      M_input->data = int(float(M_input->data) * 1.3);
      MotorsMove(M_input->data, -M_input->data);
      // HoldingAzimutTimer = millis()+10*M_input->data;
      break;
    case 49:
      MotorAzimuthHold = false;
      R_compass.startCalibration();
      M1.STOP();
      M2.STOP();
      MotorsMove(-1200, 1200);
      break;
    //-----------------------------------------------------
    // Команды зумера

    // 70 - включение пищалки на время
    case 70:
      R_buzzer.time(M_input->data);
      break;
    // 72 - включение звуков R2D2
    case 72:
      R_buzzer.r2d2();
      break;

    //-----------------------------------------------------
    // Команды настроек

    // 81 - включение торможения после движений (по умолчанию включено)
    case 81:
      HOLD = (M_input->data > 0);
      break;
    // 82 - включение измерения дистанции комплексных движений робота (по умолчанию выключено)
    case 82:
      DIST = (M_input->data > 0);
      break;

    // 88 - смена номера робота на время текущей сессии (после перезагрузки контроллера сбрасывается до прошитого имени)
    case 88:
      if (!ToChangeID) break;
      if (M_input->is_data && M_input->data < 90) {
        my_id_group = M_input->data / 10;
        my_id = M_input->data % 10;
      }
      break;
  }
  // Резрешение на смену номера действует на одну команду
  if (ToChangeID) ToChangeID = false;
}


void azimuthHold(){
  if (!MotorAzimuthHold)return;
  if (HoldingAzimutTimer!=0 && HoldingAzimutTimer < millis()) {
    M1.STOP();
    M2.STOP();
    
    MotorAzimuthHold = false;
    HoldingAzimutTimer=0;
    return;
  }

  static long tmr = millis();
  if (millis()-tmr < 100)return;
  tmr = millis();

  // int newAzimut = R_compass.getAzimut2();
  int newAzimut = R_compass.getAzimut();

  int dist = HoldingAzimut-newAzimut;
  if (dist < -180) dist += 360;
  if (dist > 180) dist -= 360;
  if (abs(dist)<5){
    return;
  }
  
  if (HoldingMove == 's'){
    // if(HoldingAzimutTimer==0)
    //   HoldingAzimutTimer = millis()+8*abs(dist)+500;
    // dist = int(float(dist) * 1.1);
    // Serial.println(dist);
    MotorsMove(dist,-dist);
  }
  // else if (HoldingMove == 'r'){
  //   int steps = max(M1.getSteps(), M2.getSteps());
    
  //   if (dist>0) {
  //     if (abs(steps - dist) < 5)return;
  //     int delta = dist - steps;
  //     M1.AddSteps(delta);
  //     M2.AddSteps(delta);
  //   }
  //   else {
  //     if (abs(dist - steps)<5)return;
  //     M1.setSteps(0);
  //     M2.setSteps(0);
  //     MotorsMove(dist,-dist);
  //   }
  // }
  // else if (HoldingMove == 'l'){
  //   int steps = max(M1.getSteps(), M2.getSteps());
    
  //   if (dist<0) {
  //     if (abs(steps + dist) < 5)return;
  //     int delta = - dist - steps;
  //     M1.AddSteps(delta);
  //     M2.AddSteps(delta);
  //   }
  //   else {
  //     if (abs(dist + steps) < 5)return;
  //     M1.setSteps(0);
  //     M2.setSteps(0);
  //     MotorsMove(dist,-dist);
  //   }
  // }
  else if (HoldingMove == 'f'){
    M1.AddSteps(abs(dist));
    M2.AddSteps(abs(dist));
    if (dist>0) M1.PassStep(abs(dist));
    else M2.PassStep(abs(dist));
  }else if (HoldingMove == 'b'){
    M1.AddSteps(abs(dist));
    M2.AddSteps(abs(dist));
    if (dist>0) M2.PassStep(abs(dist));
    else M1.PassStep(abs(dist));
  }

}
