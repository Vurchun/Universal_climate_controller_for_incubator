#include <DS1307new.h>
#include <L298N.h>
#include <DHT.h>
#include <DHT_U.h>
#include <EEPROM.h>
#include <FastIO.h>
#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SR.h>
#include <LiquidCrystal_SR2W.h>
#include <LiquidCrystal_SR3W.h>
#include <Wire.h>
#include <OneWire.h>
#define EEPROM_ADDR 0x50
//#define DS18S20_ID 0x10
//#define DS18B20_ID 0x28
#define DHTPIN 12                                                // датчик температуры и влажности DHT22 на 12 цифровом входе
#define DHTTYPE DHT22                                            // выбор типа датчика DHT 11 
DHT dht(DHTPIN, DHTTYPE);
#include <DallasTemperature.h>                                   // Подключаем библиотеку с функциями для работы с DS18B20 (запросы, считывание и преобразование возвращаемых данных)
#define ONE_WIRE_BUS 11                                          // Указываем пин подключения data-вывода датчика температуры
OneWire oneWire(ONE_WIRE_BUS);                                   // Сообщаем библиотеке об устройстве, работающем по протоколу 1-Wire
DallasTemperature sensors(&oneWire);                             // инициализация DS18b20 на 13 входе   
#include <PID_v1.h>
                  
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  //LiquidCrystal lcd(11, 10, 9, 8, 7, 6);   инициализация входов на дисплей 16Х2
int BS, BL, BankSave;                                                 // Переменые для сохранения настроек 
double TempIncubations = 37.7, deltaTIncubations = 0.2;               // температура инкубации, дельта Т
#define maxdeltaTIncubations 2                                        // максимальное значение дельтаТ для меню
#define mindeltaTIncubations 0.1                                      // минимальное  значение дельтаТ для меню
double maxTempFanStart = 37.9;                                        // максимально допустимая температура при которой включается аварийная продувка
#define maxTempFanStartMenuMin 16                                     // максимальное значение температуры работы вентилятора для меню
#define maxTempFanStartMenuMax 61                                     // минимальное значение температуры работы вентилятора для меню

int TimeFanWork = 60;                                                 // время проветривания инкубатора от СО2 в секундах
#define maxTimeFanWork 900                                           // максимальное время проветривания инкубатора от СО2 для меню в секундах
#define minTimeFanWork 5                                              // минимальное время проветривания инкубатора от СО2 для меню в секундах
#define FadeTimeFanWork 1                                             // шаг изменения значения времени работы вентилятора в меню в секундах
int TimeIntervalFanWork = 30;                                         // интервал между включениями проветривания инкубатора от СО2 в минутах
#define maxTimeIntervalFanWork 300                                    // максимальный интервал между включениями проветривания инкубатора от СО2 для меню в минутах
#define minTimeIntervalFanWork 1                                      // минимальный интервал между включениями проветривания инкубатора от СО2 для меню в минутах
#define FadeTimeIntervalFanWork 1                                     // шаг изменения значения интервала между включениями проветривания инкубатора от СО2 в меню в минутах
int FanWorkFlag = 1;                                                  // флаг активности продувки инкубатора от СО2

#define maxTempIncubationsDanger 40                                   // максимальная температура инкубации для меню
#define minTempIncubationsDanger 35                                   // минимальная температура инкубации для меню
#define FadeAmountTemp 0.1                                            // шаг изменения тепертуры в меню
int HumiditiIncubation = 60, deltaHumiditiIncubation = 1;             // влажность инкубации дельта влажности
#define mindeltaHum 1                                                 // минимальная дельта влажности для меню
#define maxdeltaHum 10                                                // максимальная дельта влажности для меню
#define FadeAmountdeltaHum 1                                         // шаг изминения дельты влажности для меню
#define FadeAmountHum 1                                               // шаг изменения влажности в меню
#define MaximumHumiditiIncubation 70                                  // максимальное значение влажности для меню
#define MinimumHumiditiIncubation 10                                 // минимальное значение влажности для меню
int KHumiditiIncubation = 0.45,DecreaseDayHumiditiIncubation = 10,IncreaseDayHumiditiIncubation = 16;
int DDHI,IDHI;
#define MaximumDecreaseDayHumiditiIncubation 255  
#define MinimumDecreaseDayHumiditiIncubation 1 
#define MaximumIncreaseDayHumiditiIncubation 255  
#define MinimumIncreaseDayHumiditiIncubation 1
#define MaximumKHumiditiIncubation 100  
#define MinimumKHumiditiIncubation 1 
int TimeRotations = 24;                                               // колличество переворотов яиц в инкубаторе в сутки
const int RotationPerDay[9] = {0, 1 , 2 , 3 , 4 , 6 , 8 , 12 , 24 }; // колличество возможных переворотов яиц в сутки
int i = 0; int k = 0;
double Tnow;                                                          // Реальная температура на данный момент в инкубаторе на DS18b20
int hum;                                                              // Реальная влажность на данный момент в инкубаторе на DHT22
int MainMenu = 0, SubMenu = 0, FlagMenu = 0;                          // переменные для управления меню
int Rotate = 1;                                                       // направолние вращения двигателя (1-вправо, 0-влево)
int tf = 0;
int NOWyear, NOWmonth, NOWday, NOWhour, NOWminute, NOWsecond;
int Setyear, Setmonth, Setday, Sethour, Setminute, Setsecond;
bool TIFlagd = 0, TIFlagf = 0;                                         // флаг индикации таймера инкубации 0 - таймер не активен, 1 - таймер активен
int TRyear, TRmonth, TRday, TRhour, TRminute, TRsecond;               // переменные в которых будет сохранена дата начала инкубации
int bank = 0;                                                         // номер банка записи и загрузки настроек инкубации
int FlagTempFan;
int FlagTimeFan = 0;    // 
//Define Variables we'll be connecting to
double Output;
//Define the aggressive and conservative Tuning Parameters
double consKp=1, consKi=0.05, consKd=0.25;
PID myPID(&Tnow, &Output, &TempIncubations, consKp, consKi, consKd, DIRECT);  //Specify the links and initial tuning parameters

L298N motor(6,8,7,8,7,6);

uint8_t strelka_vverh_vniz[8] = { B00100, B01110, B11111, B00000, B11111, B01110, B00100 };     // закодирована в двоичной системе СТРЕЛКА ВВЕРХ ВНИЗ 
uint8_t temp_cel[8]           = { B11000, B11000, B00110, B01001, B01000, B01001, B00110 };     // закодирована в двоичной системе ГРАДУС ЦЕЛЬСИЯ
uint8_t temp_del[8]           = { B00000, B00100, B00100, B01010, B01010, B10001, B11111 };     // закодирована в двоичной системе ДЕЛЬТА
uint8_t   Hot_ON[8]           = { B01111, B10000, B00111, B00001, B01110, B10000, B01111 };     // закодирована в двоичной системе ОБОГРЕВ
uint8_t   Fan_ON[8]           = { B11011, B11011, B11011, B00100, B00100, B11011, B11011 };     // закодирована в двоичной системе ОБДУВ
uint8_t   Hum_ON[8]           = { B00100, B00100, B01110, B11011, B10111, B10111, B01110 };     // закодирована в двоичной системе УВЛАЖНЕНИЕ                             
uint8_t   WiFi_ON[8]          = { B00000, B11111, B01110, B10101, B01110, B00100, B00000 };     // закодирована в двоичной системе WiFi                             



int PinHot = 2;                            // первоначальное подключение обогрева на реле №1 
int PinFan = 3;                             // первоначальное подключениe обдувa на реле №2
int PinHum = 4;                             // первоначальное подключение увлажнителя на реле №3

float voltage ;            // напряжение на аккумуляторе
int netpower ;           // наличие сети
int DayInc ;
  
 
#define PinButtons 17                       // кнопки меню на A3 аналоговом входе
#define PinButtonsLimit1 16                 // конечник 1 лотка яиц на A1 аналоговом входе
#define PinButtonsLimit2 15                 // конечник 2 лотка яиц на A2 аналоговом входе

int m;                                   // значения отображения информации на табле
int buttons_Menu;             // значения кнопок меню 
int           buttons_Limit_Swith;
unsigned char buttons_Limit_Swith1;
unsigned char buttons_Limit_Swith2;
int           PressingButtons;

unsigned long currentTime;
unsigned long loopTime;

unsigned long currentMillis;
unsigned long StartMillis = 0 ;                                              //счетчик прошедшего времени для AutoStartMenu
#define interval 15000                                                 //задержка автовозврата к StartMenu 7сек
unsigned long ds18b20readMillis = 0 ;                                           //счетчик прошедшего времени для интервала чтения с датчика температуры
#define ds18b20interval 100                                          //опрос датчика температуры каждую 1 сек
unsigned long dht22readMillis ;                                              //счетчик прошедшего времени для интервала чтения с датчика влажности
#define dht22interval 100                                            //опрос датчика влажности каждые 1 сек
unsigned long MenuRewriteMillis = 0 ;                                          //счетчик прошедшего времени для обновления главного меню
#define MenuRewriteinterval 500                                        //обновление главного меню через 0,5сек
unsigned long espwriteMillis = 0;
unsigned long TimeFaning = 0;                                                    //счетчик прошедшего времени для продувки инкубатора от СО2
unsigned long TimeIntervalFaning;                                          //счетчик прошедшего времени для интервала между продувками инкубатора от СО2
unsigned long TimeFaningInterval = TimeFanWork * 1000;                 //длительность работы вентилятора при продувке инкубатора от СО2 
unsigned long TimeIntervalFaningInterval = TimeIntervalFanWork * 60000;//длительность интервала между продувками инкубатора от СО2

void setup()
{
  lcd.begin(20, 4);
  Wire.begin();
  Serial.begin(115200);             // Запускаем вывод данных на серийный порт ESP8266
  sensors.begin();                // Запускаем библиотеку измерения температуры
  analogReference(EXTERNAL);        // Команда для сравнение напржения с входом AREF
  myPID.SetMode(AUTOMATIC);

  loopTime = currentTime;
  pinMode(PinButtons, INPUT);         // кнопки меню на 1 аналоговом входе (подключаем подтягивающий внешний резистор 2kOm)
  pinMode(PinButtonsLimit2, INPUT);         // конечник 1 на 2 аналоговом входе (подключаем подтягивающий внешний резистор 2kOm)
  pinMode(PinButtonsLimit1, INPUT);         // конечник 2 на 3 аналоговом входе (подключаем подтягивающий внешний резистор 2kOm)

  pinMode(2, OUTPUT);                       // Реле №1 на 2 выходе
  pinMode(3, OUTPUT);                       // Реле №2 на 3 выходе
  pinMode(4, OUTPUT);                       // Реле №3 на 4 выходе
  pinMode(5, OUTPUT);                       // Реле №4 на 5 выходе
  digitalWrite(2, HIGH);  digitalWrite(3, HIGH);  digitalWrite(4, HIGH);  digitalWrite(5, HIGH); // первоначальное отключение всех устройств при загрузке с паузой в 0,2сек чтобы не было скачка падения напряжения на БП
                                                   // --------------- чтение из EEPROM привязки номера исполнительного устройства к типу регулируемой велечины
  PinHot = 2;       //  настройки пина обогрева
  PinFan = 3;       //  настройки пина обдува
  PinHum = 4;       //  настройки пина увлажнения
                          // --------------- чтение из EEPROM привязки номера исполнительного устройства к типу регулируемой велечины



                          // --------------- чтение из EEPROM установок настройки инкубатора сохраненных в банк настроек №0
  byte lowByte = EEPROM.read(0); delay(100);                                            // чтение lowByte температуры инкубации *100 из ячейки "0"
  byte higtByte = EEPROM.read(1); delay(100);                                            // чтение higtByte температуры инкубации *100 из ячейки "1"
  TempIncubations = (((lowByte << 0) & 0xFF) + ((higtByte << 8) & 0xFF00)) / 100.00;
  lowByte = EEPROM.read(2); delay(100);                                                 // чтение lowByte дельтаТ температуры инкубации *100 из ячейки "2"
  higtByte = EEPROM.read(3); delay(100);                                                 // чтение higtByte дельтаТ температуры инкубации *100 из ячейки "3"
  deltaTIncubations = (((lowByte << 0) & 0xFF) + ((higtByte << 8) & 0xFF00)) / 100.00;
  HumiditiIncubation = EEPROM.read(4); delay(100);                                       // чтение дельты влажности инкубации из ячейки "4"
  deltaHumiditiIncubation = EEPROM.read(5);  delay(100);                                 // чтение дельты влажности инкубации из ячейки "5"
  TimeRotations = EEPROM.read(6);   delay(100);                                          // чтение времени через которое происходит переворот яиц из ячейки "6"
  lowByte = EEPROM.read(7);  delay(100);                                                // чтение lowByte температуры продувки *100 из ячейки "7"
  higtByte = EEPROM.read(8);  delay(100);                                                // чтение higtByte температуры продувки *100 из ячейки "8"
  maxTempFanStart = (((lowByte << 0) & 0xFF) + ((higtByte << 8) & 0xFF00)) / 100.00;
  lowByte = EEPROM.read(9);  delay(100);                                                // чтение lowByte времени вентиляции инкубатора от СО2 из ячейки "9"
  higtByte = EEPROM.read(10); delay(100);                                                // чтение higtByte времени вентиляции инкубатора от СО2 из ячейки "10"
  TimeFanWork = ((lowByte << 0) & 0xFF) + ((higtByte << 8) & 0xFF00);
  lowByte = EEPROM.read(11); delay(100);                                                // чтение lowByte интервала между вентиляциями инкубатора от СО2 из ячейки "11"
  higtByte = EEPROM.read(12); delay(100);                                                // чтение higtByte интервала между вентиляциями инкубатора от СО2 из ячейки "12"
  TimeIntervalFanWork = ((lowByte << 0) & 0xFF) + ((higtByte << 8) & 0xFF00);
  FanWorkFlag = EEPROM.read(13); delay(100);                                             // чтение флага активностивентиляциями инкубатора от СО2 из ячейки "13"
  TimeFaningInterval = TimeFanWork * 1000;                                                 //чтение длительности работы вентилятора при продувке инкубатора от СО2 
  TimeIntervalFaningInterval = TimeIntervalFanWork * 60000;                                //чтение длительности интервала между продувками инкубатора от СО2
  KHumiditiIncubation = EEPROM.read(14); delay(100);                                      //чтение коеффициента изменения влажности
  DecreaseDayHumiditiIncubation = EEPROM.read(15); delay(100);                          //чтение дня понижения  влажности
  IncreaseDayHumiditiIncubation = EEPROM.read(16); delay(100);                              //чтение дня повишения влажности
  DayInc = EEPROM.read(130); delay(100);                                               //чтение дней инкубации
  consKp = EEPROM.read(131); delay(100);                                            //чтение Пропорционального коефициента             
  consKi = EEPROM.read(132); delay(100);                                            //чтение Интегрального коефициента                 
  consKd = EEPROM.read(133); delay(100);                                             //чтение Дефиринциального коефициента      
  

           
                                               // --------------- чтение из EEPROM установок настройки инкубатора сохраненных в банк настроек №1

                                               // --------------- чтение из EEPROM установок и времени начала инкубации таймера инкубации
  TRyear = EEPROM.read(120) + 2000; delay(100);                                         // чтение года начала инкубации 
  TRmonth = EEPROM.read(121); delay(100);                                                // чтение месяца начала инкубации
  TRday = EEPROM.read(122); delay(100);                                                // чтение дня начала инкубации
  TRhour = EEPROM.read(123); delay(100);                                                // чтение часа начала инкубации
  TRminute = EEPROM.read(124); delay(100);                                                // чтение минуты начала инкубации
  TRsecond = EEPROM.read(125); delay(100);                                                // чтение секунды начала инкубации
  TIFlagd = EEPROM.read(126); delay(100);                                                // чтение флага ВКЛЮЧЕН (1) / ВЫКЛЮЧЕН (0)  индикации таймера инкубации
  TIFlagf = EEPROM.read(127); delay(100);                                                // чтение флага ВКЛЮЧЕН (1) / ВЫКЛЮЧЕН (0)  таймера инкубации
                                                 // --------------- чтение из EEPROM установок и времени начала инкубации таймера инкубации



                                         
  lcd.createChar(1, strelka_vverh_vniz);  lcd.createChar(3, temp_cel);  lcd.createChar(4, temp_del);  lcd.createChar(5, Hot_ON);  lcd.createChar(6, Fan_ON);  lcd.createChar(7, Hum_ON); lcd.createChar(8, WiFi_ON);

  lcd.setCursor(0, 0);  lcd.print("*-*-*-*-****-*-*-*-*");
  lcd.setCursor(0, 1);  lcd.print("   AutoInkubator   ");
  lcd.setCursor(0, 2);  lcd.print("  By_Vurchun_V-1.2  ");
  lcd.setCursor(0, 3);  lcd.print("*-*-*-*-****-*-*-*-*");
  delay(3000);
  lcd.clear();
  dht.begin();
  currentTime = millis();
  loopTime = currentTime;

}
void RASEEPROMSTimer()
{
  TRyear = RTC.year - 2000; TRmonth = RTC.month; TRday = RTC.day; TRhour = RTC.hour; TRminute = RTC.minute; TRsecond = RTC.second;
  EEPROM.write(120, TRyear); delay(100);                                             // запись года начала инкубации в ячейку 120 EEPROM
  EEPROM.write(121, TRmonth); delay(100);                                             // запись месяца начала инкубации в ячейку 121 EEPROM
  EEPROM.write(122, TRday); delay(100);                                             // запись дня начала инкубации в ячейку 122 EEPROM
  EEPROM.write(123, TRhour); delay(100);                                             // запись часа начала инкубации в ячейку 123 EEPROM
  EEPROM.write(124, TRminute); delay(100);                                             // запись минуты начала инкубации в ячейку 124 EEPROM
  EEPROM.write(125, TRsecond); delay(100);                                             // запись секунды начала инкубации в ячейку 125 EEPROM
  if(IDHI == 0) HumiditiIncubation /=KHumiditiIncubation/100; delay (100); BS = BankSave * 20 + 4;     EEPROM.write(BS, HumiditiIncubation);delay(100);IDHI=0; DDHI=0;
  IDHI=0; DDHI=0; DayInc=0; currentMillis=0;
                                                                  
}

void SaveToEEPROM(int BankSave)                                                         // запись данных во внутренний EEPROM 
{
  int TIC = int(TempIncubations * 100); byte lowByte = ((TIC >> 0) & 0xFF); byte higtByte = ((TIC >> 8) & 0xFF);
  BS = BankSave * 20 + 0;     EEPROM.write(BS, lowByte);                  delay(100);    // запись lowByte температуры инкубации * 100 в ячейку "0" банка "bank"
  BS = BankSave * 20 + 1;     EEPROM.write(BS, higtByte);                 delay(100);    // запись higtByte температуры инкубации * 100 в ячейку "1" банка "bank"
  int DTIC = int(deltaTIncubations * 100);  lowByte = ((DTIC >> 0) & 0xFF); higtByte = ((DTIC >> 8) & 0xFF);
  BS = BankSave * 20 + 2;     EEPROM.write(BS, lowByte);                  delay(100);    // запись lowByte дельтаТ температуры инкубации * 100 в ячейку "2" банка "bank"
  BS = BankSave * 20 + 3;     EEPROM.write(BS, higtByte);                 delay(100);    // запись higtByte дельтаТ температуры инкубации * 100 в ячейку "3" банка "bank"
  BS = BankSave * 20 + 4;     EEPROM.write(BS, HumiditiIncubation);       delay(100);    // запись дельты влажности инкубации в ячейку "4" банка "bank"
  BS = BankSave * 20 + 5;     EEPROM.write(BS, deltaHumiditiIncubation);  delay(100);    // запись дельты влажности инкубации в ячейку "5" банка "bank"
  BS = BankSave * 20 + 6;     EEPROM.write(BS, TimeRotations);            delay(100);    // запись времени через которое происходит переворот яиц в ячейку "6" банка "bank"
  int TICc = int(maxTempFanStart * 100); lowByte = ((TICc >> 0) & 0xFF); higtByte = ((TICc >> 8) & 0xFF);
  BS = BankSave * 20 + 7;     EEPROM.write(BS, lowByte);                  delay(100);    // запись lowByte температуры продувки * 100 в ячейку "7" банка "bank"
  BS = BankSave * 20 + 8;     EEPROM.write(BS, higtByte);                 delay(100);    // запись higtByte температуры продувки * 100 в ячейку "8" банка "bank"
  lowByte = ((TimeFanWork >> 0) & 0xFF); higtByte = ((TimeFanWork >> 8) & 0xFF);
  BS = BankSave * 20 + 9;     EEPROM.write(BS, lowByte);                  delay(100);    // запись lowByte времени вентиляции инкубатора от СО2 в ячейку "9" банка "bank"
  BS = BankSave * 20 + 10;    EEPROM.write(BS, higtByte);                 delay(100);    // запись higtByte времени вентиляции инкубатора от СО2 в ячейку "10" банка "bank"
  lowByte = ((TimeIntervalFanWork >> 0) & 0xFF); higtByte = ((TimeIntervalFanWork >> 8) & 0xFF);
  BS = BankSave * 20 + 11;    EEPROM.write(BS, lowByte);                  delay(100);    // запись lowByte интервала между вентиляциями инкубатора от СО2 в ячейку "11" банка "bank"
  BS = BankSave * 20 + 12;    EEPROM.write(BS, higtByte);                 delay(100);    // запись higtByte интервала между вентиляциями инкубатора от СО2 в ячейку "12" банка "bank"
  BS = BankSave * 20 + 13;    EEPROM.write(BS, FanWorkFlag);              delay(100);    // запись флага активностивентиляциями инкубатора от СО2 в ячейку "13" банка "bank"
  BS = BankSave * 20 + 14;  EEPROM.write(BS,KHumiditiIncubation); delay(100);            // запись Коефицыента влажности инкубатора в ячейку "14" банка "bank"
  BS = BankSave * 20 + 15;  EEPROM.write(BS,DecreaseDayHumiditiIncubation); delay(100);  // запись дня понижения влажности в ячейку "15" банка "bank"
  BS = BankSave * 20 + 16;  EEPROM.write(BS,IncreaseDayHumiditiIncubation); delay(100); // запись дня повишения влажности в ячейку "16" банка "bank"
  }
void LoadFromEEPROM(int BankLoad)                                                       // загрузка данных из внутреннего EEPROM
{
  BL = BankLoad * 20 + 0; byte lowByte = EEPROM.read(BL);            delay(100);    // чтение lowByte температуры инкубации *100 из ячейки "0"
  BL = BankLoad * 20 + 1;     byte higtByte = EEPROM.read(BL);            delay(100);    // чтение higtByte температуры инкубации *100 из ячейки "1"
  TempIncubations = (((lowByte << 0) & 0xFF) + ((higtByte << 8) & 0xFF00)) / 100.00;
  BL = BankLoad * 20 + 2;     lowByte = EEPROM.read(BL);                 delay(100);    // чтение lowByte дельтаТ температуры инкубации *100 из ячейки "2"
  BL = BankLoad * 20 + 3;     higtByte = EEPROM.read(BL);                 delay(100);    // чтение higtByte дельтаТ температуры инкубации *100 из ячейки "3"
  deltaTIncubations = (((lowByte << 0) & 0xFF) + ((higtByte << 8) & 0xFF00)) / 100.00;
  BL = BankLoad * 20 + 4;     HumiditiIncubation = EEPROM.read(BL);       delay(100);    // чтение дельты влажности инкубации из ячейки "4"
  BL = BankLoad * 20 + 5;     deltaHumiditiIncubation = EEPROM.read(BL);  delay(100);    // чтение дельты влажности инкубации из ячейки "5"
  BL = BankLoad * 20 + 6;     TimeRotations = EEPROM.read(BL);            delay(100);    // чтение времени через которое происходит переворот яиц из ячейки "6"
  BL = BankLoad * 20 + 7;     lowByte = EEPROM.read(BL);                 delay(100);    // чтение lowByte температуры продувки *100 из ячейки "7"
  BL = BankLoad * 20 + 8;     higtByte = EEPROM.read(BL);                 delay(100);    // чтение higtByte температуры продувки *100 из ячейки "8"
  maxTempFanStart = (((lowByte << 0) & 0xFF) + ((higtByte << 8) & 0xFF00)) / 100.00;
  BL = BankLoad * 20 + 9;     lowByte = EEPROM.read(BL);  delay(100);                   // чтение lowByte времени вентиляции инкубатора от СО2 из ячейки "9"
  BL = BankLoad * 20 + 10;    higtByte = EEPROM.read(BL);  delay(100);                   // чтение higtByte времени вентиляции инкубатора от СО2 из ячейки "10"
  TimeFanWork = ((lowByte << 0) & 0xFF) + ((higtByte << 8) & 0xFF00);
  BL = BankLoad * 20 + 11;    lowByte = EEPROM.read(BL); delay(100);                    // чтение lowByte интервала между вентиляциями инкубатора от СО2 из ячейки "11"
  BL = BankLoad * 20 + 12;    higtByte = EEPROM.read(BL); delay(100);                    // чтение higtByte интервала между вентиляциями инкубатора от СО2 из ячейки "12"
  TimeIntervalFanWork = ((lowByte << 0) & 0xFF) + ((higtByte << 8) & 0xFF00);
  BL = BankLoad * 20 + 13;    FanWorkFlag = EEPROM.read(BL); delay(100);                 // чтение флага активностивентиляциями инкубатора от СО2 из ячейки "13"
  BL = BankLoad * 20 + 14;  KHumiditiIncubation = EEPROM.read(BL); delay(100);            // чтение Коефицыента влажности инкубатора в ячейку "14" банка "bank"
  BL = BankLoad * 20 + 15;  DecreaseDayHumiditiIncubation = EEPROM.read(BL); delay(100);  // чтение дня понижения влажности в ячейку "15" банка "bank"
  BL = BankLoad * 20 + 16;  IncreaseDayHumiditiIncubation = EEPROM.read(BL); delay(100); // чтение дня повишения влажности в ячейку "16" банка "bank" 
  TimeFaningInterval = TimeFanWork * 1000;                 //длительность работы вентилятора при продувке инкубатора от СО2 
  TimeIntervalFaningInterval = TimeIntervalFanWork * 60000;//длительность интервала между продувками инкубатора от СО2
}


void PressKeyMenu()                                                                      // Вычиление нажатия кнопок  
{
  PressingButtons = 0;
  buttons_Menu = analogRead(PinButtons);
  Serial.print("  Resistant key button module="); Serial.print(buttons_Menu); Serial.println(" ");
  if (buttons_Menu >= 0 && buttons_Menu < 50)  PressingButtons = 1;           // меню       
  else  if (buttons_Menu > 400 && buttons_Menu < 500) PressingButtons = 2;     // вверх       
  else  if (buttons_Menu > 100 && buttons_Menu < 300) PressingButtons = 3;     // вниз      
  else  if ( buttons_Menu > 600 && buttons_Menu < 700)PressingButtons = 4;      // выбор      
  else  if (buttons_Menu > 800 && buttons_Menu < 900) PressingButtons = 5;  // переворот       
  else  PressingButtons = 0; 
  delay(100);           
}

void PreSetTime()
{
  Setyear = RTC.year; Setmonth = RTC.month; Setday = RTC.day; Sethour = RTC.hour; Setminute = RTC.minute; Setsecond = RTC.second;
}
void GoSetTime(int InputX)
{
  switch (SubMenu) {
  case 1:  Setyear = InputX;    break;
  case 2:  Setmonth = InputX;   break;
  case 3:  Setday = InputX;     break;
  case 4:  Sethour = InputX;    break;
  case 5:  Setminute = InputX;  break;
  case 6:  Setsecond = InputX;  break;
  }
}
void SetTime()
{
  RTC.getTime();
  RTC.stopClock();
  RTC.fillByYMD(Setyear, Setmonth, Setday);      delay(250);
  RTC.fillByHMS(Sethour, Setminute, Setsecond);  delay(250);
  RTC.setTime();
  delay(100);
  RTC.startClock();
}
void PrintMenuWrite(int FlagM)
{
  switch (FlagM) {
  case 0:  lcd.setCursor(15, 1);  lcd.print("\1");  break;
  case 1:  lcd.setCursor(15, 1);  lcd.print("*");   break;
  }
}

int Sec, SecPer, Min, MinPer, Hou, HouPer, Dey, DeyPer, Mon, MonPer, Yer;
void TimerCalculatePrint()
{  
  if (TIFlagf == 1) {
    if (RTC.second >= TRsecond) { Sec = RTC.second - TRsecond;          SecPer = 0; }
    else { Sec = 60 + (RTC.second - TRsecond);          SecPer = 1; }
    if ((RTC.minute - SecPer) >= TRminute) { Min = RTC.minute - TRminute - SecPer; MinPer = 0; }
    else { Min = 60 + (RTC.minute - TRminute) - SecPer; MinPer = 1; }
    if ((RTC.hour - MinPer) >= TRhour) { Hou = RTC.hour - TRhour - MinPer; HouPer = 0; }
    else { Hou = 24 + (RTC.hour - TRhour) - MinPer; HouPer = 1; }
    if ((RTC.day - HouPer) >= TRday) { Dey = RTC.day - TRday - HouPer; DeyPer = 0; }
    else {
      switch (RTC.month) {
      case 2: { Dey = 31 + (RTC.day - TRday) - HouPer; DeyPer = 1; break; }
      case 3: {
        if (RTC.year % 4 == 0 && RTC.year % 100 != 0 || RTC.year % 400 == 0)
        {
          Dey = 29 + (RTC.day - TRday) - HouPer; DeyPer = 1;
        }
        else
        {
          Dey = 28 + (RTC.day - TRday) - HouPer; DeyPer = 1;
        }
        break; }
      case 4:  { Dey = 31 + (RTC.day - TRday) - HouPer; DeyPer = 1; break; }
      case 5:  { Dey = 30 + (RTC.day - TRday) - HouPer; DeyPer = 1; break; }
      case 6:  { Dey = 31 + (RTC.day - TRday) - HouPer; DeyPer = 1; break; }
      case 7:  { Dey = 30 + (RTC.day - TRday) - HouPer; DeyPer = 1; break; }
      case 8:  { Dey = 31 + (RTC.day - TRday) - HouPer; DeyPer = 1; break; }
      case 9:  { Dey = 31 + (RTC.day - TRday) - HouPer; DeyPer = 1; break; }
      case 10: { Dey = 30 + (RTC.day - TRday) - HouPer; DeyPer = 1; break; }
      case 11: { Dey = 31 + (RTC.day - TRday) - HouPer; DeyPer = 1; break; }
      case 12: { Dey = 30 + (RTC.day - TRday) - HouPer; DeyPer = 1; break; }
      case 1:  { Dey = 31 + (RTC.day - TRday) - HouPer; DeyPer = 1; break; }
      }
    }
    if ((RTC.month - DeyPer) >= TRmonth) { Mon = RTC.month - TRmonth - DeyPer; MonPer = 0; }
    else { Mon = 12 + (RTC.day - TRmonth) - DeyPer; MonPer = 1; }
    Yer = RTC.year - TRyear;
    DayInc = Dey;
  }
  }



void StartFan() // включение и отключение вентилятора и печать их состояний на дисплей 
{
  if (Tnow >= (maxTempFanStart + (deltaTIncubations / 2)))
  {
    FlagTempFan = 1;
  }
  else
  {
    if (Tnow < (maxTempFanStart - (deltaTIncubations / 2)))
    {
      FlagTempFan = 0;
    }
  }
  if (FanWorkFlag == 1) {
    switch (FlagTimeFan) {
    case 1: {
      if ((currentMillis - TimeFaning) > TimeFaningInterval) {
        FlagTimeFan = 0;
        TimeIntervalFaning = currentMillis;
      } break; }
    case 0: {
      if ((currentMillis - TimeIntervalFaning) > TimeIntervalFaningInterval) {
        FlagTimeFan = 1;
        TimeFaning = currentMillis;
      } break; }
    }
  }
  if (FlagTempFan == 1 || FlagTimeFan == 1) { digitalWrite(PinFan, LOW); lcd.setCursor(19, 2); lcd.print("\6"); }
  else { digitalWrite(PinFan, HIGH); lcd.setCursor(19, 2); lcd.print(" "); }
}
void StartHot()                                                     // включение и отключение обогрева и печать их состояний на дисплей
{
  if (Tnow < (TempIncubations - (deltaTIncubations / 2)))
  {
  myPID.SetTunings(consKp, consKi, consKd);
  myPID.Compute();
  analogWrite(PinHot, Output);
  lcd.setCursor(19, 0);
  lcd.print("\5");
  
}
  else
  {
    if (Tnow >= (TempIncubations + (deltaTIncubations / 2)))
    {
      digitalWrite(PinHot, HIGH);
      if (Tnow < (TempIncubations + (deltaTIncubations / 2)))
      {
        lcd.setCursor(19, 0);
        lcd.print(" ");
      }
    }
  }
}
void StartHum()
{ 
  if(Dey == IncreaseDayHumiditiIncubation && IDHI == 0)HumiditiIncubation *=KHumiditiIncubation/100; delay (100); IDHI=1; DDHI=0;  BS = BankSave * 20 + 4;     EEPROM.write(BS, HumiditiIncubation);       delay(100);    // запись дельты влажности инкубации в ячейку "4" банка "bank"
  if(Dey == DecreaseDayHumiditiIncubation && DDHI == 0)HumiditiIncubation /=KHumiditiIncubation/100; delay (100); IDHI=0; DDHI=1;  BS = BankSave * 20 + 4;     EEPROM.write(BS, HumiditiIncubation);       delay(100);    // запись дельты влажности инкубации в ячейку "4" банка "bank"
  if (hum < (HumiditiIncubation - (deltaHumiditiIncubation / 2)))
  {
    digitalWrite(PinHum, LOW);
    lcd.setCursor(19, 1);
    lcd.print("\7");
  
  }
  else
  {
    if (hum >(HumiditiIncubation + (deltaHumiditiIncubation / 2)))
    {
      digitalWrite(PinHum, HIGH);
      lcd.setCursor(19, 1);
      lcd.print(" ");

    }
  }
}
void TempRead()                                                     // Чтение температуры с датчика DS18b20
{
  if (currentMillis - ds18b20readMillis > ds18b20interval)
  {
    ds18b20readMillis = currentMillis;
    sensors.requestTemperatures();                 // Запрос на измерение температуры (1-й ошибочный)
    delay(700);                                   // Задержка перед поторным измерением
    sensors.requestTemperatures();                 // Запрос на измерение температуры (повторный)
    double T1 = double(sensors.getTempCByIndex(0));   // Получаем значение температуры
    delay(700);                                   // Задержка перед поторным измерением
    double T2 = double(sensors.getTempCByIndex(1));   // Получаем значение температуры
    delay(700);                                   // Задержка перед поторным измерением
    Serial.println(T1);Serial.println(T2);
    if (T1 <= (-50)) Tnow = T2;
    if (T2 <= (-50)) Tnow = T1;
     Tnow = ( T1 + T2 ) / 2;
    }
}
void HumRead_DHT22()
{
  if (currentMillis - dht22readMillis > dht22interval)
  {
    dht22readMillis = currentMillis;
    hum = dht.readHumidity();
   
  }
}
void Rotating()                                    // процедура переворота лотка
{
label_1:
  buttons_Limit_Swith1 = analogRead(PinButtonsLimit1);  buttons_Limit_Swith2 = analogRead(buttons_Limit_Swith2);  delay(100);
  if (buttons_Limit_Swith1 < 50 || buttons_Limit_Swith2 < 50) buttons_Limit_Swith = 1;  else buttons_Limit_Swith = 0; // если хотябы один из конечников замкнут то button_Limit_Swith = 1
  if (buttons_Limit_Swith == 1)
  {
    //---------------------------------------------------------------------------------------------------------------------------//
    RTC.getTime();  TempRead();  HumRead_DHT22();  StartFan();  StartHot();  StartHum();
    //---------------------------------------------------------------------------------------------------------------------------//
    PressKeyMenu();  if (PressingButtons == 5) goto label_3;          // проверка нажатия кнопки запуска/остановки переворота лотка
    if (Rotate == 1) {
      lcd.setCursor(0, 3);  lcd.print("start rotate EGG");  motor.drive_motor(0, 200);
    }
    else if (Rotate == 0) {
      lcd.setCursor(0, 3);  lcd.print("start rotate EGG"); motor.drive_motor(1, 200);
    }goto label_3;
  }
label_2:
  buttons_Limit_Swith1 = analogRead(PinButtonsLimit1);  buttons_Limit_Swith2 = analogRead(PinButtonsLimit2);  delay(100);
  if (buttons_Limit_Swith1 < 50 || buttons_Limit_Swith2 < 50) buttons_Limit_Swith = 1;  else buttons_Limit_Swith = 0; // если хотябы один из конечников замкнут то button_Limit_Swith = 1
  if (buttons_Limit_Swith == 0) {
    //---------------------------------------------------------------------------------------------------------------------------//
    RTC.getTime();  TempRead();  HumRead_DHT22();  StartFan();  StartHot();  StartHum();  Esp();
    //---------------------------------------------------------------------------------------------------------------------------//
    PressKeyMenu();  if (PressingButtons == 5) goto label_3;          // проверка нажатия кнопки запуска/остановки переворота лотка
    if (Rotate == 1) {
      lcd.setCursor(0, 3);  lcd.print("start rotate EGG");  motor.drive_motor(0, 200);
    }
    else if (Rotate == 0) {
      lcd.setCursor(0, 3);  lcd.print("start rotate EGG");  motor.drive_motor(1, 200);
    }goto label_3;
  }
  else
  {
  label_3:
    lcd.setCursor(0, 3);  lcd.print("stop rotate EGG ");  motor.full_stop(100);
    if (Rotate == 1) Rotate = 0; else if (Rotate == 0) Rotate = 1;
  }
}
void timerot()                                                             // вычисление когда переворачивать яйца
{
  if (TimeRotations != 0)
  {
    int ti = 24 / TimeRotations;
    for (int t = 0; t < 24; t = t + ti)
    {
       if (RTC.hour == t && RTC.minute ==  0)
      {
        if (tf == 0) Rotating();tf = 1;
        if (tf == 1)break; 
      }
    }
  }
  if (RTC.minute > 0)
  {
    tf = 0;
  }
} 

void Esp()
{
  voltage = analogRead(14) * 5.72 / 1024 * 4.575; 
  if( voltage < 50 ) netpower = 1;
  else netpower = 0;
  DayInc = currentMillis /1000 / 60 / 60 /24 ;
  Serial.println("1");                                           //Start == 001
  lcd.setCursor(18, 3); lcd.print("\8");       
  Serial.println("Tnow");  Serial.println(Tnow);                       
  Serial.println("TempIncubations"); Serial.println(TempIncubations);    
  Serial.println("hum");  Serial.println(hum);                        
  Serial.println("HumiditiIncubation");  Serial.println(HumiditiIncubation); 
  Serial.println("Dey");   Serial.println(Dey);                     
  Serial.println("Voltage");   Serial.println(voltage);                   
  Serial.println("NetPower");   Serial.println(netpower);                   
  }

void loop()
{ 
  PressingButtons = 0;
  currentMillis = millis();
   RTC.getTime();
  if (FlagMenu == 0)
  {
    NOWyear = RTC.year; NOWmonth = RTC.month; NOWday = RTC.day; NOWhour = RTC.hour; NOWminute = RTC.minute; NOWsecond = RTC.second;
  }  
  if (currentTime >= (loopTime))
  {
    PressKeyMenu();
    switch (PressingButtons) {
case 1: {                                                                                                        // обработка события нажатия кнопки "МЕНЮ"
      if ((MainMenu == 0 && SubMenu == 0) || (MainMenu == 0 && SubMenu == 1)) { MainMenu = 1;  SubMenu = 0;  StartMillis = currentMillis;  delay(200); }
      else {
        if (MainMenu == 4 && SubMenu == 0) { PreSetTime(); SubMenu = 1; StartMillis = currentMillis;  delay(200); }
        else {
          if (MainMenu != 0 && SubMenu == 0) { SubMenu = 1;  StartMillis = currentMillis;  delay(200); }
          else {
            if (MainMenu == 4 && SubMenu != 0) { SetTime(); SubMenu = 0; FlagMenu = 0; StartMillis = currentMillis; delay(200); }
            else { if (MainMenu != 0 && SubMenu != 0) { SubMenu = 0;  FlagMenu = 0;  StartMillis = currentMillis;  delay(200); } }
          }
        }
      }break; }
    case 2: {                                                                                                        // обработка события нажатия кнопки "ВВЕРХ"
      if (MainMenu != 0 && SubMenu == 0) { MainMenu--;  StartMillis = currentMillis;  delay(200);  if (MainMenu < 1)  MainMenu = 6; }
      else {
        if (MainMenu == 1 && SubMenu != 0 && FlagMenu == 0) { SubMenu--;  StartMillis = currentMillis;  delay(200);  if (SubMenu < 1)  SubMenu = 9; }
        else {
          if (MainMenu == 4 && SubMenu != 0 && FlagMenu == 0) { SubMenu--;  StartMillis = currentMillis;  delay(200);  if (SubMenu < 1)  SubMenu = 6; }
          else {
            if (MainMenu == 5 && SubMenu != 0 && FlagMenu == 0) { SubMenu--;  StartMillis = currentMillis;  delay(200);  if (SubMenu < 1)  SubMenu = 5; }
            else {
              if (MainMenu == 6 && SubMenu != 0 && FlagMenu == 0) { SubMenu--;  StartMillis = currentMillis;  delay(200);  if (SubMenu < 1)  SubMenu = 3; }
              else {
                if (MainMenu == 1 && FlagMenu == 1) {
                  switch (SubMenu) {
                  case 1: {  TempIncubations += FadeAmountTemp;                    StartMillis = currentMillis; if (TempIncubations >= maxTempIncubationsDanger)TempIncubations = maxTempIncubationsDanger;      delay(100);  break; }
                  case 2: {  deltaTIncubations += FadeAmountTemp;                StartMillis = currentMillis; if (deltaTIncubations >= maxdeltaTIncubations)deltaTIncubations = maxdeltaTIncubations;        delay(100);  break; }
                  case 3: {  HumiditiIncubation += FadeAmountHum;               StartMillis = currentMillis; if (HumiditiIncubation >= MaximumHumiditiIncubation)HumiditiIncubation = MaximumHumiditiIncubation;  delay(100);  break; }
                  case 4: {  deltaHumiditiIncubation += + FadeAmountdeltaHum; StartMillis = currentMillis; if (deltaHumiditiIncubation >= maxdeltaHum)deltaHumiditiIncubation = maxdeltaHum;           delay(100);  break; }
                  case 5: {  DecreaseDayHumiditiIncubation += 1; StartMillis = currentMillis; if (DecreaseDayHumiditiIncubation >= MaximumDecreaseDayHumiditiIncubation)DecreaseDayHumiditiIncubation = MaximumDecreaseDayHumiditiIncubation;           delay(100);  break; }
                  case 6: {  IncreaseDayHumiditiIncubation += 1; StartMillis = currentMillis; if (IncreaseDayHumiditiIncubation >= MaximumIncreaseDayHumiditiIncubation)IncreaseDayHumiditiIncubation = MaximumIncreaseDayHumiditiIncubation;           delay(100);  break; }
                  case 7: { KHumiditiIncubation += 1; StartMillis = currentMillis; if (KHumiditiIncubation >= MaximumKHumiditiIncubation)KHumiditiIncubation = MaximumKHumiditiIncubation;           delay(100);  break; }
                  case 8: { i++;  if (i > 8)  i = 0;  TimeRotations = RotationPerDay[i];         StartMillis = currentMillis;                                                                                                     delay(100);  break; }
                  case 9: { StartMillis = currentMillis; if (FanWorkFlag == 0) FanWorkFlag = 1; else if (FanWorkFlag == 1) FanWorkFlag = 0; delay(100);break; }
                 }
                }
                else {
                  if (MainMenu == 2 && FlagMenu == 0 && SubMenu == 1) { bank++; StartMillis = currentMillis;  delay(200); if (bank > 4) bank = 4; }
                  else {
                    if (MainMenu == 3 && FlagMenu == 0 && SubMenu == 1) { bank++; StartMillis = currentMillis;  delay(200); if (bank > 4) bank = 4; }
                    else {
                      if (MainMenu == 4 && FlagMenu == 1) {
                        switch (SubMenu) {
                        case 1: {  NOWyear++;    if (NOWyear > 2030)  NOWyear = 2030;  delay(200);  GoSetTime(NOWyear);  break; }
                        case 2: {  NOWmonth++;   if (NOWmonth > 12)   NOWmonth = 12;   delay(200);  GoSetTime(NOWmonth);  break; }
                        case 3: {  NOWday++;     if (NOWday > 31)     NOWday = 31;     delay(200);  GoSetTime(NOWday);  break; }
                        case 4: {  NOWhour++;    if (NOWhour > 23)    NOWhour = 23;    delay(200);  GoSetTime(NOWhour);  break; }
                        case 5: {  NOWminute++;  if (NOWminute > 59)  NOWminute = 59;  delay(200);  GoSetTime(NOWminute);  break; }
                        case 6: {  NOWsecond++;  if (NOWsecond > 59)  NOWsecond = 59;  delay(200);  GoSetTime(NOWsecond);  break; }
                        }
                      }
                      else {
                        if (MainMenu == 5 && FlagMenu == 1) {
                          switch (SubMenu) {
                          case 1: {  consKp += 0.01;                    StartMillis = currentMillis; if (consKp >= 100)consKp = 100;      delay(100); EEPROM.write(131,consKp);  break; }
                          case 2: {  consKi += 0.01;                    StartMillis = currentMillis; if (consKp >= 100)consKp = 100;      delay(100); EEPROM.write(132,consKi);  break; }
                          case 3: {  consKd += 0.01;                    StartMillis = currentMillis; if (consKp >= 100)consKp = 100;      delay(100); EEPROM.write(133,consKd);  break; }
                          case 4: { maxTempFanStart += FadeAmountTemp;                    StartMillis = currentMillis; if (maxTempFanStart >= maxTempFanStartMenuMax)maxTempFanStart = maxTempFanStartMenuMax;        delay(100);  break; }
                          case 5: { TimeFanWork += FadeTimeFanWork; StartMillis = currentMillis; if (TimeFanWork >= maxTimeFanWork) TimeFanWork = maxTimeFanWork; TimeFaningInterval = TimeFanWork * 1000;  delay(100);  break; }
                          case 6: { TimeIntervalFanWork += FadeTimeIntervalFanWork; StartMillis = currentMillis; if (TimeIntervalFanWork >= maxTimeIntervalFanWork)TimeIntervalFanWork = maxTimeIntervalFanWork; TimeIntervalFaningInterval = TimeIntervalFanWork * 60000; delay(100);  break; }
                   }
                        }
                        else {
                          if (MainMenu == 6 && FlagMenu == 1) {
                            switch (SubMenu) {
                            case 1: {  delay(100); break; }
                            case 2: {  if (TIFlagd == 0)   TIFlagd = 1;                      else if (TIFlagd == 1) TIFlagd = 0; EEPROM.write(110, TIFlagd); delay(200); break; }
                            case 3: {  if (TIFlagf == 0) { TIFlagf = 1; RASEEPROMSTimer(); } else if (TIFlagf == 1) TIFlagf = 0; EEPROM.write(111, TIFlagf); delay(200); break; }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }break; }
    case 3: {                                                                                                        // обработка события нажатия кнопки "ВНИЗ"
      if (MainMenu != 0 && SubMenu == 0) { MainMenu++;  StartMillis = currentMillis;  delay(200);  if (MainMenu > 6)  MainMenu = 1; }
      else {
        if (MainMenu == 1 && SubMenu != 0 && FlagMenu == 0) { SubMenu++;  StartMillis = currentMillis;  delay(200);  if (SubMenu > 9)  SubMenu = 1; }
        else {
          if (MainMenu == 4 && SubMenu != 0 && FlagMenu == 0) { SubMenu++;  StartMillis = currentMillis;  delay(200);  if (SubMenu > 6)  SubMenu = 1; }
          else {
            if (MainMenu == 5 && SubMenu != 0 && FlagMenu == 0) { SubMenu++;  StartMillis = currentMillis;  delay(200);  if (SubMenu > 6)  SubMenu = 1; }
            else {
              if (MainMenu == 6 && SubMenu != 0 && FlagMenu == 0) { SubMenu++;  StartMillis = currentMillis;  delay(200);  if (SubMenu > 3)  SubMenu = 1; }
              else {
                if (MainMenu == 1 && FlagMenu == 1) {
                  switch (SubMenu) {
                  case 1: {  TempIncubations -= FadeAmountTemp;                      StartMillis = currentMillis;  if (TempIncubations <= minTempIncubationsDanger)  TempIncubations = minTempIncubationsDanger;      delay(100);  break; }
                  case 2: {  deltaTIncubations -= FadeAmountTemp;                  StartMillis = currentMillis;  if (deltaTIncubations <= mindeltaTIncubations)  deltaTIncubations = mindeltaTIncubations;        delay(100);  break; }
                  case 3: {  HumiditiIncubation -= FadeAmountHum;                 StartMillis = currentMillis;  if (HumiditiIncubation <= MinimumHumiditiIncubation)  HumiditiIncubation = MinimumHumiditiIncubation;  delay(100);  break; }
                  case 4: {  deltaHumiditiIncubation -= FadeAmountdeltaHum;  StartMillis = currentMillis;  if (deltaHumiditiIncubation <= mindeltaHum)  deltaHumiditiIncubation = mindeltaHum;           delay(100);  break; }
                  case 5: {  DecreaseDayHumiditiIncubation -= 1; StartMillis = currentMillis; if (DecreaseDayHumiditiIncubation <= MinimumDecreaseDayHumiditiIncubation)DecreaseDayHumiditiIncubation = MinimumDecreaseDayHumiditiIncubation;           delay(100);  break; }
                  case 6: {  IncreaseDayHumiditiIncubation -= 1; StartMillis = currentMillis; if (IncreaseDayHumiditiIncubation <= MinimumIncreaseDayHumiditiIncubation)IncreaseDayHumiditiIncubation = MinimumIncreaseDayHumiditiIncubation;           delay(100);  break; }
                  case 7: { KHumiditiIncubation -= 1; StartMillis = currentMillis; if (KHumiditiIncubation <= MinimumKHumiditiIncubation)KHumiditiIncubation = MinimumKHumiditiIncubation;           delay(100);  break; }
                  case 8: { i--;  if (i < 0)  i = 8;  TimeRotations = RotationPerDay[i];            StartMillis = currentMillis;                                                                                                           delay(100);  break; }
                  case 9: { StartMillis = currentMillis; if (FanWorkFlag == 0) FanWorkFlag = 1; else if (FanWorkFlag == 1) FanWorkFlag = 0; delay(100);break; }
                }
                }
                else {
                  if (MainMenu == 2 && FlagMenu == 0 && SubMenu == 1) { bank--; StartMillis = currentMillis;  delay(200); if (bank < 0) bank = 0; }
                  else {
                    if (MainMenu == 3 && FlagMenu == 0 && SubMenu == 1) { bank--; StartMillis = currentMillis;  delay(200); if (bank < 0) bank = 0; }
                    else {
                      if (MainMenu == 4 && FlagMenu == 1) {
                        switch (SubMenu) {
                        case 1: {  NOWyear--;    StartMillis = currentMillis;  if (NOWyear < 2015) NOWyear = 2015; delay(200);  GoSetTime(NOWyear);  break; }
                        case 2: {  NOWmonth--;   StartMillis = currentMillis;  if (NOWmonth < 1) NOWmonth = 1;   delay(200);  GoSetTime(NOWmonth);  break; }
                        case 3: {  NOWday--;     StartMillis = currentMillis;  if (NOWday < 1) NOWday = 1;     delay(200);  GoSetTime(NOWday);  break; }
                        case 4: {  NOWhour--;    StartMillis = currentMillis;  if (NOWhour < 0) NOWhour = 0;    delay(200);  GoSetTime(NOWhour);  break; }
                        case 5: {  NOWminute--;  StartMillis = currentMillis;  if (NOWminute < 0) NOWminute = 0;  delay(200);  GoSetTime(NOWminute);  break; }
                        case 6: {  NOWsecond--;  StartMillis = currentMillis;  if (NOWsecond < 0) NOWsecond = 0;  delay(200);  GoSetTime(NOWsecond);  break; }
                        }
                      }
                      else {
                        if (MainMenu == 5 && FlagMenu == 1) {
                          switch (SubMenu) {
                          case 1: {  consKp -= 0.01;                    StartMillis = currentMillis; if (consKp >= 0.01)consKp = 0.01;      delay(100); EEPROM.write(131,consKp); break; }
                          case 2: {  consKi -= 0.01;                    StartMillis = currentMillis; if (consKp >= 0.01)consKp = 0.01;      delay(100); EEPROM.write(132,consKi); break; }
                          case 3: {  consKd -= 0.01;                    StartMillis = currentMillis; if (consKp >= 0.01)consKp = 0.01;      delay(100); EEPROM.write(133,consKd); break; }
                          case 4: { maxTempFanStart -= FadeAmountTemp;                      StartMillis = currentMillis;  if (maxTempFanStart <= maxTempFanStartMenuMin)  maxTempFanStart = maxTempFanStartMenuMin;        delay(100);  break; }
                          case 5: { TimeIntervalFanWork -= FadeTimeIntervalFanWork;     StartMillis = currentMillis;  if (TimeIntervalFanWork <= minTimeIntervalFanWork)  TimeIntervalFanWork = minTimeIntervalFanWork;  TimeIntervalFaningInterval = TimeIntervalFanWork * 60000;   delay(100);  break; }
                          case 6: { TimeFanWork -=FadeTimeFanWork;                             StartMillis = currentMillis;  if (TimeFanWork <= minTimeFanWork)  TimeFanWork = minTimeFanWork; TimeFaningInterval = TimeFanWork * 1000; delay(100);  break; }
                  }
                        }
                        else {
                          if (MainMenu == 6 && FlagMenu == 1) {
                            switch (SubMenu) {
                            case 1: {  StartMillis = currentMillis; delay(100); break; }
                            case 2: {  StartMillis = currentMillis; if (TIFlagd == 0)   TIFlagd = 1;                      else if (TIFlagd == 1) TIFlagd = 0; EEPROM.write(110, TIFlagd); delay(200); break; }
                            case 3: {  StartMillis = currentMillis; if (TIFlagf == 0) { TIFlagf = 1; RASEEPROMSTimer(); } else if (TIFlagf == 1) TIFlagf = 0; EEPROM.write(111, TIFlagf); delay(200); break; }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }break; }
    case 4: {                                                                                                        // обработка события нажатия кнопки "ВЫБОР"
      if (MainMenu == 0 && SubMenu == 0) { MainMenu = 0;  SubMenu = 1;  delay(200); }
      else {
        if (MainMenu == 0 && SubMenu == 1) { MainMenu = 0;  SubMenu = 0;  delay(200); }
        //else {  if ( MainMenu == 2 && SubMenu == 1 ) { SaveToEEPROM ( bank ); lcd.setCursor(0, 1); lcd.print( "    saving...   " ); }
        //else {  if ( MainMenu == 3 && SubMenu == 1 ) { LoadFromEEPROM ( bank ); lcd.setCursor(0, 1); lcd.print( "    loading...  " ); }
        else {
          if (MainMenu != 0 && SubMenu != 0 && FlagMenu == 0) { FlagMenu = 1;  delay(200); }
          else { if (MainMenu != 0 && SubMenu != 0 && FlagMenu == 1) { FlagMenu = 0;  delay(200); } }
        }
      }
      break; }
    case 5: {                                                                                                        // обработка события нажатия кнопки "ПЕРЕВОРОТ"
      Rotating();
      break; }
    }
    switch (MainMenu) {
    case 0:      // главное меню "0"
      switch (SubMenu) {
      case 0:  m = 0; break;  // вывод на экран главного меню "0" подменю "0"
      case 1:  m = 1; break;  // вывод на экран главного меню "0" подменю "1"
      } 
      break;
    case 1:      // главное меню "1"
      switch (SubMenu) {
      case 0:  m = 10; break;  // вывод на экран главного меню "1" подменю "0"
      case 1:  m = 11; break;  // вывод на экран главного меню "1" подменю "1"
      case 2:  m = 12; break;  // вывод на экран главного меню "1" подменю "2"
      case 3:  m = 13; break;  // вывод на экран главного меню "1" подменю "3"
      case 4:  m = 14; break;  // вывод на экран главного меню "1" подменю "4"
      case 5:  m = 15; break;  // вывод на экран главного меню "1" подменю "5"
      case 6:  m = 16; break;  // вывод на экран главного меню "1" подменю "6"
      case 7:  m = 17; break;  // вывод на экран главного меню "1" подменю "7"
      case 8:  m = 18; break;  // вывод на экран главного меню "1" подменю "8"
      case 9:  m = 19; break;  // вывод на экран главного меню "1" подменю "9"
      } 
      break;
    case 2:      // главное меню "2"
      switch (SubMenu) {
      case 0:  m = 20; break;  // вывод на экран главного меню "2" подменю "0"
      case 1:  m = 21; break; // вывод на экран главного меню "2" подменю "1"
      } 
      break;
    case 3:      // главное меню "3"
      switch (SubMenu) {
      case 0:  m = 30; break;  // вывод на экран главного меню "3" подменю "0"
      case 1:  m = 31; break;  // вывод на экран главного меню "3" подменю "1"
      } 
      break;
    case 4:
      switch (SubMenu) {
      case 0:  m = 40; break;  // вывод на экран главного меню "4" подменю "0"
      case 1:  m = 41; break;  // вывод на экран главного меню "4" подменю "1"
      case 2:  m = 42; break;  // вывод на экран главного меню "4" подменю "2"
      case 3:  m = 43; break;  // вывод на экран главного меню "4" подменю "3"
      case 4:  m = 44; break;  // вывод на экран главного меню "4" подменю "4"
      case 5:  m = 45; break;  // вывод на экран главного меню "4" подменю "5"
      case 6:  m = 46; break;  // вывод на экран главного меню "4" подменю "6"
      } 
      break;
    case 5:
      switch (SubMenu) {
      case 0:  m = 50; break;  // вывод на экран главного меню "5" подменю "0"
      case 1:  m = 51; break;  // вывод на экран главного меню "5" подменю "1"
      case 2:  m = 52; break;  // вывод на экран главного меню "5" подменю "2"
      case 3:  m = 53; break;  // вывод на экран главного меню "5" подменю "3"
      case 4:  m = 54; break;  // вывод на экран главного меню "5" подменю "4"
      case 5:  m = 55; break;  // вывод на экран главного меню "5" подменю "5"
      case 6:  m = 56; break;  // вывод на экран главного меню "5" подменю "6"
      } 
      break;
    case 6:
      switch (SubMenu) {
      case 0:  m = 60; break;  // вывод на экран главного меню "6" подменю "0"
      case 1:  m = 61; break;  // вывод на экран главного меню "6" подменю "1"
      case 2:  m = 62; break;  // вывод на экран главного меню "6" подменю "2"
      case 3:  m = 63; break;  // вывод на экран главного меню "6" подменю "3"
      }
    }

    if (MainMenu == 4 && SubMenu != 0) {}
    else {
      if (currentMillis - StartMillis > interval) {
        StartMillis = currentMillis;
        if (MainMenu == 0 && SubMenu == 0) { MainMenu = 0;  SubMenu = 1; }
        else {
          if (MainMenu == 0 && SubMenu == 1) { MainMenu = 0;  SubMenu = 0; }
          else { MainMenu = 0;  SubMenu = 0; }
        }
      }
    }
  }
  switch (m) {
  case 0: {  lcd.clear();lcd.setCursor(0, 0);  lcd.print("T="); lcd.print(Tnow); lcd.print("\3 (");lcd.print(TempIncubations);    lcd.print("\3)");  
             lcd.setCursor(0, 1);  lcd.print("H="); lcd.print(hum);  lcd.print("%("); lcd.print(HumiditiIncubation); lcd.print("%)");
             if( voltage == 0 ){lcd.print("220V - ON"); }if( voltage > 0 ){ lcd.print("Bat="); lcd.print(voltage);}
             lcd.setCursor(0, 2);  lcd.print("Day = ");lcd.print(DayInc);lcd.print(" ; ");
            if (RTC.hour < 10) lcd.print(0); lcd.print(RTC.hour); lcd.print(":"); if (RTC.minute < 10) lcd.print(0); lcd.print(RTC.minute); lcd.print(":"); if (RTC.second < 10) lcd.print(0); lcd.print(RTC.second);
              TempRead(); HumRead_DHT22();StartFan();  StartHot();  StartHum(); timerot();TimerCalculatePrint();Esp();FlagMenu = 0;             break; }
  case 10: {  lcd.clear(); lcd.setCursor(0, 1); lcd.print(F("    Setting     ")); lcd.setCursor(0, 2); lcd.print(F("   incubation   ")); lcd.setCursor(15, 1);                lcd.print("\1");                                                                                  delay(100);FlagMenu = 0;             break; }
  case 11: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("Temperature inc ")); lcd.setCursor(0, 1); lcd.print(F("t = ")); lcd.print(TempIncubations);        lcd.print("\3                ");                                      PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 12: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("    Delta T     ")); lcd.setCursor(0, 1); lcd.print(F("\4t = ")); lcd.print(deltaTIncubations);      lcd.print("\3                ");                                      PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 13: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("   Humiditi     ")); lcd.setCursor(0, 1); lcd.print(F("H = ")); lcd.print(HumiditiIncubation);     lcd.print("%                 ");                                      PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 14: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("    Delta H     ")); lcd.setCursor(0, 1); lcd.print(F("\4h = ")); lcd.print(deltaHumiditiIncubation); lcd.print("%                 ");                                      PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 15: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F(" Day Decrease H ")); lcd.setCursor(0, 1); lcd.print(F(" ")); lcd.print(DecreaseDayHumiditiIncubation); lcd.print("                 ");                                      PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 16: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F(" Day Increase H ")); lcd.setCursor(0, 1); lcd.print(F(" ")); lcd.print(IncreaseDayHumiditiIncubation); lcd.print("                 ");                                      PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 17: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("     Factor H   ")); lcd.setCursor(0, 1); lcd.print(F(" ")); lcd.print(KHumiditiIncubation); lcd.print("                 ");                                      PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 18: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("  Rotate Egg    ")); lcd.setCursor(0, 1);                                  lcd.print(TimeRotations);     lcd.print(" times per day    ");                                      PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 20: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("  Save setting  ")); lcd.setCursor(0, 1); lcd.print(F("   to  EEPROM   ")); lcd.setCursor(15, 1);                lcd.print("\1");                                                                                  delay(100); FlagMenu = 0;             break; }
  case 21: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("Save setting to ")); lcd.setCursor(0, 1); lcd.print(F("bank ")); lcd.print(bank); if (FlagMenu == 0) { lcd.print(" press set"); delay(100); } else { SaveToEEPROM(bank); lcd.print(" saving...");      delay(100); }                          break; }
  case 30: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("  Load setting  ")); lcd.setCursor(0, 1); lcd.print(F("  from  EEPROM  ")); lcd.setCursor(15, 1);                lcd.print("\1");                                                                                  delay(100); FlagMenu = 0;             break; }
  case 31: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("Load setting fro")); lcd.setCursor(0, 1); lcd.print(F("bank ")); lcd.print(bank); if (FlagMenu == 0) { lcd.print(" press set"); delay(100); } else { LoadFromEEPROM(bank); lcd.print(" loading..");    delay(100); }                          break; }
  case 40: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("      Time      ")); lcd.setCursor(0, 1); lcd.print(F("     setting    ")); lcd.setCursor(15, 1);                lcd.print("\1");                                                                                  delay(100); FlagMenu = 0;             break; }
  case 41: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("  setup  year   ")); lcd.setCursor(5, 1);                                  lcd.print(Setyear);                                                                                     PrintMenuWrite(FlagMenu); delay(100);                           break; }
  case 42: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("  setup  month  ")); lcd.setCursor(6, 1);                                  lcd.print(Setmonth);                                                                                     PrintMenuWrite(FlagMenu); delay(100);                           break; }
  case 43: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("   setup  day   ")); lcd.setCursor(6, 1);                                  lcd.print(Setday);                                                                                     PrintMenuWrite(FlagMenu); delay(100);                           break; }
  case 44: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("  setup  hour   ")); lcd.setCursor(6, 1);                                  lcd.print(Sethour);                                                                                     PrintMenuWrite(FlagMenu); delay(100);                           break; }
  case 45: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("  setup minute  ")); lcd.setCursor(6, 1);                                  lcd.print(Setminute);                                                                                     PrintMenuWrite(FlagMenu); delay(100);                           break; }
  case 46: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("  setup second  ")); lcd.setCursor(6, 1);                                  lcd.print(Setsecond);                                                                                     PrintMenuWrite(FlagMenu); delay(100);                           break; }
  case 50: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F(" setting  relay ")); lcd.setCursor(0, 1); lcd.print("     output     "); lcd.setCursor(15, 1);  lcd.print("\1");                                                                                                delay(100); FlagMenu = 0;             break; }
  case 51: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("      K p     ")); lcd.setCursor(0, 1); lcd.print(F("P = ")); lcd.print(consKp);                                            PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 52: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("      K i     ")); lcd.setCursor(0, 1); lcd.print(F("I = ")); lcd.print(consKi);                                            PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 53: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("      K d     ")); lcd.setCursor(0, 1); lcd.print(F("D = ")); lcd.print(consKd);                                            PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 54: { lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("   Cooling T    ")); lcd.setCursor(0, 1); lcd.print(F("t = ")); lcd.print(maxTempFanStart);        lcd.print("\3                ");                                      PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 55: { lcd.clear(); lcd.setCursor(0, 0); lcd.print(F(" CO2 faning time")); lcd.setCursor(0, 1); lcd.print(F("time = ")); lcd.print(TimeFanWork);            lcd.print(" sec. ");                                      PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 56: { lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("CO2 fan interval")); lcd.setCursor(0, 1); lcd.print(F("time = ")); lcd.print(TimeIntervalFanWork);    lcd.print(" min. ");                                      PrintMenuWrite(FlagMenu); delay(100);                          break; }
  case 60: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("     Timer      ")); lcd.setCursor(0, 1); lcd.print("   incubation   "); lcd.setCursor(15, 1);                lcd.print("\1");                                                                                  delay(100); FlagMenu = 0;             break; }
  case 61: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("  reset  timer  ")); PrintMenuWrite(FlagMenu); if (FlagMenu == 0) { lcd.setCursor(0, 1);                 lcd.print("yes - press set ");                                                                   delay(100); } else RASEEPROMSTimer();  break; }
  case 62: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F(" display timer  ")); lcd.setCursor(0, 1); if (TIFlagd == 0) lcd.print("       OFF      "); else if (TIFlagd == 1) lcd.print("     IS ON      ");                               PrintMenuWrite(FlagMenu); delay(100);                           break; }
  case 63: {  lcd.clear(); lcd.setCursor(0, 0); lcd.print(F("     timer      ")); lcd.setCursor(0, 1); if (TIFlagf == 0) lcd.print("       OFF      "); else if (TIFlagf == 1) lcd.print("     IS ON      ");                               PrintMenuWrite(FlagMenu); delay(100);                           break; }
  }}
