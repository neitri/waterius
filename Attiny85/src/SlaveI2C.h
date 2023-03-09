#ifndef _SLAVEI2C_h
#define _SLAVEI2C_h

#include <Arduino.h>
#include <Waterius.h>

class SlaveI2C
{
protected:
    static uint8_t txBuffer[sizeof(Header)];
    static uint8_t txBufferPos;
    static uint8_t setup_mode;

    static bool masterSentSleep;
    //Время включения Wi-Fi
    static unsigned long SentSleep_timestamp; 

    static void requestEvent();
    static void newCommand();
    static void receiveEvent(int howMany);
    static void getWakeUpPeriod();

public:
    void begin(const uint8_t);
    static void end();
    bool masterGoingToSleep();
};

#endif
