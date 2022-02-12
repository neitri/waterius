
#include "Setup.h"
#include "Sensor.h"
#include "Power.h"
#include "SlaveI2C.h"

#include <Wire.h>

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>  
#include <avr/sleep.h>
#include <avr/wdt.h>

// Для логирования раскомментируйте LOG_ON в Setup.h
#if defined(LOG_ON)
	TinyDebugSerial mySerial;
#endif


#define FIRMWARE_VER 23    // Передается в ESP и на сервер в данных.
  
/*
Версии прошивок 

22 - 2021.07.13 - dontsovcmc
	1. переписана работа с watchdog: чип перезагрузится в случае сбоя

21 - 2021.07.01 - dontsovcmc
    1. переписана работа с watchdog
	2. поле voltage стало uint16 (2 байта от uint32 пустые для совместимости с 0.10.3)
	3. период пробуждения 15 мин, от ESP получит 1440 или другой.

20 - 2021.05.31 - dontsovcmc
    1. atmelavr@3.3.0
	2. конфигурация для attiny45

19 - 2021.04.03 - dontsovcmc
	1. WDTCR = _BV( WDCE ); в resetWatchdog

18 - 2021.04.02 - dontsovcmc
	1. WDTCR |= _BV( WDIE ); в прерывании

17 - 2021.04.01 - dontsovcmc
    1. Рефакторинг getWakeUpPeriod

16 - 2021.03.29 - dontsovcmc
	1. Отключение подтягивающих резисторов в I2C (ошибка в tinycore)
	2. Отключение ESP с задержкой 100мс после получения команды на сон (потребление ESP ниже на 7мкА).

15 - 2021.02.07 - kick2nick
	Время пробуждения ESP изменено с 1 суток (1440 мин.) на настриваемое значение
	1. Добавил период пробуждения esp.
	2. Добавил команду приема периода пробуждения по I2C.

14 - 2020.11.09 - dontsovcmc
    1. поддержка attiny84 в отдельной ветке

13 - 2020.06.17 - dontsovcmc
	1. изменил формулу crc
	2. поддержка версии на 4 счетчика (attiny84)
	   -D BUILD_WATERIUS_4C2W

12 - 2020.05.15 - dontsovcmc
	1. Добавил команду T для переключения режима пробуждения
	2. Добавил отправку аналогового уровня замыкания входа в ЕСП
	3. Исправил инициализацию входов. Кажется после перезагрузки +1 импульс
	4. Добавил crc при отправке данных

11 - 2019.10.20 - dontsovcmc
    1. Обновил алгоритм подсчёта импульсов.
	   Теперь импульс: 1 раз замыкание + 3 раза разомкнуто. Период 250мс +- 10%.
	
10 - 2019.09.16 - dontsovcmc 
    1. Замеряем питание пока общаемся с ESP
	2. Время настройки 10 минут.

9 - 2019.05.04 - dontsovcmc
    1. USIWire заменен на Wire

8 - 2019.04.05 - dontsovcmc
    1. Добавил поддержку НАМУР. Теперь чтение состояния analogRead
	2. Добавил состояние входов.

7 - 2019.03.01 - dontsovcmc
	1. Обновил фреймворк до Platformio Atmel AVR 1.12.5
	2. Время аварийного отключения ESP 120сек. 
	   Даже при отсутствии связи ESP раньше в таймауты уйдет и пришлет "спим".
*/
     
// Счетчики импульсов

// Waterius Classic: https://github.com/dontsovcmc/waterius
//
//                                +-\/-+
//       RESET   (D  5/A0)  PB5  1|    |8  VCC
//  *Counter1*   (D  3/A3)  PB3  2|    |7  PB2  (D  2/ A1)         SCL   *Button*
//  *Counter0*   (D  4/A2)  PB4  3|    |6  PB1  (D  1)      MISO         *Power ESP*
//                          GND  4|    |5  PB0  (D  0)      MOSI   SDA   
//                                +----+
//
// https://github.com/SpenceKonde/ATTinyCore/blob/master/avr/extras/ATtiny_x5.md

// Данные Wateriusa
NewHeader head = {FIRMWARE_VER, 0,0, WATERIUS_2C, {0,0,0},{0,0,0}, 0, NORMAL_MODE, WAKEUP_PERIOD_DEFAULT};  //24 байт

// Структура для хранения в EEPROM
struct WateriusStore{
	Store Channel1;
	Store Channel2;
	uint8_t Resets;
};
WateriusStore WS EEMEM = {	{{0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
									{{0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
									0};					 

static Sensor sensorA(&head.chanel1,&WS.Channel1);
#ifndef LOG_ON
static Sensor sensorB(&head.chanel2,&WS.Channel2);
#endif
static ESPPowerPin esp;

/* 	Флаг сбрасывается стороживым таймером каждые 0,25 секунды.
*/
volatile uint8_t tick=0;

/* Вектор прерываний сторожевого таймера watchdog */
ISR(WDT_vect) { 
	tick=0;
}  

// Главный цикл, повторящийся раз в сутки или при настройке вотериуса
int main() {
	noInterrupts();
	head.service = MCUSR; // причина перезагрузки
	MCUSR = 0;            // без этого не работает после перезагрузки по watchdog
	wdt_disable();
    wdt_enable(WDTO_250MS);
	WDTCR |= _BV(WDIE); 
	interrupts(); 
	ADCSRA=3;

	set_sleep_mode( SLEEP_MODE_PWR_DOWN );

	head.resets = eeprom_read_byte(&WS.Resets);
	head.resets++;
	eeprom_update_byte(&WS.Resets, head.resets);

	// Inicialize
	//adc
	DDRB &= ~_BV(PB4);
	DDRB &= ~_BV(PB3);

	LOG_BEGIN(9600); 
	LOG(F("==== START ===="));
	LOG(F("MCUSR")); LOG(head.service);
	LOG(F("RESET")); LOG(head.resets);
	LOG(F("EEPROM used:")); LOG(storage.size() + 1);
	LOG(F("Data:"));
	LOG(head.chanel1.counter);
	LOG(head.chanel2.counter);

	
	while(1){
		if (!tick){ // Если проснулись по сторожевому таймеру то обрабатывает счетчики
			tick=1;
			
			WDTCR |= _BV(WDIE); 
			power_adc_enable(); 
    		adc_enable();       //после подачи питания на adc
			PORTB|=_BV(PORTB4);
			ADMUX = (1<<ADLAR) | 2;
			sensorA.check();
			PORTB&=~_BV(PORTB4);
			#ifndef LOG_ON
				PORTB|=_BV(PORTB3);	
				ADMUX = (1<<ADLAR) | 3;
				sensorB.check();
				PORTB&=~_BV(PORTB3);
			#endif
			adc_disable();
    		power_adc_disable();

			// count minute
			ESPPowerPin::tick();			
		}		
		#ifdef LOG_ON
			mySerial.print(F("."));
		#endif
		sleep_mode();
	}
}
