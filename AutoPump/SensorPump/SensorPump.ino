// НАСТРОЙКА ПИНОВ
#define PinDs A1      // Пин подключения датчика температуры DS18B20
#define PinHum A2     // Пин подключения датчика влажности
#define MosPump 3     // Пин подключения мосфета для контроля помпы

// НАСТРОЙКИ РАБОТЫ СИСТЕМЫ АВТОПОЛИВА
#define Hum_min 30          // Нижний предел влажности почвы в процентах (когда нужно включать)
#define Hum_max 50          // Верхний предел влажности почвы в процентах (когда нужно выключать)

#define AnalogMin 650       // Показания датчика влажности в сухой почве
#define AnalogMax 300       // Показания датчика влажности в воде

#include <LiquidCrystal_I2C.h>    // Подключение библиотеки для работы с дисплеем
byte degree[8]   = {B11100,B10100,B11100,B00000,B00000,B00000,B00000,B00000,};    // Попиксельная прорисовка символа градуса
LiquidCrystal_I2C lcd(0x27,16,2);   // Задаем адрес и размер дисплея

#include <microDS18B20.h>   // Подключение библиотеки для работы с датчиком температуры
MicroDS18B20 dallasSensors(PinDs);    // Инициализация датчика


// ОБЪЯВЛЕНИЕ ПЕРЕМЕННЫХ
unsigned long readTimer;
uint16_t hum;
float temp;
 
void setup()
{
  delay(300);                   // Ждём готовности модулей 300мс перед началом инициализации
  Serial.begin(9600);           // Устанавливаем скорость передачи данных с модулей в 9600 бод   
                    
  lcd.init();                   // Инициализация дисплея    
  lcd.createChar(1, degree);    // Создаем символ градуса и присваиваем ему номер 1
  lcd.backlight();              // Включение подсветки дисплея 

  pinMode(MosPump, OUTPUT);     // Настройка пинов подключения мосфетов на вывод                
}


void loop()
{
  if (millis() - readTimer > 1000) {  // Секундный таймер (для стабильности измерений) (millis() считает время с момента включения программы, при превышении разницы значений запуск условия)
    
    temp = dallasSensors.getTemp();   // Считывание показаний датчика температуры DS18B20
    dallasSensors.requestTemp();

    hum = analogRead(PinHum);             // Считывание показаний влажности
    hum = map(hum, AnalogMin, AnalogMax, 0, 100);     // Нормализация значений влажности

            // ОТЛАДКА (вывод в монитор порта)
            Serial.print("\t"); 
            Serial.print(String(temp));
            Serial.print("\t");
            Serial.print(String(hum));
            Serial.print("\t");
            Serial.println("MosPump_" + String(digitalRead(MosPump)));

    readTimer = millis();                 // Обнулить таймер (делаем разность значений равной нулю)
  }

    // Условия работы помпы                        
    if (hum < Hum_min) {                                                        
      digitalWrite(MosPump, LOW);    // Включение мосфета помпы (ВНИМАНИЕ! Замени LOW на HIGH, чтобы помпа начала включаться)
      lcd.setCursor(11,1);
      lcd.print(" (ON)");
    }                                
    if (hum > Hum_max) {                                          
      digitalWrite(MosPump, LOW);     // Выключение мосфета помпы  
      lcd.setCursor(11,1);
      lcd.print("(OFF)");         
    }   
              
  lcd.setCursor(0,0);           // Устанавливаем курсор на 1 строку, ячейка 1
  lcd.print("TEMP = " + String(temp) + "\1C ");      // Выводим показания температуры и символ градуса
  
  lcd.setCursor(0,1);           // Устанавливаем курсор на 2 строку, ячейка 1
  lcd.print("HUM = " + String(hum) + "%  ");       // Выводим показания влажности              
 
  delay(5);                     // Задержка выполнения тела программы для стабильности работы
}
