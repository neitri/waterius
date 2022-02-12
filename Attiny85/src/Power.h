#ifndef _WATERIUS_POWER_h
#define _WATERIUS_POWER_h

#include <Arduino.h>
#include "SlaveI2C.h"

//Выключение ADC сохраняет ~230uAF. 
#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC


#define BUTTON_FILTER 1
#define BUTTON_LONG_PRESS	20
#define BUTTON_SHORT_PRESS	1
#define BUTTON_SHUTDOWN		8

/*
    Клас управления питанием ESP с инициализацией I2C 
    В методе tick() подсчитывается время работы по истечению которого 
    включается режим передачи данных. По длительному нажатию кнопки 
    включается редим настройки. Также реализовано ограничение 
    длительности работы ESP.
*/
class ESPPowerPin {
    protected:
        static uint8_t btn_r;
        static uint8_t btn_f;
        static uint16_t countMinute;
	    static uint8_t wdt_count;

    public:
    explicit ESPPowerPin();
    // Подать питание с ESP, и включить i2c
    static void on();
    // Отключить питание с ESP, и включить i2c
    static void off();      
    // Установить нормальный режим работы (только подсчет импульсов)
    static void setModeNormal();
    // Установить режим работы "Настройка"
    static void setModeSetup();
    // Установить режим работы "Передача данных"
    static void setModeTransmit();
    // Функция подсчета времени
    static void tick();
};

#endif

