
#include "Power.h"
#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include "Setup.h"
#include "SlaveI2C.h"
#include "Sensor.h"

extern struct NewHeader head;

/* Счетчик минут до передачи данных*/
uint16_t ESPPowerPin::countMinute = WAKEUP_PERIOD_DEFAULT;
/* Счетчик отсчета минутного интервала*/
uint8_t ESPPowerPin::wdt_count = 240;
/* таймер нажатого состояния кнопки*/
uint8_t ESPPowerPin::btn_r = 0;
/* таймер отпущенного состояния кнопки */
uint8_t ESPPowerPin::btn_f = 0;

ESPPowerPin::ESPPowerPin()
{
    setModeNormal();
};

void ESPPowerPin::setModeNormal()
{
    head.mode = NORMAL_MODE;
    countMinute = head.wakeupPeriod;
    wdt_count = 0;
    off();
}

void ESPPowerPin::setModeSetup()
{
    head.mode = SETUP_MODE;
    countMinute = 10;
    wdt_count = 0;
    on();
}

void ESPPowerPin::setModeTransmit()
{
    head.mode = TRANSMIT_MODE;
    countMinute = 0;
    wdt_count = 60;
    on();
}

void ESPPowerPin::tick()
{
    if (wdt_count == 0)
    { // выполнение каждую минуту
        wdt_count = 240;
        if (countMinute == 0)
        {
            if (head.mode == NORMAL_MODE)
            {
                setModeTransmit();
            }
            else
            {
                setModeNormal();
            }
        }
        else
        {
            countMinute--;
        }
    }
    else
    {
        wdt_count--;
    }

    // On key pressed
    if (!(PINB & _BV(PB2)))
    {
        btn_r++;
        if (btn_r > BUTTON_FILTER)
        {
            btn_f = 0;
        }
    }
    else if (btn_r)
    {
        btn_f++;
        if (btn_f > BUTTON_FILTER)
        {
            if (head.mode == NORMAL_MODE)
            {
                if (btn_r > BUTTON_LONG_PRESS)
                {
                    setModeSetup();
                }
                else if (btn_r > BUTTON_SHORT_PRESS)
                {
                    setModeTransmit();
                }
            }
            else
            {
                if (btn_r > BUTTON_SHUTDOWN)
                {
                    setModeNormal();
                    // btn_f=0;
                }
            }
            btn_r = 0;
        }
    }
}

void ESPPowerPin::on()
{
    power_all_enable();
    set_sleep_mode(SLEEP_MODE_IDLE);
    SlaveI2C::begin();
    asm volatile("sbi %0, %1 "
                 :
                 : "I"(_SFR_IO_ADDR(DDRB)), "I"(PB1)
                 :);
    asm volatile("sbi %0, %1 "
                 :
                 : "I"(_SFR_IO_ADDR(PORTB)), "I"(PB1)
                 :);
}

void ESPPowerPin::off()
{
    asm volatile("cbi %0, %1 "
                 :
                 : "I"(_SFR_IO_ADDR(PORTB)), "I"(PB1)
                 :);

    asm volatile("cbi %0, %1 "
                 :
                 : "I"(_SFR_IO_ADDR(DDRB)), "I"(PB1)
                 :);
    SlaveI2C::end();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    power_all_disable();
}
