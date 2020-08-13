// НАСТРОЙКА ПИНОВ
#define PinDs A1      // Пин подключения датчика температуры DS18B20
#define PinHum A2     // Пин подключения датчика влажности/температуры DHT11
#define PinHall 3     // Пин подключения датчика Холла
#define MosTemp 5     // Пин подключения мосфета для контроля нагревателя
#define MosHum 8      // Пин подключения мосфета для контроля влажности
#define MosLight 7    // Пин подключения мосфета для контроля освещения
#define RTC_DAT 9     // Три пина подключения часов реального времени DS1302
#define RTC_CLK 6
#define RTC_RST 4

// НАСТРОЙКИ РАБОТЫ ИНКУБАТОРА
#define T 37.0            // Поддерживаемая температура внутри в градусах Цельсия. На деле будет +- 0.5 градуса.
#define RH_min 45           // Нижний предел влажности внутри в процентах.
#define RH_max 60           // Верхний предел влажности внутри в процентах.
#define mosHumTimeout 20  // Время ожидания до включения увлажнителя после достижения нижнего порогового значения, минут


#include <LiquidCrystal_I2C.h>    // Подключение библиотеки для работы с дисплеем
byte degree[8]   = {B11100,B10100,B11100,B00000,B00000,B00000,B00000,B00000,};    // Попиксельная прорисовка символа градуса
LiquidCrystal_I2C lcd(0x27,16,2);   // Задаем адрес и размер дисплея

#include <microDS18B20.h>   // Подключение библиотеки для работы с датчиком температуры
MicroDS18B20 dallasSensors(PinDs);    // Инициализация датчика

#include <DHT.h>    // Подключение библиотеки для работы с датчиком влажности
DHT dht(PinHum, DHT11);   // Инициализация датчика

// ОБЪЯВЛЕНИЕ ПЕРЕМЕННЫХ
unsigned long readTimer;
unsigned long waitTimer;
int hum;
int hall;
float temp;
float tempDHT;
int CurDay;

 
void setup()
{
  delay(300);                   // Ждём готовности модулей 300мс перед началом инициализации
  Serial.begin(9600);           // Устанавливаем скорость передачи данных с модулей в 9600 бод   
                    
  lcd.init();                   // Инициализация дисплея    
  lcd.createChar(1, degree);    // Создаем символ градуса и присваиваем ему номер 1

  pinMode(MosTemp, OUTPUT);     // Настройка пинов подключения мосфетов на вывод
  pinMode(MosHum, OUTPUT);
  pinMode(MosLight, OUTPUT);                  
}


void loop()
{
  if (millis() - readTimer > 1000) {  // Секундный таймер (для стабильности измерений) (millis() считает время с момента включения программы, при превышении разницы значений запуск условия)
    
    temp = dallasSensors.getTemp();   // Считывание показаний датчика температуры DS18B20
    dallasSensors.requestTemp();

    tempDHT = (dht.readTemperature());    // Считывание показаний температуры с датчика DHT11
    hum = (dht.readHumidity());           // Считывание показаний влажности
    hum = constrain(hum, 0, 99);          // Нормализация значений влажности

    hall = digitalRead(PinHall);          // Считывание показаний датчика Холла

            // ОТЛАДКА (вывод в монитор порта)
            Serial.print("\t"); 
            Serial.print(String(temp));
            Serial.print("\t");
            Serial.print(String(hum));
            Serial.print("\t");
            Serial.print("MosTemp_" + String(digitalRead(MosTemp)));
            Serial.print("\t");
            Serial.println("MosHum_" + String(digitalRead(MosHum)));

    readTimer = millis();                 // Обнулить таймер (делаем разность значений равной нулю)
  }

    // Условия работы обогрева                        
    if (temp < T) {                                                        
      digitalWrite(MosTemp, HIGH);    // Включение мосфета нагревателя
      lcd.setCursor(0,0);
      lcd.print("TEMP");
    }                                
    else {                                          
      digitalWrite(MosTemp, LOW);     // Выключение мосфета нагревателя  
      lcd.setCursor(0,0);
      lcd.print("    ");         
    }   

    // Условия работы увлажнения                       
    if (hum < RH_min /*&& millis() - waitTimer > mosHumTimeout*60000*/) {   // Если влажность ниже порогового значения и прошло время таймера, запустить                                                  
      digitalWrite(MosHum, HIGH);     // Включение мосфета увлажнителя
      lcd.setCursor(6,0);
      lcd.print("HUM");
    }                                
    if (hum > RH_max || hall == 0 || millis() - waitTimer > /*mosHumTimeout*60000 +*/ 300000){    // Если влажность выше порогового значения или прошло время таймера, отключить                                        
      digitalWrite(MosHum, LOW);    // Выключение мосфета увлажнителя
      lcd.setCursor(6,0);
      lcd.print("   ");
      waitTimer = millis();   // Обнуление таймера
    }
            
    // Условия открытие двери                         
    if (hall == 0) {                  // Дверь открылась                                                      
      digitalWrite(MosLight, HIGH);   // Включение света
      lcd.backlight();    // Включение подсветки дисплея             
      lcd.setCursor(11,0);
      lcd.print("LIGHT");
    }                                
    else {                            // Дверь закрылась
      digitalWrite(MosLight, LOW);    // Выключение света        
      lcd.noBacklight();              // Выключение подсветки дисплея 
      lcd.setCursor(11,0);
      lcd.print("     ");               
    }

  
  lcd.setCursor(0,1);           // Устанавливаем курсор на 2 строку, ячейка 1
  lcd.print(String(temp));      // Выводим показания температуры
  lcd.setCursor(5,1);           // Устанавливаем курсор на 2 строку, ячейка 4
  lcd.print("\1C");             // Выводим символ градуса
  
  lcd.setCursor(8,1);
  lcd.print(String(hum));       // Выводим показания влажности              
  lcd.setCursor(11,1);
  lcd.print("%");

//  lcd.setCursor(13,1);
//  lcd.print(String(CurDay));    // Выводим текущий день
//  lcd.setCursor(15,1);
//  lcd.print("d");
 
  delay(5);                             // Задержка выполнения тела программы для стабильности работы
}
