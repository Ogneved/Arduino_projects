#define PERIOD 172800     // period of work in sec  (November-April: 172800, May-October: 172800)
#define WORK 28          // time of work in sec     (November-April: 35, May-October: 50)
#define MOS 1           // connection pin of mosfet

uint32_t mainTimer, myTimer;
boolean state = false;

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC


void setup() {
  for (byte i = 0; i < 6; i++) {
    pinMode(i, INPUT);
  }
  adc_disable();          // off ADC

  wdt_reset();            
  wdt_enable(WDTO_1S);   // watchdog initialisation
  // 15MS, 30MS, 60MS, 120MS, 250MS, 500MS, 1S, 2S, 4S, 8S

  WDTCR |= _BV(WDIE);     // allow watchdog interrupt
  sei();                  // allow interrupt
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // full sleep
}

void loop() {
  mainTimer++;

  if (!state) {                           
    if ((long)mainTimer - myTimer > PERIOD) {   
      myTimer = mainTimer;                
      state = true;                       
      pinMode(MOS, OUTPUT);               
      digitalWrite(MOS, HIGH);            
    }
  } else {                                
    if ((long)mainTimer - myTimer > WORK) {    
      myTimer = mainTimer;                
      state = false;                      
      digitalWrite(MOS, LOW);             
      pinMode(MOS, INPUT);                
    }
  }

  sleep_enable();   // allow sleep
  sleep_cpu();      // sleep
}

ISR (WDT_vect) {
  WDTCR |= _BV(WDIE); // allow watchdog interrupt 
}
